#include "clientwindow.h"
#include "ui_clientwindow.h"

ClientWindow::ClientWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ClientWindow)
{
    ui->setupUi(this);
    //Загружаем настройки
    settings = new QSettings(otherPath + "settings.ini", QSettings::IniFormat, this);
    uploadSettings();

    ui->chatList->setIconSize(QSize(320, 240));
    ui->clientList->setIconSize(QSize(60, 40));
    ui->chatList->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
    ui->clientList->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);

    ui->loadProgress->setValue(0);
    ui->disconnectAct->setDisabled(true);
    ui->sendButton->setDisabled(true);
    ui->saveHistoryAct->setDisabled(true);
    ui->actionShowIP->setChecked(true);
    ui->actionShowTime->setChecked(true);
    ui->labelIP->setText(client.ip);
    ui->labelPort->setText(QString::number(client.port));
    ui->actionOther->setText(client.statusName);
    switch(client.status){
    case Status::Online:
        ui->labelStatus->setText("Online");
        ui->actionOnline->setChecked(true);
        break;
    case Status::NotInPlace:
        ui->labelStatus->setText("Not In Place");
        ui->actionNotInPlace->setChecked(true);
        break;
    case Status::NotDisturb:
        ui->labelStatus->setText("Do Not Disturb");
        ui->actionDoNotDisturb->setChecked(true);
        break;
    case Status::Other:
        ui->labelStatus->setText(client.statusName);
        ui->actionOther->setChecked(true);
        break;
    } if (showIP) {
        ui->actionShowIP->setChecked(true);
        ui->labelIP->setHidden(false);
        ui->labelPort->setHidden(false);
        ui->label_2->setHidden(false);
        ui->label_3->setHidden(false);
    } else {
        ui->actionShowIP->setChecked(false);
        ui->labelIP->setHidden(true);
        ui->labelPort->setHidden(true);
        ui->label_2->setHidden(true);
        ui->label_3->setHidden(true);
    }
    if (showTime) ui->actionShowTime->setChecked(true);
    else ui->actionShowTime->setChecked(false);
    if (ui->actionShowLB->isChecked()) ui->loadProgress->setVisible(true);
    else ui->loadProgress->setHidden(true);

    client.socket = new QSslSocket(this);
    connect(client.socket, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
    connect(client.socket, SIGNAL(connected()), this, SLOT(slotSocketConnected()));
    connect(client.socket, SIGNAL(disconnected()), this, SLOT(slotSocketDisconnected()));
    connect(client.socket, SIGNAL(encrypted()), this, SLOT(slotEncrypted()));
    setWindowTitle(client.username);
    dataSize = 0;
    isConnected = false;

    //Контекстное меню списка клиентов
    ui->clientList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->clientList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));

    //Контекстное меню на поле сообщений
    ui->chatList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->chatList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenuOnMessageField(QPoint)));

    //Контекстное меню на кнопке
    ui->sendButton->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->sendButton, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenuOnSendButton(QPoint)));
}

ClientWindow::~ClientWindow()
{
    saveSettings();
    file.close();
    if (!ui->actionAutosave->isChecked()) {
        for (int i = 0; i < fileNames.size(); i++) {
            QFile curFile(fileNames[i]);
            curFile.remove();
        }
    } fileNames.clear();
    delete settings;
    delete ui;
}

void ClientWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
    ui->quitAct->triggered();
}

void ClientWindow::uploadSettings(){
    client.ip = settings->value("ip", "127.0.0.1").toString();
    client.port = settings->value("port", "45678").toInt();
    client.username = settings->value("username", "Unknown User").toString();
    client.status = settings->value("status", Status::Online).toInt();
    client.statusName = settings->value("customStatus", "Working on Lab").toString();
    client.path = settings->value("avatarPath", ":/new/prefix1/other/client.png").toString();
    setGeometry(settings->value("geometry", QRect(200, 200, 530, 388)).toRect());
    //Цвет
    windowColor.setNamedColor(settings->value("backgroundColor", "white").toString());
    pal.setColor(QPalette::Background, windowColor);
    setAutoFillBackground(true);
    setPalette(pal);
    myMsgColor.setNamedColor(settings->value("clientMsgColor", "black").toString());
    otherMsgColor.setNamedColor(settings->value("otherMsgColor", "blue").toString());
    client.colorName = settings->value("clientProfileColor", "white").toString();
    //UI
    showIP = settings->value("showModeIP", "1").toBool();
    showTime = settings->value("showModeTime", "1").toBool();
    ui->actionAutosave->setChecked(settings->value("fileAutoSave", "1").toBool());
    ui->actionShowLB->setChecked(settings->value("showLoadBar", "1").toBool());
}
void ClientWindow::saveSettings(){
    settings->setValue("ip", client.ip);
    settings->setValue("port", client.port);
    settings->setValue("username", client.username);
    settings->setValue("status", client.status);
    settings->setValue("customStatus", client.statusName);
    settings->setValue("avatarPath", client.path);
    settings->setValue("geometry", geometry());
    settings->setValue("backgroundColor", windowColor.name());
    settings->setValue("clientMsgColor", myMsgColor.name());
    settings->setValue("otherMsgColor", otherMsgColor.name());
    settings->setValue("clientProfileColor", client.colorName);
    settings->setValue("showModeIP", QString::number(ui->actionShowIP->isChecked()));
    settings->setValue("showModeTime", QString::number(ui->actionShowTime->isChecked()));
    settings->setValue("fileAutoSave", QString::number(ui->actionAutosave->isChecked()));
    settings->setValue("showLoadBar", QString::number(ui->actionShowLB->isChecked()));
}

