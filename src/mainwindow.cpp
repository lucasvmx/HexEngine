#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "hexengine.h"
#include "config_form.h"
#include "forminjector.h"
#include "exceptions.h"
#include "common.h"
#include "autorevision.h"

#include <QFileDialog>
#include <QTime>
#include <QGraphicsEffect>
#include <QMessageBox>
#ifndef Q_OS_LINUX
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QMediaPlaylist>
#endif
#include <QCloseEvent>

using namespace Engine;

static HexEngine *engine = nullptr;

static QGraphicsOpacityEffect *effect = nullptr;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QString name = "HexEngine " + QString(VCS_TAG) + " build " + VCS_SHORT_HASH;

    setWindowTitle(name);

    engine = new Engine::HexEngine();
    this->configure_progress_bar();
    this->connect_all();
    this->setWindowIcon(QIcon(":/files/icon1.ico"));
    ui->text_browser->setAutoFillBackground(false);

    effect = new QGraphicsOpacityEffect(ui->text_browser);
    effect->setOpacity(0.9);
    ui->text_browser->setGraphicsEffect(effect);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete engine;

#ifdef Q_OS_WIN
    delete effect;
#endif
}

void MainWindow::connect_all()
{
    ui->statusBar->showMessage( "Loading user interface ...");

    QObject::connect(ui->line_edit_input, SIGNAL(textChanged(QString)),
                     this, SLOT(validate_line_edit_text(QString)));

    QObject::connect(ui->btn_browse_input, SIGNAL(clicked(bool)),
                     this, SLOT(handle_btn_browse_input_pressed(bool)));

    QObject::connect(ui->btn_browse_output, SIGNAL(clicked(bool)),
                     this, SLOT(handle_btn_browse_output_pressed(bool)));

    QObject::connect(engine, SIGNAL(progress_updated(int)),
                     ui->progress_bar, SLOT(setValue(int)));

    QObject::connect(engine, SIGNAL(text_browser_updated(QString)),
                     ui->text_browser, SLOT(insertHtml(QString)));

    QObject::connect(engine, SIGNAL(engine_stopped(bool)),
                     ui->btn_init_engine, SLOT(setEnabled(bool)));

    QObject::connect(ui->btn_init_engine, SIGNAL(clicked(bool)),
                     this, SLOT(handle_btn_init_engine_pressed(bool)));

    QObject::connect(engine, SIGNAL(status_bar_updated(QString, int)),
                     ui->statusBar, SLOT(showMessage(QString,int)));

    QObject::connect(engine ,SIGNAL(status_bar_color_changed(QString)),
                     this, SLOT(change_status_bar_color(QString)));

    QObject::connect(ui->actionQuit, SIGNAL(triggered(bool)),
                     this, SLOT(handle_action_quit(bool)));

    QObject::connect(engine, SIGNAL(engine_stopped(bool)),
                     this, SLOT(play_completed_task_sound(bool)));

    QObject::connect(engine, SIGNAL(engine_stopped_with_error(bool)),
                     this, SLOT(play_completed_task_with_error_sound(bool)));

    QObject::connect(ui->btn_configure_engine, SIGNAL(clicked(bool)),
                     this, SLOT(handle_btn_configure_engine_pressed(bool)));

    QObject::connect(ui->btn_stop_engine, SIGNAL(clicked(bool)),
                     this, SLOT(handle_btn_stop_engine_pressed(bool)));

    QObject::connect(ui->actionBatchFileInjector, SIGNAL(triggered(bool)),
                     this, SLOT(handle_action_batch_file_inject(bool)));

     ui->statusBar->showMessage( "Loading completed");
}

void MainWindow::configure_progress_bar(int min, int max, int value)
{
    ui->progress_bar->setMaximum(max);
    ui->progress_bar->setMinimum(min);
    ui->progress_bar->setValue(value);
}

void MainWindow::change_status_bar_color(QString rgb)
{
    QString sheet;

    if(rgb.endsWith( ");"))
        return;
    if(!rgb.startsWith("rgb("))
        return;

    sheet = "\
    QStatusBar\
    {\
        background-color: " + rgb + ";" + \
        "color: rgb(255, 255, 255);\
    }";

    ui->statusBar->setStyleSheet(sheet);
}

void MainWindow::validate_line_edit_text(QString text)
{
    bool validated = true;
    QFile f(text);
    QDir d(text);

    if((!f.exists()) || (d.isRoot()) || (!d.isReadable()))
        validated = false;

    if(validated)
    {
        this->change_status_bar_color("rgb(0, 170, 0)");
        ui->statusBar->showMessage( "Input file location is valid", 0);
    } else
    {
        this->change_status_bar_color("rgb(255, 0, 0)");
        ui->statusBar->showMessage("The input file location isn't valid.", 0);
    }
}

