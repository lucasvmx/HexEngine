#include "config_form.h"
#include "ui_config_form.h"
#include <QTime>
#include <QFile>
#include <QDateTime>
#include <QMessageBox>

#ifdef QT_DEBUG
#include <QDebug>
#endif

/*
 * Icons:
 *
 * Alexandre Luis - https://www.iconfinder.com/stormicons
 * Oxygen Icons - http://www.oxygen-icons.org/
 * Madarancio - http://mandarancio.deviantart.com/
 */

/*
	Button click sound
	
	Title: Button
	About: The sound of a button being clicked. great for flash design or operating system.
	Uploaded: 07.27.09 | License: Attribution 3.0 | Recorded by Mike Koenig | File Size: 45 KB
*/

/*
 *	Sound:
 *	task-completed: https://notificationsounds.com/message-tones/finished-task-206 Creative Commons
 */

config_form::config_form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::config_form)
{
    ui->setupUi(this);
    this->connect_all_events();
    this->setWindowTitle("Settings");
    this->setWindowIcon(QIcon(":/files/icon2.ico"));
}

config_form::~config_form()
{
    delete ui;
}

void config_form::handle_dial_value_changed(int value)
{
    QString text;
    QString new_text;
    QString tx;
    qint32 seed;
    QTime tm;
    qint32 random;

    tm = QTime::currentTime();
    seed = tm.hour() + tm.second() + tm.minute() + tm.msec();
    qsrand(seed);
    random = qrand() % 50 + 1;

    try
    {
        text = QString::asprintf( "0x%%.%dx",value);
        new_text = QString::asprintf("Hex: %s Dec: %d", text.toStdString().c_str(), random);
        tx = QString::asprintf(new_text.toStdString().c_str(), random);
#ifdef QT_DEBUG
        qDebug() << text << ":" << tx << ":" << new_text;
#endif
    } catch(std::exception e)
    {
        QMessageBox::critical(this,"Fatal error", QString(e.what()));
        return;
    }

    ui->lcd_display_ndigits->display(value);
    ui->label_ndigits_sample->setText(tx);
}

void config_form::handle_window_destroyed(QObject *o)
{
    QString title = "Alert";
    QMessageBox::StandardButtons buttons = QMessageBox::Yes | QMessageBox::No;
    QString text = "Do you want to save your settings";
    Q_UNUSED(o);

    if(QMessageBox::warning(this, title, text, buttons) == QMessageBox::Yes)
        handle_btn_save_settings_clicked(true);
}

void config_form::handle_btn_save_settings_clicked(bool b)
{
    QFile *ini = NULL;
    QDateTime tm;

    Q_UNUSED(b);

    ini = new QFile(CONFIG_FILENAME);

    if(ini == NULL)
    {
        QMessageBox::critical(this, "Error", "Failed to save settings");
        return;
    }

    if(ini->open(QIODevice::WriteOnly) == false)
    {
        QMessageBox::critical(this, "Error", "Failed to save settings [open]");
        return;
    }

    /* Write header */
    tm = QDateTime::currentDateTime().toLocalTime();
    ini->write(QString("# File created by hexengine on %1 at %2\n").arg( \
                   tm.date().toString(),
                   tm.time().toString()).toStdString().c_str());
    ini->write("# Please. Be careful while editing this file directly\n\n");
    ini->write("[settings]\n");

    /* Write settings ... */
    if(ui->checkbox_carray_output->isChecked())
        ini->write(QString("c_array = %1\n").arg(true).toStdString().c_str());
    else
        ini->write(QString("c_array = %1\n").arg(false).toStdString().c_str());

    if(ui->groupBox->isEnabled())
        ini->write(QString("digits = %1").arg(ui->dial_ndigits->value()).toStdString().c_str());
    else
        ini->write("digits = 0");

    ini->close();
    if(ini->size() <= 0)
        QMessageBox::critical(this, "Error", "Settings can't be saved");
    else
    {
        QMessageBox::information(this, "Notice", "Settings saved");
        this->close();
    }
}

void config_form::connect_all_events()
{
    QObject::connect(ui->dial_ndigits, SIGNAL(valueChanged(int)),
                     this, SLOT(handle_dial_value_changed(int)));

    QObject::connect(ui->checkbox_carray_output, SIGNAL(toggled(bool)),
                     ui->groupBox, SLOT(setEnabled(bool)));

    QObject::connect(ui->checkbox_carray_output, SIGNAL(toggled(bool)),
                     ui->dial_ndigits, SLOT(setEnabled(bool)));
    QObject::connect(this, SIGNAL(destroyed(QObject*)),
                     this, SLOT(handle_window_destroyed(QObject *)));

    QObject::connect(ui->btn_save_settings, SIGNAL(clicked(bool)),
                     this, SLOT(handle_btn_save_settings_clicked(bool)));
}