void ClientWindow::sendToServer(int command, QString message, int option)
{
    QByteArray data; data.clear();
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_12);
    out << static_cast<quint64>(0) << (quint8)command;
    if(command == Commands::SendMessage) {
        out << client.username << message;
        //Set receivers
        int cnt = 0;
        for (int i = 0; i < includedClients.size(); i++) {
            if (includedClients[i].second) {
                cnt++;
            }
        } out << cnt;
        for (int i = 0; i < includedClients.size(); i++) {
            if (includedClients[i].second) {
                out << includedClients[i].first->username;
            }
        }
    } else if (command == Commands::Authentication) {
        out << client.ip << QString::number(client.port)
            << client.username << QString::number(client.status)
            << client.date << client.time << client.path
            << client.statusName << client.colorName;
    } else if (command == Commands::Exit) {
        out << client.username;
    } else if (command == Commands::DataChanged) {
        out << client.username;
        if (option == Status::Other || option == Commands::PathChanged || option == Commands::ColorChanged) {
            out << QString::number(option);
        }  out << message;
    } else if (command == Commands::SendImage) {
        out << client.username << message;
        //Set receivers
        int cnt = 0;
        for (int i = 0; i < includedClients.size(); i++) {
            if (includedClients[i].second) {
                cnt++;
            }
        } out << cnt;
        for (int i = 0; i < includedClients.size(); i++) {
            if (includedClients[i].second) {
                out << includedClients[i].first->username;
            }
        }
    } else if (command == Commands::SendFile) {
        QFile sendingFile(message);
        QString fileName(message.split("/").last());
        QDataStream fileRead(&sendingFile);
        fileRead.setVersion(QDataStream::Qt_5_12);
        if (sendingFile.open(QIODevice::ReadOnly)) {
            //Отправляем размер и имя файла
            out << client.username << static_cast<quint64>(sendingFile.size()) << fileName;
            //Set receivers
            int cnt = 0;
            for (int i = 0; i < includedClients.size(); i++) {
                if (includedClients[i].second) {
                    cnt++;
                }
            } out << cnt;
            for (int i = 0; i < includedClients.size(); i++) {
                if (includedClients[i].second) {
                    out << includedClients[i].first->username;
                }
            }
            out.device()->seek(0);
            out << (quint64)(data.size() - sizeof(quint64));
            client.socket->write(data);
            client.socket->waitForReadyRead(100);
            //Отправляем сам файл
            quint64 curSendedSize = 0;
            char bytes[8]; bytes[7] = '\0';
            while (curSendedSize < static_cast<quint64>(sendingFile.size())) {
                int lenght = fileRead.readRawData(bytes, sizeof(char) * 7);
                data.setRawData(bytes, sizeof(char) * lenght);
                curSendedSize += client.socket->write(data, sizeof(char) * lenght);
                client.socket->waitForReadyRead(100);
                qDebug() << curSendedSize << " " << static_cast<quint64>(sendingFile.size()) << " |" << lenght;
                //Обновляем прогресс отправки
                ui->loadProgress->setValue(static_cast<int>(static_cast<qreal>(curSendedSize) / static_cast<qreal>(sendingFile.size()) * 100));
            }
            sendingFile.close();
            QFile::copy(sendingFile.fileName() ,downloadPath + fileName);
            fileNames.push_back(downloadPath + fileName);
            QListWidgetItem *item;
            if (showTime) {
                if (showIP)
                    item = new QListWidgetItem(QTime::currentTime().toString() + " & " + client.ip + "/ You send a file: " + fileName, ui->chatList);
                else item = new QListWidgetItem(QTime::currentTime().toString() + "/ You send a file: " + fileName, ui->chatList);
            } else {
                if (showIP)
                    item = new QListWidgetItem(client.ip + "/ You send a file: " + fileName, ui->chatList);
                else item = new QListWidgetItem("You send a file: " + fileName, ui->chatList);
            }
            item->setForeground(myMsgColor);
            ui->chatList->addItem(item);
        } return;
    }
    out.device()->seek(0);
    out << (quint64)(data.size() - sizeof(quint64));
    client.socket->write(data);
}
void ClientWindow::slotReadyRead()
{
    //PACKET_DEFENCER
    QDataStream in(client.socket);
    if (dataSize == 0) {
        if (static_cast<size_t>(client.socket->bytesAvailable()) < sizeof(quint64)) return;
        in >> dataSize;
    }
    if (static_cast<quint64>(client.socket->bytesAvailable()) < dataSize)
        return;
    else dataSize = 0;
    //END
    quint8 command;
    in >> command;
    QListWidgetItem *item;
    if(static_cast<int>(command) == Commands::SendMessage) {
        QString name, message, date, time;
        in >> name >> message >> date >> time;
        if(name == client.username) {
            if (showTime) {
                if (showIP)
                    item = new QListWidgetItem(time + " & " + client.ip + "/ You: " + message, ui->chatList);
                else item = new QListWidgetItem(time + "/ You: " + message, ui->chatList);
            } else {
                if (showIP)
                    item = new QListWidgetItem(client.ip + "/ You: " + message, ui->chatList);
                else item = new QListWidgetItem("You: " + message, ui->chatList);
            }
            item->setForeground(myMsgColor);
            ui->chatList->addItem(item);
        } else {
            if (client.status != Status::NotDisturb)
                QSound::play(otherPath + "newmessage.wav");
            if (showTime) {
                if (showIP)
                    item = new QListWidgetItem(time + " & " + client.ip + "/ " + name + ": " + message, ui->chatList);
                else item = new QListWidgetItem(time + "/ " + name + ": " + message, ui->chatList);
            } else {
                if (showIP)
                    item = new QListWidgetItem(client.ip + "/ " + name + ": " + message, ui->chatList);
                else item = new QListWidgetItem(name + ": " + message, ui->chatList);
            }
            item->setForeground(otherMsgColor);
            ui->chatList->addItem(item);
        }
        //Добавляем данные для XML
        xmlData.push_back(date);
        xmlData.push_back(time);
        xmlData.push_back(client.ip);
        xmlData.push_back(name);
        xmlData.push_back(message);

    } else if (static_cast<int>(command) == Commands::Authentication) {
        QString name, message;
        in >> name >> message;
        if(name == client.username) {
            item = new QListWidgetItem(message, ui->chatList);
            item->setForeground(Qt::darkGreen);
            ui->chatList->addItem(item);
            ui->saveHistoryAct->setEnabled(true);
            sendToServer(Commands::UpdateDataBase);
        } else {
            item = new QListWidgetItem(message, ui->chatList);
            item->setForeground(Qt::green);
            ui->chatList->addItem(item);
        }

    } else if (static_cast<int>(command) == Commands::Exit) {
        QString name, message;
        in >> name >> message;
        if (name == "Server"){
            item = new QListWidgetItem(message, ui->chatList);
            item->setForeground(Qt::darkRed);
            ui->chatList->addItem(item);
        } else if(name != client.username){
            item = new QListWidgetItem(message, ui->chatList);
            item->setForeground(Qt::darkRed);
            ui->chatList->addItem(item);
            for(int i = 0; i < includedClients.size(); i++){
                if(name == ui->clientList->item(i)->text()){
                    includedClients.removeAt(i);
                    QListWidgetItem *it = ui->clientList->takeItem(i);
                    delete it;
                    break;
                }
            }
        }

    } else if (static_cast<int>(command) == Commands::UpdateDataBase) {
        quint8 countOfClients;
        in >> countOfClients;
        for (int i = 0; i < (int)countOfClients; i++) {
            QString name, status, date, time, path, customStatus, colorName;
            in >> name >> status >> date >> time >> path >> customStatus >> colorName;
            MyClient *ptr = new MyClient("127.0.0.1", 45678, name, status.toInt(), date, time, nullptr, path, customStatus, colorName);
            includedClients.push_back(std::make_pair(ptr, true));
            item = new QListWidgetItem(QIcon(path), name, ui->clientList);
            ui->clientList->addItem(item);
        }

    } else if (static_cast<int>(command) == Commands::NewClient) {
        QString name, status, date, time, path, customStatus, colorName;
        in >> name >> status >> date >> time >> path >> customStatus >> colorName;
        MyClient *ptr = new MyClient("127.0.0.1", 45678, name, status.toInt(), date, time, nullptr, path, customStatus, colorName);
        includedClients.push_back(std::make_pair(ptr, true));
        item = new QListWidgetItem(QIcon(path), name, ui->clientList);
        ui->clientList->addItem(item);

    } else if (static_cast<int>(command) == Commands::Restart) {
        statusBar()->showMessage("Server reloading...", 2500);
        item = new QListWidgetItem(client.username + " has been disconnected for reload server!", ui->chatList);
        item->setForeground(Qt::darkRed);
        ui->chatList->addItem(item);
        item->setForeground(Qt::darkYellow);
        item->setText("Server has been reloaded!");
        ui->chatList->addItem(item);
        client.socket->disconnectFromHost();

    } else if (static_cast<int>(command) == Commands::DataChanged) {
        QString name, message; in >> name >> message;
        if (message == QString::number(Status::Online) ||
        message == QString::number(Status::NotInPlace) ||
        message == QString::number(Status::NotDisturb)) {
            if (name == client.username) {
                item = new QListWidgetItem("Server: Your status has been updated!", ui->chatList);
                item->setForeground(Qt::cyan);
                ui->chatList->addItem(item);
            } else {
                for (std::pair<MyClient *, bool> ptr: includedClients) {
                    if (ptr.first->username == name) {
                        ptr.first->status = message.toInt();
                        item = new QListWidgetItem("Server: " + name + " status has been updated!", ui->chatList);
                        item->setForeground(Qt::darkCyan);
                        ui->chatList->addItem(item);
                        break;
                    }
                }
            }
        } else if (message == QString::number(Status::Other)) {
            QString customStatus; in >> customStatus;
            if (name == client.username) {
                item = new QListWidgetItem("Server: Your status has been updated for " + customStatus + "!", ui->chatList);
                item->setForeground(Qt::cyan);
                ui->chatList->addItem(item);
            } else {
                for (std::pair<MyClient *, bool> ptr: includedClients) {
                    if (ptr.first->username == name) {
                        ptr.first->status = message.toInt();
                        ptr.first->statusName = customStatus;
                        item = new QListWidgetItem("Server: " + name + " status has been updated for " + customStatus + "!", ui->chatList);
                        item->setForeground(Qt::darkCyan);
                        ui->chatList->addItem(item);
                        break;
                    }
                }
            }
        } else if (message == QString::number(Commands::PathChanged)) {
            QString path; in >> path;
            if (name == client.username) {
                item = new QListWidgetItem("Server: Your avatar has been updated!", ui->chatList);
                item->setForeground(Qt::cyan);
                ui->chatList->addItem(item);
            } else {
                for (std::pair<MyClient *, bool> ptr: includedClients) {
                    if (ptr.first->username == name) {
                        ptr.first->path = path;
                        for(int i = 0; i < includedClients.size(); i++){
                            if(name == ui->clientList->item(i)->text()){
                                ui->clientList->item(i)->setIcon(QIcon(path));
                                break;
                            }
                        }
                        item = new QListWidgetItem("Server: " + name + " avatar has been updated!", ui->chatList);
                        item->setForeground(Qt::darkCyan);
                        ui->chatList->addItem(item);
                        break;
                    }
                }
            }
        } else if (message == QString::number(Commands::ColorChanged)) {
            QString colorName; in >> colorName;
            QColor tmp; tmp.setNamedColor(colorName);
            if (name == client.username) {
                item = new QListWidgetItem("Server: Your background color has been updated!", ui->chatList);
                item->setForeground(tmp);
                ui->chatList->addItem(item);
            } else {
                for (std::pair<MyClient *, bool> ptr: includedClients) {
                    if (ptr.first->username == name) {
                        ptr.first->colorName = colorName;
                        item = new QListWidgetItem("Server: " + name + " background color has been updated!", ui->chatList);
                        item->setForeground(tmp);
                        ui->chatList->addItem(item);
                        break;
                    }
                }
            }
        } else {
            if (name == client.username) {
                client.username = message;
                setWindowTitle(client.username);
                item = new QListWidgetItem("Server: Your name has been updated for " + message + "!", ui->chatList);
                item->setForeground(Qt::cyan);
                ui->chatList->addItem(item);
            } else {
                for (std::pair<MyClient *, bool> ptr: includedClients) {
                    if (ptr.first->username == name) {
                        ptr.first->username = message;
                        for(int i = 0; i < includedClients.size(); i++){
                            if(name == ui->clientList->item(i)->text()){
                                ui->clientList->item(i)->setText(message);
                                break;
                            }
                        }
                        item = new QListWidgetItem("Server: " + name + " name has been updated for " + message + "!", ui->chatList);
                        item->setForeground(Qt::darkCyan);
                        ui->chatList->addItem(item);
                        break;
                    }
                }
            }
        }
    } else if (static_cast<int>(command) == Commands::SendImage) {
        QString name, message, date, time;
        in >> name >> message >> date >> time;
        QPixmap image; image.load(message);
        if(name == client.username) {
            if (showTime) {
                if (showIP)
                    item = new QListWidgetItem(time + " & " + client.ip + "/ You send image:", ui->chatList);
                else item = new QListWidgetItem(time + "/ You send image:", ui->chatList);
            } else {
                if (showIP)
                    item = new QListWidgetItem(client.ip + "/ You send image:", ui->chatList);
                else item = new QListWidgetItem("You send image:", ui->chatList);
            }
            item->setForeground(myMsgColor);
            ui->chatList->addItem(item);
            item = new QListWidgetItem(QIcon(image), "", ui->chatList);
            ui->chatList->addItem(item);
        } else {
            if (client.status != Status::NotDisturb){
                QSound::play(otherPath + "newmessage.wav");
            }
            if (showTime) {
                if (showIP)
                    item = new QListWidgetItem(time + " & " + client.ip + "/ " + name + " send image:", ui->chatList);
                else item = new QListWidgetItem(time + "/ " + name + " send image:", ui->chatList);
            } else {
                if (showIP)
                    item = new QListWidgetItem(client.ip + "/ " + name + " send image:", ui->chatList);
                else item = new QListWidgetItem(name + " send image:", ui->chatList);
            }
            item->setForeground(otherMsgColor);
            ui->chatList->addItem(item);
            item = new QListWidgetItem(QIcon(image), "", ui->chatList);
            ui->chatList->addItem(item);
        }
        //Конвертация в Base64
        QImage img(QIcon(image).pixmap(20, 20).toImage());
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        img.save(&buffer, "PNG");
        QString base64 = QString::fromLatin1(byteArray.toBase64().data());
        //Добавляем данные для XML
        xmlData.push_back(date);
        xmlData.push_back(time);
        xmlData.push_back(client.ip);
        xmlData.push_back(name);
        xmlData.push_back(base64);
    } else if (static_cast<int>(command) == Commands::ForceQuit) {
        item = new QListWidgetItem("Validation failed, your name already used!", ui->chatList);
        item->setForeground(Qt::darkYellow);
        ui->chatList->addItem(item);
        isConnected = false;
        client.socket->disconnectFromHost();
    } else if (static_cast<int>(command) == Commands::ForbiddenName) {
        item = new QListWidgetItem("Cannot change username, this name already used!", ui->chatList);
        item->setForeground(Qt::darkYellow);
        ui->chatList->addItem(item);
    } else if (static_cast<int>(command) == Commands::FileAccepted) {
        quint64 fileSize, curFileSize = 0; QString name, time, fileName;
        in >> name >> time >> fileSize >> fileName;
        sendToServer(Commands::Ready);
        QFile receivedFile(downloadPath + fileName);
        if (receivedFile.open(QFile::WriteOnly)) {
            QDataStream fileWrite(&receivedFile);
            fileWrite.setVersion(QDataStream::Qt_5_12);
            while (curFileSize < fileSize) {
                sendToServer(Commands::Ready);
                QByteArray packet = client.socket->readAll();
                curFileSize += fileWrite.writeRawData(packet.data(), packet.size());
                client.socket->waitForReadyRead(100);
                qDebug() << curFileSize << " vs " << fileSize << " |" << packet.size();
                ui->loadProgress->setValue(static_cast<int>(static_cast<qreal>(curFileSize) / static_cast<qreal>(fileSize) * 100));
            } receivedFile.close(); fileNames.push_back(receivedFile.fileName());
        } if (client.status != Status::NotDisturb){
            QSound::play(otherPath + "newmessage.wav");
        }
        if (showTime) {
            if (showIP)
                item = new QListWidgetItem(time + " & " + client.ip + "/ " + name + " sended a file: " + fileName, ui->chatList);
            else item = new QListWidgetItem(time + "/ " + name + " sended a file: " + fileName, ui->chatList);
        } else {
            if (showIP)
                item = new QListWidgetItem(client.ip + "/ " + name + " sended a file: " + fileName, ui->chatList);
            else item = new QListWidgetItem(name + " sended a file: " + fileName, ui->chatList);
        }
        item->setForeground(otherMsgColor);
        ui->chatList->addItem(item);
        QFile file(receivedFile.fileName());
        if (file.open(QIODevice::ReadOnly)) {
            QString fileData = QString::fromLatin1(file.readAll().toBase64().data());
            //XML
            xmlData.push_back(QDate::currentDate().toString());
            xmlData.push_back(time);
            xmlData.push_back(client.ip);
            xmlData.push_back(name);
            xmlData.push_back(QString::fromStdString(md5(fileData.toStdString())));
        } file.close();
    }
}

