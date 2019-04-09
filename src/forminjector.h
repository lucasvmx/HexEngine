#ifndef FORMINJECTOR_H
#define FORMINJECTOR_H

#include <QWidget>

namespace Ui {
class formInjector;
}

class formInjector : public QWidget
{
    Q_OBJECT

public:
    explicit formInjector(QWidget *parent = nullptr);
    ~formInjector();

public slots:
        void handle_push_button_search_input_clicked(bool b);
        void handle_push_button_search_output_clicked(bool b);
        void handle_push_button_inject_clicked(bool b);
        void update_status(QString s);
        void stop_gif();
        void start_gif();
        void on_pushButton_StopInjection_clicked();

private:
    Ui::formInjector *ui;

    void connectEvents();
};

#endif // FORMINJECTOR_H
