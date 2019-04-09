#ifndef DISK_H
#define DISK_H

#include <QtCore>
#include "exceptions.h"

#ifdef Q_OS_WIN
#include <windows.h>
#else
#include <unistd.h>
#endif

class disk
{
public:
    ~disk();
    disk(const char *root);
    long read_sector(unsigned int sector_number, unsigned int *data, unsigned int len);
    bool open_disk() noexcept(false);
    bool close_disk();
    bool isOpen() const;
private:
    QString map_root_drive();
    const char *root;
    bool isDiskOpen;
#ifdef Q_OS_LINUX
    FILE *disk_file = nullptr;
#else
    HANDLE disk_file = nullptr;
#endif
};

#endif // DISK_H