void ClientWindow::slotEncrypted(){
    isConnected = true;
    client.date = QDate::currentDate().toString();
    client.time = QTime::currentTime().toString();
    sendToServer(Commands::Authentication);
}

//Меню "General"
void ClientWindow::on_connectAct_triggered()
{
    statusBar()->showMessage("Connect to server...", 2500);
    client.socket->setPeerVerifyMode(QSslSocket::VerifyNone);
    client.socket->connectToHostEncrypted(client.ip, client.port);
}
void ClientWindow::slotSocketConnected()
{
    ui->connectAct->setDisabled(true);
    ui->disconnectAct->setEnabled(true);
    ui->quitAct->setDisabled(true);
    ui->sendButton->setEnabled(true);
    ui->ipPortAct->setDisabled(true);
    ui->presets->setDisabled(true);
}

void ClientWindow::on_disconnectAct_triggered()
{
    isConnected = false;
    statusBar()->showMessage("Disconnect from server...", 2500);
    sendToServer(Commands::Exit);
    client.socket->disconnectFromHost();
}
void ClientWindow::slotSocketDisconnected()
{
    ui->connectAct->setEnabled(true);
    ui->disconnectAct->setDisabled(true);
    ui->quitAct->setEnabled(true);
    ui->sendButton->setDisabled(true);
    ui->ipPortAct->setEnabled(true);
    ui->clientList->clear();
    ui->presets->setEnabled(true);
    QListWidgetItem *item = new QListWidgetItem("You has been disconnected from server!", ui->chatList);
    item->setForeground(Qt::red);
    ui->chatList->addItem(item);
}

