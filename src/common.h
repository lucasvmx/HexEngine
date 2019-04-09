#ifndef COMMON_H
#define COMMON_H

#include <QObject>

// Para uso com a função playSound()
#define ID_TASK_COMPLETED               0x2
#define ID_TASK_COMPLETED_WITH_ERROR    0x3

#ifndef ARRAYSIZE
#define ARRAYSIZE(x)    (sizeof(x)/sizeof(x[0]))
#endif

class common
{
public:
    common();
    static unsigned long long getRAMSizeKB();
    static void playSound(int id);
};

#endif // COMMON_H
