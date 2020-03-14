#include "disk.h"
#include <QtCore>
#include <assert.h>
#include <exception>
#include <QRegExp>
#include "exceptions.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <shellapi.h>
#include <shlwapi.h>
#else

#endif

using namespace std;

Disk::~Disk()
{
    this->root = "";
#ifdef QT_DEBUG
    qDebug() << "Destructing disk class";
#endif
}

Disk::Disk(const char *root)
{
    this->root = root;
#ifdef Q_OS_WIN
    this->disk_file = INVALID_HANDLE_VALUE;
#else
    this->disk_file = nullptr;
#endif
    this->isDiskOpen = false;
}

Disk::Disk(QString root)
{
#ifdef QT_DEBUG
    qDebug() << __FILE__ << ":" << __LINE__ << root;
#endif

    this->root = root.toStdString().c_str();
#ifdef Q_OS_WIN
    this->disk_file = INVALID_HANDLE_VALUE;
#else
    this->disk_file = nullptr;
#endif
    this->isDiskOpen = false;
}

bool Disk::OpenDisk() noexcept(false)
{
#ifdef Q_OS_WIN
    HANDLE hDisk = nullptr;
    QString device = "";

    device = MapRootDrive();

#ifdef QT_DEBUG
    qDebug() << "Device" << this->root << "is mapped to" << device;
#endif

    hDisk = CreateFileA(device.toStdString().c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if(hDisk == INVALID_HANDLE_VALUE)
    {
#ifdef QT_DEBUG
        qDebug() << "Failed to open:" << device << "Error:" << GetLastError();
#endif

        if(isDiskOpen)
            isDiskOpen = false;

        this->disk_file = nullptr;

        return false;
    }

#ifdef QT_DEBUG
    qDebug() << "Device open:" << device;
#endif

    this->disk_file = hDisk;
    this->isDiskOpen = true;

    return true;
#else
    FILE *hDisk = nullptr;

    hDisk = fopen(root.toStdString().c_str(), "rb");
    if(hDisk == nullptr)
    {
        if(isDiskOpen)
            isDiskOpen = false;

        throw new UnauthorizedAccessException(QString("Falha ao abrir a unidade %1 para leitura").arg(root));
    }

    isDiskOpen = true;
    this->disk_file = hDisk;

    return true;
#endif
}

bool Disk::IsOpen() const
{
    bool a,b;

#ifdef Q_OS_WIN
    a = disk_file != nullptr;
#else
    a = disk_file != nullptr;
#endif

    b = isDiskOpen;

    return(a && b);
}

bool Disk::IsHardDiskPath(QString path = QString())
{
#ifdef Q_OS_WIN
    QDir s;

    if(path.isNull() || path.isEmpty())
        return false;

    s = QDir(path);

    return s.isRoot();
#else
    return path.startsWith("/dev/sd") || path.startsWith("/media");
#endif
}

unsigned long Disk::GetBytesPerSector()
{
#ifdef Q_OS_WIN
    DWORD bytes_per_sector;

    if(!GetDiskFreeSpaceA(root.toStdString().c_str(), nullptr, &bytes_per_sector, nullptr, nullptr))
    {
#ifdef QT_DEBUG
        qDebug() << "GetDiskFreeSpaceA failed:" << GetLastError();
#endif
        return 0;
    }

    return bytes_per_sector;
#else
    QString path = "/sys/block/%1/queue/hw_sector_size";
    FILE *info = nullptr;
    unsigned long bytes_per_sector = 0;
    QString devname = "";
    QStringList list;

    list = root.split("/");

    for(QString item : list)
    {
        if(item.startsWith("sd"))
        {
            devname = item;
            break;
        }
    }

    if(devname.isEmpty())
        return 0;

    path = path.arg(devname);

    info = fopen(path.toStdString().c_str(), "r");
    if(!info)
    {
#ifdef QT_DEBUG
        qDebug() << "Error while acessing" << path;
#endif
        return 0;
    }

    fscanf(info, "%lu", &bytes_per_sector);
    fclose(info);

    return bytes_per_sector;
#endif
}

unsigned long long Disk::GetSize()
{
#ifdef Q_OS_WIN
    ULARGE_INTEGER free;
    ULARGE_INTEGER avaiable_to_caller;
    ULARGE_INTEGER total;

    if(!GetDiskFreeSpaceExA(root.toStdString().c_str(), &avaiable_to_caller, &total, &free))
    {
#ifdef QT_DEBUG
        qDebug() << "GetDiskFreeSpaceExA failed:" << GetLastError();
#endif
        return 0ULL;
    }

    return total.QuadPart;
#else
    QStringList list;
    QString devname;
    unsigned number = 0;
    unsigned long long size;

    list = root.split("/");
    for(QString item : list)
    {
        if(item.startsWith("/dev/sd"))
        {
            devname = item;
            if(devname.length() == 4)
            {
                for(QChar c : devname)
                {
                    if(isdigit(c.toLatin1()))
                    {
                        number = static_cast<unsigned>(c.toLatin1());
                        break;
                    }
                }
            }
        }

        if(number) break;
    }

    FILE *data = nullptr;
    QString path = (number == 0) ? "/sys/block/%1/%2/size":"/sys/block/%1/%2%3/size";

    if(number > 0)
        path = path.arg(devname).arg(devname).arg(number);
    else
        path = path.arg(devname).arg(devname);

    data = fopen(path.toStdString().c_str(), "r");
    if(data == nullptr)
    {
        return 1;
    }

    fscanf(data, "%llu", &size);
    fclose(data);

    return size;
#endif
}

long Disk::ReadSector(unsigned sector_number = 0, unsigned char *data = nullptr, unsigned len = 0)
{
    Q_ASSERT(data != nullptr);
    Q_ASSERT(len > 0);

#ifdef Q_OS_WIN
    DWORD bytes_per_sector;
    DWORD bytes_readed;
    BOOL bOk;

    if(!this->IsOpen() || this->disk_file == INVALID_HANDLE_VALUE)
    {
        qDebug() << "Disk is not open";
        return -1;
    }

    bytes_per_sector = GetBytesPerSector();
    if(bytes_per_sector == 0)
    {
        qDebug() << "Failed to get disk information: " << GetLastError();
        return -2;
    }

    len /= sizeof(unsigned char);

    qDebug() << "Data contains" << len << "bytes";

    if(len < bytes_per_sector)
    {
        qDebug() << "Len must be greather than " << bytes_per_sector << ". Actually it is " << len;
        return -3;
    }

    // Set file pointer position
    if(SetFilePointer(this->disk_file, static_cast<LONG>(sector_number * bytes_per_sector), nullptr, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
        qDebug() << "Failed to set file pointer position: " << GetLastError();
        return -4;
    }

    // Read the sector and write data to a buffer
    bOk = ReadFile(this->disk_file, data, len, &bytes_readed, nullptr);
    if(!bOk)
    {
        qDebug() << "Failed to read sector " << sector_number << "from disk " << root << ". Error " << GetLastError();
        return -5;
    }

    return static_cast<long>(bytes_readed);
#else
    size_t readed = 0;

    if(!IsOpen())
    {
        qDebug() << "Disk is not open";
        return 1;
    }

    // Set file pointer position
    if(fseek(disk_file, sector_number,SEEK_SET) != 0)
    {
        return 1;
    }

    if(IsOpen())
    {
        readed = fread(data, sizeof(unsigned int), len, disk_file);
    }

    return static_cast<long>(readed);
#endif
}

bool Disk::CloseDisk()
{
    if(this->IsOpen())
    {
#ifdef Q_OS_WIN
      return(CloseHandle(disk_file) != FALSE);
#else
      return(fclose(disk_file) == 0);
#endif
    }

    return true;
}

QString Disk::MapRootDrive()
{
    QString path = root;
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

#ifdef Q_OS_WIN
    qDebug() << "Mapping" << path << "len:" << path.length();
#endif

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

    path_mapped = QString("\\\\.\\PhysicalDrive%1").arg(id);

#ifdef QT_DEBUG
    qDebug() << "Path:" << path_mapped;
#endif

    return path_mapped;
#else
    (void)path;
    (void)path_mapped;
    (void)letter;
    (void)c_letter;
    (void)id;

    return root;
#endif
}
