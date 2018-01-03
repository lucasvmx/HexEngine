#ifndef CONFIG_FORM_H
#define CONFIG_FORM_H

#include <QWidget>
#include <QThread>

namespace Ui {
class config_form;
}

#ifndef CONFIG_FILENAME
#define CONFIG_FILENAME     "hexengine_settings.ini"
#endif

class config_form : public QWidget
{
    Q_OBJECT

public:
    explicit config_form(QWidget *parent = 0);
    ~config_form();

public slots:
    void handle_dial_value_changed(int value);
    void handle_window_destroyed(QObject *o);
    void handle_btn_save_settings_clicked(bool b);

private:
    Ui::config_form *ui;
    void connect_all_events(void);
};

#endif // CONFIG_FORM_H
