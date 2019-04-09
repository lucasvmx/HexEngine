#include "ini_parser.h"
#include "config_form.h"
#include <QFile>
#include <QSettings>

static QFile *file = nullptr;

ini_parser::ini_parser()
{
    file = new QFile(CONFIG_FILENAME);
    Q_ASSERT_X(file != NULL, "ini_parser", "Null pointer detected");
    file->open(QIODevice::ReadOnly);
    Q_ASSERT_X(file->isOpen(), "ini_parser", "File can't be openned");
}

ini_parser::~ini_parser()
{
    if(file)
        file->close();
}

int ini_parser::getSectionAddr(QString section)
{
    char data[256];
    QString line_data;
    int line = 1;
    int address = -1;
    int r;

    Q_ASSERT_X(file != NULL, "getSectionAddr", "Null pointer detected");
    if(file->atEnd())
        Q_ASSERT_X(file->seek(0), "file_seek", "Failed to return to begin");

    while(!file->atEnd())
    {
        if(file->readLine(data, sizeof(data)) > 0)
        {
            line_data = data;
            line_data = line_data.simplified();
            r = line_data.compare(section);
            if(r == 0)
            {
                address = line;
                break;
            }

            line++;
        }
    }

    file->seek(0);

    return address;
}

int ini_parser::getKeyValue(QString key)
{
    char data[256];
    QString line_data;
    int addr;
    int line = 1;
    QStringList list;
    QString keyName;
    QString keyValue;
    int keyvalue = -1;

    Q_ASSERT_X(file != NULL, "getKeyValue", "Null pointer detected");
    addr = this->getSectionAddr("[settings]");
    Q_ASSERT_X(addr > 0, "getKeyValue", "Invalid address detected");

    while(!file->atEnd())
    {
        if(file->readLine(data, sizeof(data)) > 0)
        {
            if(line > addr)
            {
                line_data = data;
                line_data = line_data.simplified();
                list = line_data.split('=', QString::SkipEmptyParts);
                keyName = list.first();
                keyValue = list.last();
                keyName = keyName.simplified();
                keyValue = keyValue.simplified();
                if(keyName.compare(key) == 0)
                {
                    keyvalue = keyValue.toInt();
                    break;
                }
            }
        }
        line++;
    }

    file->seek(0);

    return keyvalue;
}
