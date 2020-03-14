#ifndef HEX_ENGINE_H
#define HEX_ENGINE_H

#include "mainwindow.h"
#include <QtCore>
#include <QThread>
#include <QObject>

#ifndef CONFIG_FILENAME
#define CONFIG_FILENAME     "hexengine_settings.ini"
#endif

namespace Engine
{
    class HexEngine : public QThread
    {
        Q_OBJECT

        friend class MainWindow;

    public:
        HexEngine();
        virtual ~HexEngine();
        void run();
        QString ts(); /* Time String */
        QString filename;
        QString outfilename;
        unsigned long long calculateConversionLimitKB();

    private:
        void CreateHexdumpFromFile(QString &filename);
        void CreateHexdumpFromDisk(QString &disk);

    signals:
        void progress_updated(int v);
        void text_browser_updated(QString status);
        void status_bar_updated(QString status, int time);
        void engine_started(bool);
        void engine_stopped(bool);
        void engine_stopped_with_error(bool);
        void status_bar_color_changed(QString rgb);
    public slots:
        void runHexEngine(bool unused);
    };
}

#endif // HEXENGINE_H