void MainWindow::play_completed_task_sound(bool b)
{
    Q_UNUSED(b);
    common::playSound(ID_TASK_COMPLETED);
}

void MainWindow::play_completed_task_with_error_sound(bool b)
{
    Q_UNUSED(b);
    common::playSound(ID_TASK_COMPLETED_WITH_ERROR);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    int result;

    if(engine->isRunning())
    {
        result = QMessageBox::question(this, "Pergunta", "Uma conversão está em andamento. Você tem certeza que deseja interromper a conversão ?");
        if(result == QMessageBox::Yes)
        {
            //engine->requestInterruption();
            engine->wait(3000);
            engine->terminate();
            event->accept();
        } else
        {
            event->ignore();
        }
    } else {
        event->accept();
    }
}

void MainWindow::handle_action_batch_file_inject(bool b)
{
    (void)b;

    static formInjector *injectorForm = nullptr;

    if(injectorForm == nullptr)
    {
        injectorForm = new formInjector();
        injectorForm->show();
        injectorForm->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
        injectorForm->setMouseTracking(true);
    } else
    {
        if(injectorForm->isVisible())
            return;

        delete injectorForm;

        injectorForm = new formInjector();
        injectorForm->show();
        injectorForm->setFocus();
        injectorForm->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
        injectorForm->setMouseTracking(true);
    }
}

void MainWindow::handle_action_quit(bool b)
{
    Q_UNUSED(b);

    engine->requestInterruption();
    engine->wait(3000);
    this->close();
}

void MainWindow::handle_btn_browse_input_pressed(bool pressed)
{
    QString fileName;
    QString generatedName;
    int dot_index;

    Q_UNUSED(pressed);

    this->change_status_bar_color("rgb(0, 170, 0)");
    fileName = QFileDialog::getOpenFileName(this, "Escolha o arquivo a ser convertido",
                                 QString(), "All Files (*.*);;");

    if(fileName.length() > 0)
    {
        ui->line_edit_input->setText(fileName);
        dot_index = fileName.lastIndexOf('.');
        generatedName = QString().fromStdString(fileName.toStdString().substr(0, static_cast<uint>(dot_index)));
        ui->line_edit_output->setText(generatedName + ".hex" );
        ui->statusBar->showMessage( "O arquivo a ser lido já foi selecionado", 5000);
    }
    else {
        ui->text_browser->setText("Nenhum arquivo selecionado");
        ui->statusBar->showMessage("Nenhum arquivo selecionado");
    }
}

void MainWindow::handle_btn_browse_output_pressed(bool pressed)
{
    QString fileName;

    Q_UNUSED(pressed);

    this->change_status_bar_color("rgb(0, 170, 0)");
    fileName = QFileDialog::getSaveFileName(this, "Save file to ...",
                                 QString(), "All Files (*.*);;");

    if(fileName.length() > 0) {
        ui->line_edit_output->setText(fileName);
        ui->statusBar->showMessage( "Input file location is set.", 5000);
    }
    else
        ui->text_browser->setText("No output file selected");
}

void MainWindow::handle_btn_init_engine_pressed(bool pressed)
{
	this->configure_progress_bar();
	
    QFile file(CONFIG_FILENAME);
    ui->text_browser->clear();
    if(file.exists())
    {
        ui->text_browser->insertHtml("["+engine->ts()+"] " + "Configuration file found<br>");
        ui->text_browser->insertHtml("["+engine->ts()+"] " + "<font blue = \"red\">Loading configuration file...</font><br>");
    }

    engine->filename = "";
    engine->outfilename = "";

    engine->filename = ui->line_edit_input->text();
    engine->outfilename = ui->line_edit_output->text();
    engine->runHexEngine(pressed);
}

void MainWindow::handle_btn_configure_engine_pressed(bool pressed)
{
    static config_form *c = nullptr;
    Q_UNUSED(pressed);

    if(c != nullptr && c->isVisible())
    {
        c->setWindowState(Qt::WindowActive);
        QMessageBox::critical(this, "Erro", "A janela de configurações já está aberta",
                              QMessageBox::Ok);
        return;
    } else
    {
        c = new config_form(nullptr);
        c->show();
    }
}

void MainWindow::handle_btn_stop_engine_pressed(bool pressed)
{
    Q_UNUSED(pressed);

    if(engine->isRunning())
    {
        engine->requestInterruption();
        engine->wait(3000);

        ui->text_browser->insertHtml("["+ engine->ts() + "] " + "<font color=\"blue\">Task canceled</font><br>");
        this->change_status_bar_color("rgb(0, 170, 0)");
        ui->statusBar->showMessage("Task canceled", 5000);
    }
    else
        QMessageBox::critical(this, "Error", "Engine isn't running");
}