void ClientWindow::on_saveHistoryAct_triggered()
{
    statusBar()->showMessage("Saving history...", 2500);
    DialogXML *window = new DialogXML(this);
    window->setWindowTitle("Save your chat history");
    window->show();
    connect(window, SIGNAL(savePath(QString, quint8)), this, SLOT(slotSavePath(QString, quint8)));
}
void ClientWindow::slotSavePath(QString path, quint8 pass)
{
    if(path != ""){
        //Шифрование
        Cipher crypto(pass);
        //Формируем XML
        general = doc.createElement("history");
        for (int i = 0; i < xmlData.size(); i += 5){
            //Теги
            this->date = doc.createElement("date");
            this->time = doc.createElement("time");
            this->ip = doc.createElement("ip");
            this->name = doc.createElement("username");
            this->message = doc.createElement("message");
            doc.appendChild(general);
            //Сообщения между тегами
            QDomText date_t = doc.createTextNode(crypto.encryptToString(xmlData[i]));
            QDomText time_t = doc.createTextNode(crypto.encryptToString(xmlData[i+1]));
            QDomText ip_t = doc.createTextNode(crypto.encryptToString(xmlData[i+2]));
            QDomText name_t = doc.createTextNode(crypto.encryptToString(xmlData[i+3]));
            QDomText message_t = doc.createTextNode(crypto.encryptToString(xmlData[i+4]));
            //Добавление в XML
            general.appendChild(this->date);
            this->date.appendChild(date_t);
            general.appendChild(this->time);
            this->time.appendChild(time_t);
            general.appendChild(this->ip);
            this->ip.appendChild(ip_t);
            general.appendChild(this->name);
            this->name.appendChild(name_t);
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

void ClientWindow::on_actionReadXML_triggered()
{
    QString path = QFileDialog::getOpenFileName(0, QObject::tr("Select xml file"),
                   downloadPath, QObject::tr("XML files (*.xml)"));
    if (path != "") {
        DialogPassword *window = new DialogPassword(this, path);
        connect(window, SIGNAL(passwordReceived(QString, quint8, bool)), this, SLOT(slotPasswordReceived(QString, quint8, bool)));
        window->setWindowTitle("Input password for this file");
        window->show();
    }
}
void ClientWindow::slotPasswordReceived(QString path, quint8 pass, bool encrypted){
    DialogXMLReader *window = new DialogXMLReader(this, path, pass, encrypted);
    window->setWindowTitle("Your selected history");
    window->show();
}

void ClientWindow::on_quitAct_triggered()
{
    statusBar()->showMessage("Please, noo...", 2500);
    DialogExit *window = new DialogExit(this);
    window->setWindowTitle("Quiting app");
    window->show();
    connect(window, SIGNAL(closeApplication()), this, SLOT(slotCloseApplication()));
}
void ClientWindow::slotCloseApplication(){
    if (isConnected) {
        ui->disconnectAct->triggered();
        client.socket->waitForReadyRead();
    }
    qApp->exit();
}
//Меню "View"
void ClientWindow::on_actionBackColor_triggered()
{
    statusBar()->showMessage("Set your background color...", 2500);
    QColor newColor = QColorDialog::getColor(windowColor, parentWidget());
    if ( newColor != windowColor ){
        windowColor = newColor;
        pal.setColor(QPalette::Background, windowColor);
        setPalette(pal);
    }
}

void ClientWindow::on_actionMyColorMsg_triggered()
{
    statusBar()->showMessage("Setting your messages color...", 2500);
    QColor newColor = QColorDialog::getColor(myMsgColor, parentWidget());
    if ( newColor != myMsgColor ){
        myMsgColor = newColor;
    }
}
void ClientWindow::on_actionOtherColorMsg_triggered()
{
    statusBar()->showMessage("Setting your friends messages color...", 2500);
    QColor newColor = QColorDialog::getColor(otherMsgColor, parentWidget());
    if ( newColor != otherMsgColor ){
        otherMsgColor = newColor;
    }
}
void ClientWindow::on_actionProfileColor_triggered()
{
    statusBar()->showMessage("Modify yourself...", 2500);
    QColor tmp, newColor; tmp.setNamedColor(client.colorName);
    newColor = QColorDialog::getColor(tmp, parentWidget());
    if ( newColor != tmp ){
        client.colorName = newColor.name();
        sendToServer(Commands::DataChanged, client.colorName, Commands::ColorChanged);
    }
}

void ClientWindow::on_actionShowIP_triggered()
{
    statusBar()->showMessage("Set visible mode of your ip...", 2500);
    if (!ui->actionShowIP->isChecked()) {
        showIP = false;
        ui->labelIP->setHidden(true);
        ui->labelPort->setHidden(true);
        ui->label_2->setHidden(true);
        ui->label_3->setHidden(true);
    }
    else {
        showIP = true;
        ui->labelIP->setHidden(false);
        ui->labelPort->setHidden(false);
        ui->label_2->setHidden(false);
        ui->label_3->setHidden(false);
    }
}
void ClientWindow::on_actionShowTime_triggered()
{
    statusBar()->showMessage("Set visible mode of your time...", 2500);
    if (!ui->actionShowTime->isChecked()) showTime = false;
    else showTime = true;
}
void ClientWindow::on_actionShowLB_triggered()
{
    if (ui->actionShowLB->isChecked())
        ui->loadProgress->setVisible(true);
    else ui->loadProgress->setHidden(true);
}
//Меню "Settings"
void ClientWindow::on_ipPortAct_triggered()
{
    statusBar()->showMessage("Changing your ip & port...", 2500);
    DialogIPPort *window = new DialogIPPort(this, client.ip, client.port);
    window->setWindowTitle("Edit Your Ip and Port");
    window->show();
    connect(window, SIGNAL(dialogIPPortParams(QString,int)), this, SLOT(slotDialogIPPortParams(QString,int)));
}
void ClientWindow::slotDialogIPPortParams(QString ip, int port)
{
    client.ip = ip;
    client.port = port;
    ui->labelIP->setText(ip);
    ui->labelPort->setText(QString::number(port));
}

void ClientWindow::on_actionAvatar_triggered()
{
    QString path = QFileDialog::getOpenFileName(0, QObject::tr("Select photo"),
        otherPath, QObject::tr("Image files (*.png)"));
    if (path != "") {
        client.path = path;
        if (isConnected) sendToServer(Commands::DataChanged, path, Commands::PathChanged);
    }
}

void ClientWindow::on_nameAct_triggered()
{
    statusBar()->showMessage("Changing your name...", 2500);
    DialogUserName *window = new DialogUserName(this, client.username);
    window->setWindowTitle("Edit Your Name");
    window->show();
    connect(window, SIGNAL(dialogUserNameParams(QString)), this, SLOT(slotDialogUserNameParams(QString)));
}
void ClientWindow::slotDialogUserNameParams(QString username)
{
    if (isConnected)
        sendToServer(Commands::DataChanged, username);
    else {
        setWindowTitle(username);
        client.username = username;
    }
}

void ClientWindow::on_actionOnline_triggered()
{
    statusBar()->showMessage("Now you are Online!", 2500);
    if (ui->actionOnline->isChecked()) {
        ui->actionNotInPlace->setChecked(false);
        ui->actionDoNotDisturb->setChecked(false);
        ui->actionOther->setChecked(false);
        client.status = Status::Online;
        ui->labelStatus->setText("Online");
        if (isConnected)
            sendToServer(Commands::DataChanged, QString::number(Status::Online));
    } else ui->actionOnline->setChecked(true);
}
void ClientWindow::on_actionNotInPlace_triggered()
{
    statusBar()->showMessage("Now you are Not In Place!", 2500);
    if (ui->actionNotInPlace->isChecked()) {
        ui->actionOnline->setChecked(false);
        ui->actionDoNotDisturb->setChecked(false);
        ui->actionOther->setChecked(false);
        client.status = Status::NotInPlace;
        ui->labelStatus->setText("Not In Place");
        if (isConnected)
            sendToServer(Commands::DataChanged, QString::number(Status::NotInPlace));
    } else ui->actionNotInPlace->setChecked(true);
}
void ClientWindow::on_actionDoNotDisturb_triggered()
{
    statusBar()->showMessage("Now you are Not Disturb!", 2500);
    if (ui->actionDoNotDisturb->isChecked()) {
        ui->actionOnline->setChecked(false);
        ui->actionNotInPlace->setChecked(false);
        ui->actionOther->setChecked(false);
        client.status = Status::NotDisturb;
        ui->labelStatus->setText("Do Not Disturb");
        if (isConnected)
            sendToServer(Commands::DataChanged, QString::number(Status::NotDisturb));
    } else ui->actionDoNotDisturb->setChecked(true);
}

void ClientWindow::on_actionOther_triggered()
{
    ui->actionOther->setChecked(true);
    statusBar()->showMessage("Creating your status...", 2500);
    DialogOtherStatus *window = new DialogOtherStatus(this, client.statusName);
    window->setWindowTitle("Create your status");
    window->show();
    connect(window, SIGNAL(dialogOtherStatusParams(QString)), this, SLOT(slotDialogOtherStatusParams(QString)));
}
void ClientWindow::slotDialogOtherStatusParams(QString customStatus){
    statusBar()->showMessage("Now you are " + customStatus + "!", 2500);
    ui->actionOnline->setChecked(false);
    ui->actionNotInPlace->setChecked(false);
    ui->actionDoNotDisturb->setChecked(false);
    ui->actionOther->setChecked(true);
    client.status = Status::Other;
    client.statusName = customStatus;
    ui->labelStatus->setText(customStatus);
    ui->actionOther->setText(customStatus);
    if (isConnected)
        sendToServer(Commands::DataChanged, customStatus, Status::Other);
}
//Меню "Help"
void ClientWindow::on_appAct_triggered()
{
    statusBar()->showMessage("Seems like a really good man...", 2500);
    DialogAboutAutor *window = new DialogAboutAutor(this);
    window->setWindowTitle("About Application");
    window->show();
}

//UI Buttons
void ClientWindow::on_profileButton_clicked()
{
    statusBar()->showMessage("Showing your profile...", 2500);
    DialogAboutClient *window = new DialogAboutClient(this, client);
    window->show();
}

void ClientWindow::on_sendButton_clicked()
{
    statusBar()->showMessage("Sending message...", 2500);
    sendToServer(Commands::SendMessage, ui->messageField->text());
    ui->messageField->clear();
}
void ClientWindow::on_messageField_returnPressed()
{
    statusBar()->showMessage("Sending message...", 2500);
    sendToServer(Commands::SendMessage, ui->messageField->text());
    ui->messageField->clear();
}

//UI ClientList
void ClientWindow::showContextMenu(QPoint pos)
{
    if(!includedClients.isEmpty() && ui->clientList->currentItem() != nullptr){
        QMenu *menu = new QMenu(this);
        QAction *infoAct = new QAction("Info about...");
        QAction *sendAct = new QAction("Send data");
        sendAct->setCheckable(true);
        for (int i = 0; i < includedClients.size(); i++) {
            if (ui->clientList->currentItem()->text() == includedClients[i].first->username) {
                if (includedClients[i].second) {
                    sendAct->setChecked(true);
                    break;
                } else {
                    sendAct->setChecked(false);
                    break;
                }
            }
        }
        connect(infoAct, SIGNAL(triggered()), this, SLOT(slotInfoAbout()));
        menu->addAction(infoAct);
        connect(sendAct, &QAction::triggered, [=]() {
            for (int i = 0; i < includedClients.size(); i++) {
                if (ui->clientList->currentItem()->text() == includedClients[i].first->username) {
                    if (sendAct->isChecked()) {
                        includedClients[i].second = true;
                        sendAct->setChecked(true);
                        break;
                    } else {
                        includedClients[i].second = false;
                        sendAct->setChecked(false);
                        break;
                    }
                }
            }
        });
        menu->addAction(infoAct);
        menu->addAction(sendAct);
        menu->popup(ui->clientList->viewport()->mapToGlobal(pos));
    }
}
void ClientWindow::slotInfoAbout(){
    for (int i = 0; i < includedClients.size(); i++){
        if (ui->clientList->currentItem()->text() == includedClients[i].first->username) {
            DialogAboutClient *window = new DialogAboutClient(this, *includedClients[i].first);
            window->setWindowTitle("About " + includedClients[i].first->username);
            window->show();
            break;
        }
    }
}

void ClientWindow::showContextMenuOnMessageField(QPoint pos)
{
    if (ui->chatList->count()) {
        QMenu *menu = new QMenu(this);
        QAction *clearAct = new QAction("Clear");
        QAction *showAct = new QAction("Show full-size image");
        QAction *fileAct = new QAction("Save file");
        QAction *openAct = new QAction("Open file...");
        connect(clearAct, &QAction::triggered, [this]() {
                ui->chatList->clear();
                xmlData.clear();
            });
        connect(showAct, &QAction::triggered, [=]() {
                    DialogImage *window = new DialogImage(this, ui->chatList->currentItem()->icon().
                        pixmap(ui->chatList->currentItem()->icon().actualSize(QSize(1920, 1080))));
                    window->setWindowTitle("About image");
                    window->show();
                    connect(window, SIGNAL(saveImage(QString, const QPixmap *)), this, SLOT(slotSaveImage(QString, const QPixmap*)));
            });
        connect(fileAct, &QAction::triggered, [=]() {
            QString fileName = ui->chatList->currentItem()->text().split(":").last();
            for (int i = 0; i < fileNames.size(); i++) {
                if (fileNames[i].split("/").last() == fileName) {
                    fileNames.removeAt(i);
                    break;
                }
            }
        });
        connect(openAct, &QAction::triggered, [=]() {
            QString fileName = ui->chatList->currentItem()->text().split(": ").last();
            for (int i = 0; i < fileNames.size(); i++) {
                if (fileNames[i].split("/").last() == fileName) {
                    QFile tmp(fileNames[i]);
                    QDesktopServices::openUrl(QUrl::fromLocalFile(tmp.fileName()));
                    break;
                }
            }
        });
        menu->addAction(clearAct);
        if (ui->chatList->currentItem()->text() == "")
            menu->addAction(showAct);
        for (int i = 0; i < fileNames.size(); i++) {
            if (fileNames[i].split("/").last() == ui->chatList->currentItem()->text().split(": ").last()) {
                menu->addAction(fileAct);
                menu->addAction(openAct);
                break;
            }
        }
        menu->popup(ui->chatList->viewport()->mapToGlobal(pos));
    }
}
void ClientWindow::slotSaveImage(QString path, const QPixmap *image){
    file.setFileName(path);
    file.open(QIODevice::WriteOnly);
    image->save(&file, "PNG");
    file.close();
}

void ClientWindow::showContextMenuOnSendButton(QPoint pos)
{
    QMenu *btnMenu = new QMenu;
    QAction *sendPic = new QAction("Send image...");
    connect(sendPic, SIGNAL(triggered()), this, SLOT(slotSendPicture()));
    QAction *sendFile = new QAction("Send file...");
    connect(sendFile, SIGNAL(triggered()), this, SLOT(slotSendFile()));

    btnMenu->addAction(sendPic);
    btnMenu->addAction(sendFile);
    btnMenu->popup(ui->sendButton->mapToGlobal(pos));
}
void ClientWindow::slotSendPicture(){
    QString path = QFileDialog::getOpenFileName(0, QObject::tr("Select sending image"),
                   otherPath, QObject::tr("Image files (*.png)"));
    if (path != "") sendToServer(Commands::SendImage, path);
}
void ClientWindow::slotSendFile(){
    QString path = QFileDialog::getOpenFileName(0, QObject::tr("Select sending file"),
                   otherPath, QObject::tr("Select file (*.* all)"));
    if (path != "") sendToServer(Commands::SendFile, path);
}

void ClientWindow::on_actionIlya_triggered()
{
    statusBar()->showMessage("Now you are .. Oh godness.. YOU ARE THE CREATOR!", 2500);
    ui->actionJasmin->setChecked(false);
    ui->actionBoris->setChecked(false);
    ui->actionAnastasia->setChecked(false);
    client.ip = "127.0.0.1";
    client.port = 45678;
    client.username = "Ilya the CREATOR";
    this->setWindowTitle(client.username);
    client.status = Status::Other;
    ui->actionOther->setChecked(true);
    ui->actionOnline->setChecked(false);
    ui->actionNotInPlace->setChecked(false);
    ui->actionDoNotDisturb->setChecked(false);
    client.statusName = "INCREDYBILIS!";
    ui->labelStatus->setText(client.statusName);
    client.path = ":/new/prefix1/other/creator.png";
    setGeometry(QRect(50, 50, 700, 450));
    windowColor.setNamedColor("grey");
    pal.setColor(QPalette::Background, windowColor);
    setAutoFillBackground(true);
    setPalette(pal);
    myMsgColor.setNamedColor("black");
    otherMsgColor.setNamedColor("blue");
    client.colorName = "white";
    showIP = true;
    ui->labelIP->setHidden(false);
    ui->labelPort->setHidden(false);
    ui->label_2->setHidden(false);
    ui->label_3->setHidden(false);
    showTime = true;
    ui->actionAutosave->setChecked(true);
    ui->actionShowLB->setChecked(true);
    ui->loadProgress->setVisible(true);
    update();
}
void ClientWindow::on_actionJasmin_triggered()
{
    statusBar()->showMessage("Now you are .. amazing Jasmin!", 2500);
    ui->actionIlya->setChecked(false);
    ui->actionBoris->setChecked(false);
    ui->actionAnastasia->setChecked(false);
    client.ip = "127.0.0.1";
    client.port = 45678;
    client.username = "Jasmin a.k.a the beauty";
    this->setWindowTitle(client.username);
    client.status = Status::Other;
    ui->actionOther->setChecked(true);
    ui->actionOnline->setChecked(false);
    ui->actionNotInPlace->setChecked(false);
    ui->actionDoNotDisturb->setChecked(false);
    client.statusName = "Always Beautiful";
    ui->labelStatus->setText(client.statusName);
    client.path = ":/new/prefix1/other/client3.png";
    setGeometry(QRect(50, 550, 700, 450));
    windowColor.setNamedColor("magenta");
    pal.setColor(QPalette::Background, windowColor);
    setAutoFillBackground(true);
    setPalette(pal);
    myMsgColor.setNamedColor("magenta");
    otherMsgColor.setNamedColor("darkMagenta");
    client.colorName = "darkMagenta";
    showIP = true;
    ui->labelIP->setHidden(false);
    ui->labelPort->setHidden(false);
    ui->label_2->setHidden(false);
    ui->label_3->setHidden(false);
    showTime = true;
    ui->actionAutosave->setChecked(true);
    ui->actionShowLB->setChecked(true);
    ui->loadProgress->setVisible(true);
    update();
}
void ClientWindow::on_actionBoris_triggered()
{
    statusBar()->showMessage("Now you are smart Boris!", 2500);
    ui->actionIlya->setChecked(false);
    ui->actionJasmin->setChecked(false);
    ui->actionAnastasia->setChecked(false);
    client.ip = "127.0.0.1";
    client.port = 45678;
    client.username = "Boris Britva";
    this->setWindowTitle(client.username);
    client.status = Status::Other;
    ui->actionOther->setChecked(true);
    ui->actionOnline->setChecked(false);
    ui->actionNotInPlace->setChecked(false);
    ui->actionDoNotDisturb->setChecked(false);
    client.statusName = "For HONOR!";
    ui->labelStatus->setText(client.statusName);
    client.path = ":/new/prefix1/other/client2.png";
    setGeometry(QRect(1200, 50, 700, 450));
    windowColor.setNamedColor("cyan");
    pal.setColor(QPalette::Background, windowColor);
    setAutoFillBackground(true);
    setPalette(pal);
    myMsgColor.setNamedColor("cyan");
    otherMsgColor.setNamedColor("darkCyan");
    client.colorName = "darkCyan";
    showIP = true;
    ui->labelIP->setHidden(false);
    ui->labelPort->setHidden(false);
    ui->label_2->setHidden(false);
    ui->label_3->setHidden(false);
    showTime = true;
    ui->actionAutosave->setChecked(true);
    ui->actionShowLB->setChecked(true);
    ui->loadProgress->setVisible(true);
    update();
}
void ClientWindow::on_actionAnastasia_triggered()
{
    statusBar()->showMessage("Now you are cool Anastasia!", 2500);
    ui->actionIlya->setChecked(false);
    ui->actionJasmin->setChecked(false);
    ui->actionBoris->setChecked(false);
    client.ip = "127.0.0.1";
    client.port = 45678;
    client.username = "Anastasia";
    this->setWindowTitle(client.username);
    client.status = Status::Other;
    ui->actionOther->setChecked(true);
    ui->actionOnline->setChecked(false);
    ui->actionNotInPlace->setChecked(false);
    ui->actionDoNotDisturb->setChecked(false);
    client.statusName = "On work...";
    ui->labelStatus->setText(client.statusName);
    client.path = ":/new/prefix1/other/client4.png";
    setGeometry(QRect(1200, 550, 700, 450));
    windowColor.setNamedColor("red");
    pal.setColor(QPalette::Background, windowColor);
    setAutoFillBackground(true);
    setPalette(pal);
    myMsgColor.setNamedColor("red");
    otherMsgColor.setNamedColor("darkRed");
    client.colorName = "darkRed";
    showIP = true;
    ui->labelIP->setHidden(false);
    ui->labelPort->setHidden(false);
    ui->label_2->setHidden(false);
    ui->label_3->setHidden(false);
    showTime = true;
    ui->actionAutosave->setChecked(true);
    ui->actionShowLB->setChecked(true);
    ui->loadProgress->setVisible(true);
    update();
}
