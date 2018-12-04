#ifndef HEXENGINE_H
#define HEXENGINE_H

#include "mainwindow.h"
#include <QtCore>
#include <QThread>

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
        ~HexEngine();
        void run();
        QString ts(); /* Time String */
        QString filename;
        QString outfilename;
        unsigned long long calculateConversionLimitKB();

    private:
        void CreateHexDumpFromFile(QString &filename);

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

    class HexInjector : public HexEngine
    {
        Q_OBJECT

    public:
        HexInjector();
        ~HexInjector();
        void run();
        void injectFile(QString from, QString to);
        void setIOFiles(QString from, QString to);

    signals:
        void status_updated(QString text);
        void started();
        void stopped();

    private:
        QString from, to;
        bool injectFileToBatchScript(QString fileToInject, QString batchFileName);

    };
}

#endif // HEXENGINE_H
