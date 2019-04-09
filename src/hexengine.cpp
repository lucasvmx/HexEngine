#include "hexengine.h"
#include "ini_parser.h"
#include "exceptions.h"
#include "common.h"

#include <QMessageBox>
#include <QChar>
#include <QDir>
#include <QFile>
#include <QString>

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
        if(digits < 2 || digits > 4)
        {
            emit text_browser_updated( "[" + ts() + "] " + \
                                       "<font color=\"red\">Alerta:</font> configurações inválidas encontradas em " + CONFIG_FILENAME + "<br>");
            digits = 2;
        }
        QFileInfo qfileinfo(file->fileName());

        file_name = qfileinfo.baseName();
        file_extension = qfileinfo.completeSuffix();
        file_extension = file_extension.replace('.', "_");
        out->write((QString("unsigned int %1_%2[] = \n{\n\t").arg(file_name).arg(file_extension)).toStdString().c_str());
    }

    // Aqui fazemos os ajustes importantes para que o programa tenha um bom desempenho em computadores com menor quantidade de RAM
     max_bytes_to_read = 65536;

#ifdef Q_OS_WIN
    DWORDLONG totalMemory;
#else
    unsigned long long totalMemory;
#endif

    totalMemory = common::getRAMSizeKB();
    if(totalMemory <= 0)
    {
        emit text_browser_updated(QString("ALERTA: falha ao obter tamanho da RAM: %1<br>").arg(errno));
    } else
    {
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

                    out->write((QString("0x%1, ").arg(uch, digits, 16, QLatin1Char('0'))).toStdString().c_str());
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
                    {
                        out->write("\n");
                    }
                    else
                    {
                        out->write((QString("%1").arg(uch, 0, 16)).toStdString().c_str());
                    }
                }
            }
		}
		
        readed += data.length();
        progress = (100 * readed / total_file_len);
        emit progress_updated(static_cast<int>(progress));
    }

    if(c_array_mode == 1)
    {
        out->write((QString("0x%1").arg(0, digits, 16, QLatin1Char('0'))).toStdString().c_str());
        out->write((QString("\n};\nunsigned int %1_%2_array_len = %3;\n").arg(file_name).arg(file_extension).arg(readed)).toStdString().c_str());
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
