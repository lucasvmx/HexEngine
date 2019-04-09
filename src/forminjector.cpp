#include "forminjector.h"
#include "ui_forminjector.h"
#include "hexengine.h"
#include "common.h"
#include "exceptions.h"
#include "hexinjector.h"

#include <QFileDialog>
#include <QMovie>
#include <QCheckBox>
#include <QMessageBox>

static QMovie *gif = nullptr;
static Engine::HexInjector *injector = nullptr;

formInjector::formInjector(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::formInjector)
{
    ui->setupUi(this);

    setFixedHeight(height());
    setFixedWidth(width());
    setWindowTitle("Injetar executável dentro de arquivo .BAT");

    ui->pushButton_Search1->setEnabled(false);
    ui->lineEdit_FilePath1->setEnabled(false);

    injector = new Engine::HexInjector();

    connectEvents();

    gif = new QMovie(":/files/spin-progress.gif");
    ui->gifLabel->setMovie(gif);

    ui->statusLabel->setText("Limite de tamanho: " + QString("%1 kB").arg(injector->calculateConversionLimitKB()));
}

formInjector::~formInjector()
{
    injector->quit();
    delete ui;
}

void formInjector::handle_push_button_search_input_clicked(bool b)
{
    (void)b;

    QString fileName = QFileDialog::getOpenFileName(this,"Selecione o arquivo a ser injetado",
                                                    QString(),"Arquivos executáveis (*.exe *.dll *.pif)");

    if(!fileName.isEmpty() && !fileName.isNull())
    {
        ui->lineEdit_FilePath->setText(fileName);
    }
}

void formInjector::disable_text_box_and_button(int checkedState)
{
    if(checkedState == Qt::CheckState::Checked)
    {
        ui->lineEdit_FilePath1->setEnabled(true);
        ui->pushButton_Search1->setEnabled(true);
    } else if(checkedState == Qt::CheckState::Unchecked)
    {
        ui->lineEdit_FilePath1->setEnabled(false);
        ui->pushButton_Search1->setEnabled(false);
    }
}

void formInjector::handle_push_button_search_output_clicked(bool b)
{
    (void)b;

    QString fileName = QFileDialog::getOpenFileName(this,"Selecione o script aonde você deseja injetar",
                                                    QString(),"Script do windows (*.bat *.cmd)");

    if(!fileName.isEmpty() && !fileName.isNull())
    {
        ui->lineEdit_FilePath1->setText(fileName);
    }
}

void formInjector::handle_push_button_inject_clicked(bool b)
{
    Q_UNUSED(b);

    if(!injector->isRunning())
    {
        injector->setIOFiles(ui->lineEdit_FilePath->text(), ui->lineEdit_FilePath1->text());
        injector->start();
    }
    else {
        QMessageBox::critical(this, "Erro", "Aguarde a conversão anterior terminar");
    }
}

void formInjector::update_status(QString s)
{
    ui->statusLabel->setText(s);
}

void formInjector::stop_gif()
{
    gif->stop();
    ui->gifLabel->hide();
}

void formInjector::start_gif()
{
    ui->gifLabel->show();
    gif->start();
}

void formInjector::on_pushButton_StopInjection_clicked()
{
    if(injector->isRunning())
    {
        //injector->requestInterruption();
        update_status("Interrompendo conversão ...");
    } else {
        QMessageBox::critical(this, "Erro", "Nenhuma conversão está sendo realizada no momento");
    }
}

void formInjector::connectEvents()
{
    connect(ui->pushButton_Search, SIGNAL(clicked(bool)), this, SLOT(handle_push_button_search_input_clicked(bool)));
    connect(ui->pushButton_Search1, SIGNAL(clicked(bool)), this, SLOT(handle_push_button_search_output_clicked(bool)));
    connect(ui->pushButton_Inject, SIGNAL(clicked(bool)), this, SLOT(handle_push_button_inject_clicked(bool)));
    connect(ui->checkBox, SIGNAL(stateChanged(int)), this, SLOT(disable_text_box_and_button(int)));
    connect(injector, SIGNAL(status_updated(QString)), this, SLOT(update_status(QString)));
    connect(injector, SIGNAL(started()), this, SLOT(start_gif()));
    connect(injector, SIGNAL(stopped()), this, SLOT(stop_gif()));
    connect(ui->pushButton_StopInjection, SIGNAL(clicked()), this, SLOT(on_pushButton_StopInjection_clicked()));
}
