#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <QtCore/QtCore>

class Exception
{
public:
    Exception();
    Exception(QString message);
    ~Exception();

    virtual QString getMessage();
private:
    QString message;
};

class IOException : public Exception
{
public:
    IOException(QString message);
    IOException();
    ~IOException();

    QString getMessage();

private:
    QString message;
};

#endif // EXCEPTION_H
