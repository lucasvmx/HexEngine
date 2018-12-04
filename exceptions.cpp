#include "exceptions.h"

#include <QException>

Exception::Exception()
{
    this->message = "Um erro ocorreu durante o processamento da aplicação";
}

Exception::Exception(QString message)
{
    this->message = message;
}

Exception::~Exception()
{

}

QString Exception::getMessage()
{
    return this->message;
}

IOException::IOException()
{
    this->message = QString("Erro de E/S");
}

IOException::~IOException()
{

}

IOException::IOException(QString message)
{
    this->message = message;
}

QString IOException::getMessage()
{
    return message;
}
