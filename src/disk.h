#ifndef DISK_H
#define DISK_H

#include <QtCore>
#include "exceptions.h"

#ifdef Q_OS_WIN
#include <windows.h>
#else
#include <unistd.h>
#endif

class Disk
{
public:
    ~Disk();
    Disk(const char *root);
    Disk(QString root);
    long ReadSector(unsigned sector_number, unsigned char *data, unsigned len);
    bool OpenDisk() noexcept(false);
    bool CloseDisk();
    bool IsOpen() const;
    static bool IsHardDiskPath(QString path);
    unsigned long GetBytesPerSector();
    unsigned long long GetSize();
private:
    QString MapRootDrive();
    QString root;
    bool isDiskOpen;
#ifdef Q_OS_LINUX
    FILE *disk_file;
#else
    HANDLE disk_file;
#endif
};

#endif // DISK_H
