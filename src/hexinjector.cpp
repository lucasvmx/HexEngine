#include "hexinjector.h"
#include "exceptions.h"
#include "common.h"

using namespace Engine;

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
            out->write(QString("%02X").arg(us).toStdString().c_str());
            count++;
        }

        bytes_lidos += static_cast<quint64>(data.length());

        emit status_updated(QString("%llu/%llu kB").arg(bytes_lidos/1024).arg(tam_total_arquivo/1024));
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
