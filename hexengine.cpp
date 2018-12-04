#include "hexengine.h"
#include "ini_parser.h"
#include "exceptions.h"
#include "common.h"

#include <QMessageBox>
#include <QChar>
#include <QDir>
#include <QFile>

#ifdef QT_DEBUG
#include <QDebug>
#endif

#ifdef Q_OS_LINUX
#include <stdlib.h>
#include <sys/sysinfo.h>
#else
#include <windows.h>
#endif

using namespace Engine;

HexEngine::HexEngine()
{

}

HexEngine::~HexEngine()
{

}

void HexEngine::run()
{
    this->CreateHexDumpFromFile(this->filename);
}

void HexEngine::CreateHexDumpFromFile(QString& filename)
{
    QFile *file = nullptr;
    QFile *out = nullptr;
    QByteArray data;
    qint64 progress = 0;
    qint64 total_file_len;
	qint64 total_file_len_KB;
    qint64 total_file_len_MB;
    qint64 readed = 0;
    qint64 max_bytes_to_read;
    ini_parser *parser = nullptr;
    qint8 digits = 2;
    qint8 c_array_mode = 0;
	QString fmt;
	qint64 written = 0;
    QString file_name;
    QString file_extension;

    if(this->filename.isNull() || this->filename.isEmpty()
        || this->outfilename.isNull() || this->outfilename.isEmpty())
    {
#ifdef QT_DEBUG
        qDebug() << this->filename << ":" << this->outfilename;
#endif
        emit text_browser_updated( "[" + ts() + "] " + "<font color=\"red\">Erro</font>: Nome de arquivo nulo<br>");
        emit engine_stopped_with_error(true);
        emit status_bar_updated( "Você precisa escolher um arquivo para ser lido", 5000);
        emit status_bar_color_changed("rgb(255, 0, 0)");
        return;
    }

    out = new QFile(outfilename);
    file = new QFile(filename);
    if(file == nullptr || out == nullptr)
    {
        emit text_browser_updated( "[" + ts() + "] " + "<font color=\"red\">Erro:</font> ponteiro nulo detectado<br>");
        emit engine_stopped_with_error(true);
        emit status_bar_updated(QString("algo deu errado (%1) ...").arg(((file == nullptr) ? (file->errorString()):(out->errorString()))), 5000);
        emit status_bar_color_changed("rgb(255, 0, 0)");
        return;
    }

    if(!file->open(QIODevice::ReadOnly) || (!out->open(QIODevice::WriteOnly)))
    {
        emit text_browser_updated( "[" + ts() + "] " + "<font color=\"red\">IO/Error</font>: Erro de E/S. Não foi possível abrir o arquivo<br>");
        emit status_bar_updated("Algo deu errado ...", 5000);
        emit engine_stopped_with_error(true);
        emit status_bar_color_changed("rgb(255, 0, 0)");
        return;
    }

	emit engine_started(true);

    // Precisamos verificar se o arquivo existe
    QFile cfgFile(CONFIG_FILENAME);
    if(cfgFile.exists())
    {
        parser = new ini_parser();
        if(parser != nullptr)
        {
            digits = static_cast<qint8>(parser->getKeyValue("digits"));
            c_array_mode = static_cast<qint8>(parser->getKeyValue("c_array"));
            if(parser)
                delete parser;
        }
    }

    total_file_len = file->size();
    emit text_browser_updated( "[" + ts() + "] " + "Lendo arquivo ... <br>");
    emit text_browser_updated( "[" + ts() + "] " + "C Array: " + \
                               ((c_array_mode == 1) ? "sim":"não") + ", Dígitos: " + \
                               QVariant(digits).toString() + "<br>");

    total_file_len_MB = total_file_len  / 1024 / 1024;
	total_file_len_KB = total_file_len  / 1024;
	
    if(c_array_mode == 1)
    {
        if(digits < 2 || digits > 12)
        {
            emit text_browser_updated( "[" + ts() + "] " + \
                                       "<font color=\"red\">Alerta:</font> configurações inválidas encontradas em " + CONFIG_FILENAME + "<br>");
            digits = 2;
        }
        QFileInfo qfileinfo(file->fileName());

        file_name = qfileinfo.baseName();
        file_extension = qfileinfo.completeSuffix();
        file_extension = file_extension.replace('.', "_");
        out->write(QString::asprintf("unsigned int %s_%s[] = \n{\n\t",
                                     file_name.toStdString().c_str(),
                                     file_extension.toStdString().c_str()).toStdString().c_str());
    }

    // Aqui fazemos os ajustes importantes para que o programa tenha um bom desempenho em computadores com menor quantidade de RAM
     max_bytes_to_read = 65536;

    DWORDLONG totalMemory = common::getRAMSizeKB();
    if(totalMemory <= 0) {
        emit text_browser_updated(QString::asprintf("ALERTA: GlobalMemoryStatus falhou com o erro %lu<br>",GetLastError()));
    } else {
        totalMemory /= 1024; // Converte para MB
        emit text_browser_updated(QString("[" + ts() + "] " + "Memória RAM: %1 MB<br>").arg(totalMemory));

        if(totalMemory <= 2048)
        {
            max_bytes_to_read = 8192;
        }
    }

    emit text_browser_updated(QString("[" + ts() + "] " + "Tamanho do arquivo: %1 KB (%2 MB)<br>").arg(total_file_len_KB).arg(total_file_len_MB));
    emit status_bar_color_changed("rgb(0, 0, 255)");
    emit status_bar_updated( "Conversão iniciada. Por favor aguarde ...", 0);
	
    while(!file->atEnd())
    {
        if(this->isInterruptionRequested())
        {
            file->close();
            out->close();
            QFile s(outfilename);
            s.remove();
            emit progress_updated(0);
            return;
        }

		if(c_array_mode == 0)
			data = file->readLine(max_bytes_to_read);
		else
			data = file->read(max_bytes_to_read);
		
		if(data.length() > 0)
		{
            if(c_array_mode == 1)
            {
                foreach(QChar ch, data)
                {
                    unsigned int uch;
                    uch = static_cast<unsigned int>(ch.toLatin1());

                    fmt = QString("0x%.%1x, ").arg(digits);
                    out->write(QString::asprintf(fmt.toStdString().c_str(), (uch > 0xff) ? 0xff:uch).toStdString().c_str());
                    written++;
                    if(written == 16)
                    {
                        written = 0;
                        out->write("\n\t");
                    }
                }
            } else 
			{
                foreach(QChar ch, data)
                {
                    unsigned int uch;
                    uch = static_cast<unsigned int>(ch.toLatin1());

                    if(ch.toLatin1() == 0x0D)
                        out->write("\n");
                    else
                        out->write(QString::asprintf("%.2X", (uch > 0xff) ? 0xff:uch).toStdString().c_str());
                }
            }
		}
		
        readed += data.length();
        progress = (100 * readed / total_file_len);
        emit progress_updated(static_cast<int>(progress));
    }

    if(c_array_mode == 1)
    {
        fmt = QString("0x%.%1x").arg(digits);
        out->write(QString::asprintf(fmt.toStdString().c_str(), 0).toStdString().c_str());
        out->write(QString::asprintf("\n};\nunsigned int %s_%s_array_len = %lld;\n", file_name.toStdString().c_str(), file_extension.toStdString().c_str(), readed).toStdString().c_str());
    }

    file->close();
    out->close();
    Q_ASSERT_X((!file->isOpen()) && (!out->isOpen()), "CreateHexDump", "Arquivo continua aberto");

    emit text_browser_updated( "[" + ts() + "] " + "Operação finalizada<br>");
    emit status_bar_updated( "Operações finalizadas com sucesso. Tenha um bom dia ...", 5000);
    emit status_bar_color_changed("rgb(0, 170, 0)");
	emit engine_stopped(true);
}

