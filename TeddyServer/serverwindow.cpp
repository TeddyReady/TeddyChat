#include "serverwindow.h"
#include "ui_serverwindow.h"

ServerWindow::ServerWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ServerWindow)
{
    ui->setupUi(this);
    server = new MyServer;
    settings = new QSettings("/home/kataich75/qtprojects/TECH/TeddyServer/other/settings.ini", QSettings::IniFormat, this);
    uploadSettings();

    connect(server, SIGNAL(serverStarted(bool)), this, SLOT(slotServerStatus(bool)));
    connect(server, SIGNAL(newConnection(MyClient *)), this, SLOT(slotNewConnection(MyClient *)));
    connect(server, SIGNAL(failedValidation()), this, SLOT(slotFailedValidation()));
    connect(server, SIGNAL(clientDisconnected(MyClient *)), this, SLOT(slotClientDisconnected(MyClient *)));
    connect(server, SIGNAL(reNameOnUI(QString, QString)), this, SLOT(slotReNameOnUI(QString, QString)));
    ui->actionReload->setDisabled(true);
    ui->actionStop->setDisabled(true);
    ui->IPLabel->setText(server->ip);
    ui->portLabel->setText(QString::number(server->port));
}
ServerWindow::~ServerWindow() {
    saveSettings();
    delete server;
    delete ui;
}

void ServerWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
    ui->actionExit->triggered();
}

void ServerWindow::uploadSettings(){
    server->ip = settings->value("ip", "127.0.0.1").toString();
    server->port = settings->value("port", "45678").toInt();
    setGeometry(settings->value("geometry", QRect(200, 400, 319, 506)).toRect());
}
void ServerWindow::saveSettings(){
    settings->setValue("ip", server->ip);
    settings->setValue("port", server->port);
    settings->setValue("geometry", geometry());
}

void ServerWindow::slotServerStatus(bool online){
    if (online) {
        ui->chatField->setTextColor(Qt::green);
        ui->chatField->append(QTime::currentTime().toString() + " Server has been deployed!");
    } else {
        ui->chatField->setTextColor(Qt::darkRed);
        ui->chatField->append(QTime::currentTime().toString() + " Fatal Error! Server cannot be deployed");
    }
}
void ServerWindow::slotNewConnection(MyClient *client) {
    ui->chatField->setTextColor(Qt::darkGreen);
    ui->chatField->append(client->time + " " + client->username + " has been connected!");
    ui->cntUsers->setText(QString::number(server->clients.size()));
    ui->clientList->addItem(client->username);
}
void ServerWindow::slotFailedValidation(){
    ui->chatField->setTextColor(Qt::darkYellow);
    ui->chatField->append(QTime::currentTime().toString() + " new client has been failed validation!");
}
void ServerWindow::slotClientDisconnected(MyClient *client) {
    ui->chatField->setTextColor(Qt::darkRed);
    ui->chatField->append(client->time + " " + client->username + " has been disconnected!");
    ui->cntUsers->setText(QString::number(server->clients.size() - 1));
    //Removing client from UI
    for(int i = server->clients.size() - 1; i >= 0; i--){
        if(client->username == ui->clientList->item(i)->text()){
            QListWidgetItem *it = ui->clientList->takeItem(i);
            delete it;
            break;
        }
    }
}
void ServerWindow::slotReNameOnUI(QString name, QString newName){
    for(int i = 0; i < server->clients.size(); i++){
        if(name == ui->clientList->item(i)->text()){
            ui->clientList->item(i)->setText(newName);
            break;
        }
    }
}

//General
void ServerWindow::on_actionDeploy_triggered()
{
    statusBar()->showMessage("Server deploying...", 2500);
    server->deployServer();
    if(server->isListening()){
        ui->actionDeploy->setDisabled(true);
        ui->actionReload->setEnabled(true);
        ui->actionStop->setEnabled(true);
        ui->actionExit->setDisabled(true);
        ui->actionNetwork->setDisabled(true);
    }
}
void ServerWindow::on_actionReload_triggered()
{
    ui->chatField->setTextColor(Qt::darkYellow);
    ui->chatField->append(QTime::currentTime().toString() + " Reloading server...");
    server->sendToClient(Commands::Restart);
}
void ServerWindow::on_actionStop_triggered()
{
    statusBar()->showMessage("Server closing...", 2500);
    server->close();
    for (int i = 0; i < server->clients.size(); i++){
        server->sendToClient(Commands::Exit, "Server", "Server close this session!");
    }
    ui->actionDeploy->setEnabled(true);
    ui->actionReload->setDisabled(true);
    ui->actionStop->setDisabled(true);
    ui->actionExit->setEnabled(true);
    ui->actionNetwork->setEnabled(true);
    ui->chatField->setTextColor(Qt::red);
    ui->chatField->append(QTime::currentTime().toString() + " Server has been closed!");
}
void ServerWindow::on_actionExit_triggered()
{
    statusBar()->showMessage("Please, noo...", 2500);
    DialogExit *window = new DialogExit(this);
    window->setWindowTitle("Quiting app");
    window->show();
    connect(window, SIGNAL(closeApplication()), this, SLOT(slotCloseApplication()));
}

void ServerWindow::slotCloseApplication(){
    if (server->clients.size()){
        ui->actionReload->triggered();
    } qApp->exit();
}
//Settings
void ServerWindow::on_actionNetwork_triggered()
{
    DialogIPPort *window = new DialogIPPort(this, server->ip, server->port);
    window->setWindowTitle("Edit Your Ip and Port");
    window->show();
    connect(window, SIGNAL(dialogIPPortParams(QString,int)), this, SLOT(slotDialogIPPortParams(QString,int)));
}
void ServerWindow::slotDialogIPPortParams(QString ip,int port){
    server->ip = ip;
    server->port = port;
    ui->IPLabel->setText(ip);
    ui->portLabel->setText(QString::number(port));
}
