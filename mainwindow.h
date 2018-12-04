#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "hexengine.h"
#include <QMainWindow>
#include <QThread>

#ifndef CONFIG_FILENAME
#define CONFIG_FILENAME     "hexengine_settings.ini"
#endif

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void handle_btn_browse_input_pressed(bool pressed);
    void handle_btn_browse_output_pressed(bool pressed);
    void handle_btn_init_engine_pressed(bool pressed);
    void handle_btn_configure_engine_pressed(bool pressed);
    void handle_btn_stop_engine_pressed(bool pressed);
    void change_status_bar_color(QString rgb = "rgb(0, 0, 255);");
    void validate_line_edit_text(QString text);
    void play_completed_task_sound(bool b);
    void play_completed_task_with_error_sound(bool b);
    void closeEvent(QCloseEvent *event);
    void handle_action_batch_file_inject(bool b);

private slots:
    void handle_action_quit(bool b);
private:
    Ui::MainWindow *ui;
    void connect_all();
    void configure_progress_bar(int min = 0, int max = 100, int value = 0);
};

#endif // MAINWINDOW_H
