#include "disk.h"
#include <QtCore>
#include <assert.h>
#include <exception>
#include <QException>
#include <QRegExp>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

using namespace std;

disk::disk(const char *root)
{
    // Can be in format: X: or X:\\ for windows
    // and /dev/sd[a-z]X for Linux

    this->root = root;
    this->disk_file = nullptr;
    this->isDiskOpen = false;
}

bool disk::open_disk()
{
#ifdef Q_OS_WIN
    HANDLE hDisk = nullptr;
    QString device = "";

    device = QString::fromUtf8(this->map_root_drive().toStdString().c_str());

#ifdef QT_DEBUG
    qDebug() << "Device" << this->root << "is mapped to" << device << " - " << device.toStdString().c_str();
#endif

    hDisk = CreateFileA(device.toStdString().c_str(), 0, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if(hDisk == nullptr)
    {
#ifdef QT_DEBUG
        qDebug() << "Failed to open:" << device << "Error:" << GetLastError();
#endif

        if(isDiskOpen)
            isDiskOpen = false;

        this->disk_file = nullptr;

        return false;
    }

    this->disk_file = hDisk;
    this->isDiskOpen = true;

    return true;
#else
    FILE *hDisk = NULL;

    hDisk = fopen(this->root,"rb");
    if(hDisk == NULL)
    {
        if(isDiskOpen)
            isDiskOpen = false;

        return false;
    }

    isDiskOpen = true;
    this->disk_file = hDisk;

    return true;
#endif
}

bool disk::isOpen() const
{
    bool a,b;

#ifdef Q_OS_WIN
    a = disk_file != nullptr;
#else
    a = disk_file != NULL;
#endif

    b = isDiskOpen;

    return(a && b);
}

long disk::read_sector(unsigned int sector_number = 0, unsigned int *data = nullptr, unsigned int len = 0)
{
    Q_ASSERT(data != nullptr);
    Q_ASSERT(len > 0);

#ifdef Q_OS_WIN
    DWORD sectors_per_cluster;
    DWORD bytes_per_sector;
    DWORD free_clusters;
    DWORD total_clusters;
    DWORD bytes_readed;

    /* Len must be sizeof(data) */

    if(!this->isOpen())
    {
        qDebug() << "Disk is not open";
        return -1;
    }

    // Calculate position to move
    BOOL bOk = GetDiskFreeSpaceA(this->root,&sectors_per_cluster,&bytes_per_sector,&free_clusters,&total_clusters);
    if(!bOk) {
        qDebug() << "Failed to get disk information: " << GetLastError();
        return -2;
    }

    len /= sizeof(unsigned int);

    qDebug() << "Data contains" << len << "bytes";

    if(len <= bytes_per_sector) {
        qDebug() << "Len must be greather than " << bytes_per_sector << ". Actually it is " << len;
        return -3;
    }

    // Set file pointer position
    if(SetFilePointer(this->disk_file, static_cast<LONG>(sector_number * bytes_per_sector),nullptr,FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
        qDebug() << "Failed to set file pointer position: " << GetLastError();
        return -4;
    }

    // Read the sector and write data to a buffer
    bOk = ReadFile(this->disk_file,data,len,&bytes_readed,nullptr);
    if(!bOk) {
        qDebug() << "Failed to read sector " << sector_number << "from disk " << root << ". Error " << GetLastError();
        return -5;
    }

    return static_cast<long>(bytes_readed);
#else
    // Set file pointer position
    fseek(disk_file,sector_number,SEEK_SET);

    if(this->isOpen()) {
        fread(data,sizeof(data),len,disk_file);
    }

    return 0;
#endif
}

bool disk::close_disk()
{
    if(this->isOpen())
    {
#ifdef Q_OS_WIN
      return(CloseHandle(disk_file) != FALSE);
#else
      return(fclose(disk_file) == 0);
#endif
    }

    return true;
}

QString disk::map_root_drive()
{
    QString path = QString::fromLatin1(root);
    QString path_mapped = "";
    QChar letter;
    char c_letter;
    int id = 0;

#ifdef Q_OS_WIN
    /*
        Example: \\.\PhysicalDrive2
        Represents: C:\\ or C:

        We need to map this into function readable string
    */

    if(path.length() != 2 && path.length() != 3)
        return "";

    letter = path.at(0);
    if(!(letter.isLetter()))
        return "";

    c_letter = letter.toLatin1();
    for(char c = 'A', i = 0; c <= 'Z'; c++, i++)
    {
        if(c_letter == c) {
            id = i;
            break;
        }
    }

    path_mapped = QString::asprintf("\\\\.\\PhysicalDrive%d", id);
#ifdef QT_DEBUG
    qDebug() << "Path:" << path_mapped;
#endif

    return path_mapped;
#else
    (void)path;
    (void)path_mapped;
    (void)letter;
    (void)c_letter;
    (void)i;
    (void)id;

    return QString::fromLatin1(root);
#endif
}
