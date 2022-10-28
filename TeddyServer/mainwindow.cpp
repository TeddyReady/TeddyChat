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
    ui->chatField->addItem(QTime::currentTime().toString()+" server started at "+server->serverAddress().toString()+":"+QString::number(server->serverPort()));
    connect(server, SIGNAL(newConnection(QString)), this, SLOT(slotNewConnection(QString)));
    connect(server, SIGNAL(clientDisconnected(QString)), this, SLOT(slotClientDisconnected(QString)));
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::slotNewConnection(QString message) {
    ui->chatField->addItem(QTime::currentTime().toString()+ " " + message);
    ui->cntUsers->setText(server->getCountOfClients());
}

void MainWindow::slotClientDisconnected(QString message) {
    ui->chatField->addItem(message);
    ui->cntUsers->setText(server->getCountOfClients());
}
