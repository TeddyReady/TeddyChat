#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    server = new MyServer;
    ui->IPLabel->setText("127.0.0.1");
    ui->portLabel->setText("2075");
    ui->chatField->setText(QTime::currentTime().toString()+" server started at "+server->serverAddress().toString()+":"+QString::number(server->serverPort()));
    ui->cntUsers->setText(server->getCountOfClients());
}

MainWindow::~MainWindow()
{
    delete ui;
}