QString HexEngine::ts()
{
    QTime t;
    t = QTime::currentTime();
    return(t.toString());
}

unsigned long long HexEngine::calculateConversionLimitKB()
{
    unsigned long long ram = common::getRAMSizeKB();
    double limit;

    if(ram <= 0)
        return 0;

    // 1 % to total de memória RAM
    limit = (ram * 0.01);

#ifdef QT_DEBUG
    qDebug() << "Limite de memória RAM: " + QString(QVariant(limit).toString());
#endif

    return static_cast<unsigned long long>(limit);
}

void HexEngine::runHexEngine(bool unused)
{
    Q_UNUSED(unused);

    this->start();
}

HexInjector::HexInjector()
{

}

HexInjector::~HexInjector()
{

}

void HexInjector::run()
{
    try {
        injectFile(this->from, this->to);
    } catch(Exception *e)
    {
        emit status_updated(e->getMessage());
        emit stopped();
        common::playSound(ID_TASK_COMPLETED_WITH_ERROR);
    }
}

void HexInjector::injectFile(QString from, QString to)
{
    injectFileToBatchScript(from,to);
}

void HexInjector::setIOFiles(QString from, QString to)
{
    this->from = from;
    this->to = to;
}

bool HexInjector::injectFileToBatchScript(QString fileToInject = QString(), QString batchFileName = QString())
{
    QFile *in = nullptr;
    QFile *out = nullptr;
    bool haveOutput = false;
    QString outName;
    const char *batch_stub[] =
    {
        "echo On Error Resume Next > %vbs_file%\n",
        "echo aGV4X3N0cgFG = \"",
        "echo Function SW48657ZW37X72546F2HF96C65(NEU1NDU1MzI1MTZBNTI0MjRFNTQ0RDNE) >> %vbs_file%\n",
        "echo   Dim KsWZnNv^, aGFuZGxl >> %vbs_file%\n",
        "echo   set KsWZnNv = CreateObject( \"Scripting.FileSystemObject\") >> %vbs_file%\n",
        "echo   set aGFuZGxl = KsWZnNv.OpenTextFile(NEU1NDU1MzI1MTZBNTI0MjRFNTQ0RDNE, 2, True) >> %vbs_file%\n",
        "echo   for i = 1 to len(aGV4X3N0cgFG) step 2 >> %vbs_file%\n",
        "echo       NEU1NDU1MzI1MTZCNTI0MjRFNTQ0RDNE = \"&h\" >> %vbs_file%\n",
        "echo       NEU1NDU1MzI1MTZCNTI0MjRFNTQ0RDNE = NEU1NDU1MzI1MTZCNTI0MjRFNTQ0RDNE ^& Mid(aGV4X3N0cgFG, i, 2) >> %vbs_file%\n",
        "echo       aGFuZGxl.write Chr(NEU1NDU1MzI1MTZCNTI0MjRFNTQ0RDNE) >> %vbs_file%\n",
        "echo   next >> %vbs_file%\n",
        "echo   aGFuZGxl.Close >> %vbs_file%\n",
        "echo End Function >> %vbs_file%\n\n",
        "echo SW48657ZW37X72546F2HF96C65(\"%file_to_inject%\") >> %vbs_file%\n"
    };
    QString line_data;
    QString vbsName;
    QFileInfo f;
    quint64 bytes_lidos = 0;
    quint64 tam_total_arquivo;

    haveOutput = (batchFileName.isNull() || batchFileName.isEmpty()) ? false:true;

    in = new QFile(fileToInject);
    if(in == nullptr)
        throw new IOException( "Falha ao abrir o arquivo " + fileToInject);


    unsigned long long limite = calculateConversionLimitKB();

    if(static_cast<unsigned long long>((in->size()/1024)) > (limite))
    {
        throw new Exception( "Tamanho limite de arquivo excedido\n\nTamanho do arquivo: " + QVariant(in->size() / 1024).toString() + " kB");
    }



    if(haveOutput)
    {
        out = new QFile(batchFileName);
        if(out == nullptr)
            throw new IOException( "Falha ao abrir arquivo " + batchFileName + " no modo escrita");

        f = QFileInfo(batchFileName);
        outName = batchFileName;
        vbsName = f.fileName() + ".vbs";
    } else {
        f = QFileInfo(fileToInject);
        outName = fileToInject + ".bat";
        vbsName = f.fileName() + ".vbs";
        out = new QFile(outName);
    }

    this->from = fileToInject;
    this->to = outName;

    if(!in->open(QIODevice::ReadOnly))
        throw new IOException( "Erro ao abrir o arquivo " + fileToInject + " no modo leitura");

    if(!out->open(QIODevice::WriteOnly))
        throw new IOException( "Erro ao abrir o arquivo " + outName + " no modo escrita");

    emit started();



    line_data = QString(batch_stub[0]).replace("%vbs_file%", vbsName);
    out->write(line_data.toStdString().c_str());
    out->write(batch_stub[1]);

    tam_total_arquivo = static_cast<quint64>(in->size());

    while(!in->atEnd())
    {
        QByteArray data = in->read(1024);
        int count = 0;

        foreach(QChar ch, data)
        {
            if(count == 60)
            {
                line_data = QString("\" >> %vbs_file%\n").replace("%vbs_file%",vbsName);
                out->write(line_data.toStdString().c_str());
                out->write("echo aGV4X3N0cgFG = aGV4X3N0cgFG ^& \"");
                count = 0;
            }

            unsigned int us;

            us = static_cast<unsigned int>(ch.toLatin1());
            out->write(QString::asprintf("%02X", (us > 0xff) ? 0xff:us).toStdString().c_str());
            count++;
        }

        bytes_lidos += static_cast<quint64>(data.length());

        emit status_updated(QString::asprintf("%llu/%llu kB", bytes_lidos / 1024, tam_total_arquivo / 1024));
    }

    out->write("\"\n");
    in->close();

    const unsigned size = ARRAYSIZE(batch_stub);
    bool bInterrupted = false;

    for(unsigned j = 2; j < size; j++)
    {
        if(this->isInterruptionRequested())
        {
            bInterrupted = true;
            break;
        }

        line_data = batch_stub[j];

        if(line_data.contains("%file_to_inject%")) {
            QFileInfo fi(fileToInject);
            line_data = line_data.replace("%file_to_inject%", "hexengine_" + fi.fileName());
        }

        if(line_data.contains("%vbs_file%"))
            line_data = line_data.replace("%vbs_file%", vbsName);

        out->write(line_data.toStdString().c_str());
    }

    in->close();
    out->close();

    emit status_updated("Injeção finalizada");
    emit stopped();

    if(bInterrupted)
        out->remove();

    delete in;
    delete out;

    common::playSound(ID_TASK_COMPLETED);

    return true;
}
