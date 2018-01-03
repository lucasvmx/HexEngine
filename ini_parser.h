#ifndef INI_PARSER_H
#define INI_PARSER_H

#include <QtCore>

#ifndef CONFIG_FILENAME
#define CONFIG_FILENAME     "hexengine_settings.ini"
#endif

class ini_parser
{
public:
    ini_parser();
    ~ini_parser();
    int getKeyValue(QString key);
private:
    int getSectionAddr(QString section);
};

#endif // INI_PARSER_H
