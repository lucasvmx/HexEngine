#include "exceptions.h"

#ifdef Q_OS_WIN
#include <QException>
#endif

Exception::Exception()
{
    message = "Um erro ocorreu durante o processamento da aplicação";
}

Exception::Exception(QString msg)
{
    message = msg;
}

Exception::~Exception()
{
    message = "";
}

QString Exception::getMessage()
{
    return message;
}

IOException::IOException()
{
    message = QString("Erro de E/S");
}

IOException::~IOException()
{
    message = "";
}

IOException::IOException(QString msg)
{
    message = msg;
}

QString IOException::getMessage()
{
    return message;
}

UnauthorizedAccessException::UnauthorizedAccessException()
{
    message = "Acesso negado";
}

UnauthorizedAccessException::UnauthorizedAccessException(QString msg)
{
    message = msg;
}

UnauthorizedAccessException::~UnauthorizedAccessException()
{
    message = "";
}

QString UnauthorizedAccessException::getMessage()
{
    return message;
}


