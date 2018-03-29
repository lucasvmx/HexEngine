#ifndef HEXENGINE_H
#define HEXENGINE_H

#include "mainwindow.h"
#include <QtCore>
#include <QThread>

#ifndef CONFIG_FILENAME
#define CONFIG_FILENAME     "hexengine_settings.ini"
#endif

class HexEngine : public QThread
{
    Q_OBJECT

    friend class MainWindow;
public:
    HexEngine();
    ~HexEngine();
    void run();
    QString filename;
    QString outfilename;
private:
    void CreateHexDumpFromFile(QString &filename);
    QString ts(); /* Time String */
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

#endif // HEXENGINE_H
