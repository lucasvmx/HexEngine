#include "common.h"

#ifdef Q_OS_WIN
    #include <QMediaPlayer>
    #include <windows.h>
#else

#endif

static QMediaPlayer *p = nullptr;

common::common()
{

}

unsigned long long common::getRAMSizeKB()
{
#ifdef Q_OS_WIN
    MEMORYSTATUSEX *mem = nullptr;
    DWORDLONG totalMemory = 0;

    mem = static_cast<MEMORYSTATUSEX*>(malloc(sizeof(MEMORYSTATUSEX)));

    if(mem == nullptr)
    {
        return 0;
    } else
    {
        mem->dwLength = sizeof(MEMORYSTATUSEX);

        if(GlobalMemoryStatusEx(mem))
        {
            totalMemory = mem->ullTotalPhys;
            totalMemory /= 1024; // converte para kB
        } else {
           return 0;
        }

        if(mem)
            free(mem);

        return totalMemory;
    }
#else
    struct sysinfo *sys = nullptr;
    __kernel_ulong_t totalMemory = 0;

    sys = static_cast<struct sysinfo*>(malloc(sizeof(struct sysinfo)));

    if(sys == nullptr)
    {
        return 0;
    } else {
        int s = sysinfo(sys);
        if(s == 0)
        {
            totalMemory = sys->totalram;
            totalMemory /= 1024; // converte para kB
        } else {
            return 0;
        }

        if(sys)
            free(sys);

        return totalMemory;
    }
#endif
}

void common::playSound(int id)
{
#ifndef Q_OS_LINUX
    p = new QMediaPlayer();

    switch(id)
    {
    case ID_TASK_COMPLETED:
        p->setMedia(QUrl("qrc:/files/finished-task.mp3"));
        p->setVolume(100);
        p->play();
    break;
    case ID_TASK_COMPLETED_WITH_ERROR:
        p->setMedia(QUrl("qrc:/files/finished-task_with-error.mp3"));
        p->setVolume(100);
        p->play();
        break;
    }

    delete p;
#else
    (void)id;
    QApplication::beep();
#endif
}
