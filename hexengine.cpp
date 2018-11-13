#include "hexengine.h"
#include "ini_parser.h"

#include <QMessageBox>

#ifdef Q_OS_LINUX
#include <stdlib.h>
#include <sys/sysinfo.h>
#else
#include <windows.h>
#endif

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
        emit text_browser_updated( "[" + ts() + "] " + "<font color=\"red\">Error</font>: Null filename received.<br>");
        emit engine_stopped_with_error(true);
        emit status_bar_updated( "You need to specify an input filename", 5000);
        emit status_bar_color_changed("rgb(255, 0, 0)");
        return;
    }

    out = new QFile(outfilename);
    file = new QFile(filename);
    if(file == nullptr || out == nullptr)
    {
        emit text_browser_updated( "[" + ts() + "] " + "<font color=\"red\">Error:</font> null pointer detected.<br>");
        emit engine_stopped_with_error(true);
        emit status_bar_updated(QString("Something was wrong (%1) ...").arg(((file == nullptr) ? (file->errorString()):(out->errorString()))), 5000);
        emit status_bar_color_changed("rgb(255, 0, 0)");
        return;
    }

    if(!file->open(QIODevice::ReadOnly) || (!out->open(QIODevice::WriteOnly)))
    {
        emit text_browser_updated( "[" + ts() + "] " + "<font color=\"red\">IO/Error</font>: Cannot open file for read or write<br>");
        emit status_bar_updated("Something was wrong ...", 5000);
        emit engine_stopped_with_error(true);
        emit status_bar_color_changed("rgb(255, 0, 0)");
        return;
    }

	emit engine_started(true);
    /* We need to check if the file exists */
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
    emit text_browser_updated( "[" + ts() + "] " + "Reading file ... <br>");
    emit text_browser_updated( "[" + ts() + "] " + "C Array: " + \
                               ((c_array_mode == 1) ? "true":"false") + ", Num. of digits: " + \
                               QVariant(digits).toString() + "<br>");

    total_file_len_MB = total_file_len  / 1024 / 1024;
	total_file_len_KB = total_file_len  / 1024;
	
    if(c_array_mode == 1)
    {
        if(digits < 2 || digits > 12)
        {
            emit text_browser_updated( "[" + ts() + "] " + \
                                       "<font color=\"red\">Warning:</font> Invalid settings detected on " + CONFIG_FILENAME + "<br>");
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

    // Check if we are in an computer with less RAM
     max_bytes_to_read = 65536;

#ifdef Q_OS_WIN
    MEMORYSTATUSEX *mem = nullptr;
    DWORDLONG totalMemory = 0;

    mem = static_cast<MEMORYSTATUSEX*>(malloc(sizeof(MEMORYSTATUSEX)));

    if(mem == nullptr)
    {
        emit text_browser_updated("WARNING: Failed to allocate memory :(<br>");
    } else
    {
        mem->dwLength = sizeof(MEMORYSTATUSEX);

        if(GlobalMemoryStatusEx(mem))
        {
            totalMemory = mem->ullTotalPhys;
            totalMemory /= 1048576; // convert to MB
            emit text_browser_updated(QString("[" + ts() + "] " + "Total memory: %1 MB<br>").arg(totalMemory));

            if(totalMemory <= 2048)
            {
                max_bytes_to_read = 8192;
            }

        } else {
            emit text_browser_updated(QString::asprintf("WARNING: GlobalMemoryStatus failed with error %lu<br>",GetLastError()));
        }

        if(mem)
            free(mem);
    }
#else
    struct sysinfo *sys = NULL;
    __kernel_ulong_t totalMemory = 0;

    sys = static_cast<struct sysinfo*>(malloc(sizeof(struct sysinfo)));

    if(sys == NULL)
    {
        emit text_browser_updated("WARNING: failed to allocate memory");
    } else {
        int s = sysinfo(sys);
        if(s == 0)
        {
            totalMemory = sys->totalram;
            totalMemory /= 1048576; // convert to MB
            emit text_browser_updated(QString("[" + ts() + "] " + "Total memory: %1 MB<br>").arg(totalMemory));

            if(totalMemory <= 2048)
            {
                max_bytes_to_read = 8192;
            }
        }

        if(sys)
            free(sys);
    }

#endif

    emit text_browser_updated(QString("[" + ts() + "] " + "File size: %1 KB (%2 MB)<br>").arg(total_file_len_KB).arg(total_file_len_MB));
    emit status_bar_color_changed("rgb(0, 0, 255)");
    emit status_bar_updated( "Process started. Please wait ...", 0);
	
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
	Q_ASSERT_X((!file->isOpen()) && (!out->isOpen()), "CreateHexDump", "File still open");

    emit text_browser_updated( "[" + ts() + "] " + "Operation completed.<br>");
    emit status_bar_updated( "Operation completed successfully. Have a nice day ...", 5000);
    emit status_bar_color_changed("rgb(0, 170, 0)");
	emit engine_stopped(true);
}

QString HexEngine::ts()
{
    QTime t;
    t = QTime::currentTime();
    return(t.toString());
}

void HexEngine::runHexEngine(bool unused)
{
    Q_UNUSED(unused);

    this->start();
}
