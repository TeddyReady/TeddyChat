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
    connect(server, SIGNAL(updateProgressBar(int)), this, SLOT(slotUpdateProgressBar(int)));
    ui->actionReload->setDisabled(true);
    ui->actionStop->setDisabled(true);
    ui->IPLabel->setText(server->ip);
    ui->portLabel->setText(QString::number(server->port));
    ui->progressBar->setVisible(true);
    ui->progressBar->setValue(0);
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
        //XML
        xmlData.push_back(QDate::currentDate().toString());
        xmlData.push_back(QTime::currentTime().toString());
        xmlData.push_back("Server has been deployed!");
    } else {
        ui->chatField->setTextColor(Qt::darkRed);
        ui->chatField->append(QTime::currentTime().toString() + " Fatal Error! Server cannot be deployed");
        //XML
        xmlData.push_back(QDate::currentDate().toString());
        xmlData.push_back(QTime::currentTime().toString());
        xmlData.push_back("Fatal Error! Server cannot be deployed");
    }
}
void ServerWindow::slotNewConnection(MyClient *client) {
    ui->chatField->setTextColor(Qt::darkGreen);
    ui->chatField->append(client->time + " " + client->username + " has been connected!");
    ui->cntUsers->setText(QString::number(server->clients.size()));
    ui->clientList->addItem(client->username);
    //XML
    xmlData.push_back(QDate::currentDate().toString());
    xmlData.push_back(client->time);
    xmlData.push_back(client->username + " has been connected!");
}
void ServerWindow::slotFailedValidation(){
    ui->chatField->setTextColor(Qt::darkYellow);
    ui->chatField->append(QTime::currentTime().toString() + " New client has been failed validation!");
    //XML
    xmlData.push_back(QDate::currentDate().toString());
    xmlData.push_back(QTime::currentTime().toString());
    xmlData.push_back("New client has been failed validation!");
}
void ServerWindow::slotClientDisconnected(MyClient *client) {
    ui->chatField->setTextColor(Qt::darkRed);
    ui->chatField->append(client->time + " " + client->username + " has been disconnected!");
    ui->cntUsers->setText(QString::number(server->clients.size() - 1));
    //Removing client from UI
    for(int i = 0; i < server->clients.size(); i++){
        if(client->username == ui->clientList->item(i)->text()){
            QListWidgetItem *it = ui->clientList->takeItem(i);
            delete it;
            break;
        }
    }
    //XML
    xmlData.push_back(QDate::currentDate().toString());
    xmlData.push_back(client->time);
    xmlData.push_back(client->username + " has been disconnected!");
}
void ServerWindow::slotReNameOnUI(QString name, QString newName){
    for(int i = 0; i < server->clients.size(); i++){
        if(name == ui->clientList->item(i)->text()){
            ui->clientList->item(i)->setText(newName);
            break;
        }
    }
}

void ServerWindow::slotUpdateProgressBar(int value){
    ui->progressBar->setValue(value);
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
    //XML
    xmlData.push_back(QDate::currentDate().toString());
    xmlData.push_back(QTime::currentTime().toString());
    xmlData.push_back("Reloading server...");
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
    //XML
    xmlData.push_back(QDate::currentDate().toString());
    xmlData.push_back(QTime::currentTime().toString());
    xmlData.push_back("Server has been closed!");
}
void ServerWindow::on_actionExit_triggered()
{
    statusBar()->showMessage("Please, noo...", 2500);
    DialogExit *window = new DialogExit(this);
    window->setWindowTitle("Quiting app");
    window->show();
    connect(window, SIGNAL(closeApplication()), this, SLOT(slotCloseApplication()));
}

void ServerWindow::on_actionSave_logs_to_XML_triggered()
{
    statusBar()->showMessage("Saving history...", 2500);
    QString path = QFileDialog::getSaveFileName(0, QObject::tr("Save server logs.."),
                   "/home/kataich75/qtprojects/TECH/Examples/" + QString("example").trimmed() + ".xml", QObject::tr("XML files (*.xml)"));
    connect(this, SIGNAL(savePath(QString)), this, SLOT(slotSavePath(QString)));
    if (path != "")
        emit savePath(path);
}
void ServerWindow::slotSavePath(QString path)
{
    if(path != ""){
        //Формируем XML
        general = doc.createElement("logs");
        for (int i = 0; i < xmlData.size(); i += 3){
            //Теги
            this->date = doc.createElement("date");
            this->time = doc.createElement("time");
            this->message = doc.createElement("message");
            doc.appendChild(general);
            //Сообщения между тегами
            QDomText date_t = doc.createTextNode(xmlData[i]);
            QDomText time_t = doc.createTextNode(xmlData[i+1]);
            QDomText message_t = doc.createTextNode(xmlData[i+2]);
            //Добавление в XML
            general.appendChild(this->date);
            this->date.appendChild(date_t);
            general.appendChild(this->time);
            this->time.appendChild(time_t);
            general.appendChild(this->message);
            this->message.appendChild(message_t);
        }
        //END
            file.setFileName(path);
            file.open(QIODevice::WriteOnly);
            QTextStream out(&file);
            doc.save(out, 4);
    }
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
