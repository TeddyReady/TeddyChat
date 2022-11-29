#include "mythread.h"

MyThread::MyThread(qintptr ID, QObject *parent) :
    QThread(parent)
{
    this->socketDescriptor = ID;
}

void MyThread::run()
{
    socket = new QSslSocket;
    if(!socket->setSocketDescriptor(socketDescriptor))
    {
        emit error(socket->error());
        return;
    }
    //KEY & CERT GENERATION
    EVP_PKEY *privKey = genKey();
    if (privKey == NULL){
       qDebug() << "NULL PRIVATE KEY";
       exit(-1);
    }

    X509 *certif = genX509(privKey);
    if (certif == NULL){
       qDebug() << "NULL CERT";
       exit(-1);
    }

    BIO *bio = BIO_new(BIO_s_mem());
    if (!PEM_write_bio_X509(bio, certif)){
       qDebug() << "NULL RETURNED BY PEM_write_bio_x509";
       exit(-1);
    }

    BUF_MEM *biostruct;
    BIO_get_mem_ptr(bio, &biostruct);
    std::unique_ptr<char[]> buf = std::make_unique<char[]>(biostruct->length);
    if (static_cast<size_t>(BIO_read(bio, buf.get(), biostruct->length)) != biostruct->length){
       qDebug() << "BIO_read unable to get information from Buffer";
       exit(-1);
    }
    QSslCertificate cert(QByteArray(buf.get(), biostruct->length));
    QSslKey key(reinterpret_cast<Qt::HANDLE>(privKey));

    QSslConfiguration config;
    config.setLocalCertificate(cert);
    config.setPrivateKey(key);

    socket->setSslConfiguration(config);
    socket->startServerEncryption();

    connect(socket, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
    exec();
}

void MyThread::sendToClient(int command, QString receiver, QString message, int option){
    QByteArray data; data.clear();
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_12);
    out << (quint64)0 << (quint8)command;
    if(command == Commands::SendMessage) {
        out << receiver << message
            << QDate::currentDate().toString()
            << QTime::currentTime().toString();
        out.device()->seek(0);
        out << quint64(data.size() - sizeof(quint64));
        //Custom Sending
        for (int k = 0; k < sendList.size(); k++) {
            for(int i = 0; i < clients.size(); i++){
                if (sendList[k] == clients[i]->username) {
                    clients[i]->socket->write(data);
                    break;
                }
            }
        } sendList.clear();
    } else if(command == Commands::Authentication || command == Commands::Exit) {
        out << receiver << message;
        out.device()->seek(0);
        out << quint64(data.size() - sizeof(quint64));
        for(int i = 0; i < clients.size(); i++){
            clients[i]->socket->write(data);
        }
    } else if (command == Commands::UpdateDataBase) {
        out << (quint8)(clients.size() - 1);
        for (int i = 0; i < clients.size() - 1; i++) {
            out << clients[i]->username
                << QString::number(clients[i]->status)
                << clients[i]->date
                << clients[i]->time
                << clients[i]->path
                << clients[i]->statusName
                << clients[i]->colorName;
        }
        out.device()->seek(0);
        out << quint64(data.size() - sizeof(quint64));
        clients[clients.size() - 1]->socket->write(data);
    } else if (command == Commands::NewClient) {
        out << clients[clients.size() - 1]->username
            << QString::number(clients[clients.size() - 1]->status)
            << clients[clients.size() - 1]->date
            << clients[clients.size() - 1]->time
            << clients[clients.size() - 1]->path
            << clients[clients.size() - 1]->statusName
            << clients[clients.size() - 1]->colorName;

        out.device()->seek(0);
        out << quint64(data.size() - sizeof(quint64));
        clients[receiver.toInt()]->socket->write(data);
    } else if (command == Commands::Restart) {
        out.device()->seek(0);
        out << quint64(data.size() - sizeof(quint64));
        for(int i = 0; i < clients.size(); i++){
            clients[i]->socket->write(data);
            emit threadClientDisconnected(clients[i]);
        } clients.clear();

    } else if (command == Commands::DataChanged) {
        out << receiver;
        if (option == Status::Other || option == Commands::PathChanged || option == Commands::ColorChanged)
            out << QString::number(option);
        out << message;
        out.device()->seek(0);
        out << quint64(data.size() - sizeof(quint64));
        for(int i = 0; i < clients.size(); i++){
            clients[i]->socket->write(data);
        }
    } else if (command == Commands::SendImage) {
        out << receiver << message
            << QDate::currentDate().toString()
            << QTime::currentTime().toString();
        out.device()->seek(0);
        out << quint64(data.size() - sizeof(quint64));
        //Custom Sending
        for (int k = 0; k < sendList.size(); k++) {
            for(int i = 0; i < clients.size(); i++){
                if (sendList[k] == clients[i]->username) {
                    clients[i]->socket->write(data);
                    break;
                }
            }
        } sendList.clear();
    } else if (command == Commands::ForceQuit) {
        out.device()->seek(0);
        out << quint64(data.size() - sizeof(quint64));
        emit threadRemoveClient(clients[clients.size() - 1]);
        clients[clients.size() - 1]->socket->write(data);
    } else if (command == Commands::ForbiddenName) {
        out.device()->seek(0);
        out << quint64(data.size() - sizeof(quint64));
        for (int i = 0; i < clients.size(); i++) {
            if (clients[i]->username == receiver) {
                clients[i]->socket->write(data);
                break;
            }
        }
    } else if (command == Commands::Ready) {
        out.device()->seek(0);
        out << quint64(data.size() - sizeof(quint64));
        for (int i = 0; i < clients.size(); i++) {
            if (clients[i]->username == receiver) {
                clients[i]->socket->write(data);
                break;
            }
        }
    } else if (command == Commands::FileAccepted) {
        //Custom Sending
        for (int k = 0; k < sendList.size(); k++) {
            for (int i = 0; i < clients.size(); i++) {
                qDebug() << sendList[k] << " " << clients[i]->username;
                if (sendList[k] == clients[i]->username) {
                    emit threadUpdateProgressBar(0);
                    QFile sendingFile(message);
                    QString fileName(message.split("/").last());
                    QDataStream fileRead(&sendingFile);
                    fileRead.setVersion(QDataStream::Qt_5_12);
                    if (sendingFile.open(QIODevice::ReadOnly)) {
                        out << receiver << QTime::currentTime().toString()
                            << static_cast<quint64>(sendingFile.size()) << fileName;
                        out.device()->seek(0);
                        out << (quint64)(data.size() - sizeof(quint64));
                        clients[i]->socket->write(data);
                        socket->waitForReadyRead(100);
                        //Отправляем сам файл
                        quint64 curSendedSize = 0;
                        char bytes[8]; bytes[7] = '\0';
                        while (curSendedSize < static_cast<quint64>(sendingFile.size())) {
                            int lenght = fileRead.readRawData(bytes, sizeof(char) * 7);
                            data.setRawData(bytes, sizeof(char) * lenght);
                            curSendedSize += clients[i]->socket->write(data, sizeof(char) * lenght);
                            socket->waitForReadyRead(100);
                            qDebug() << curSendedSize << " " << static_cast<quint64>(sendingFile.size()) << " |" << lenght;
                            //Обновляем прогресс отправки
                            emit threadUpdateProgressBar(static_cast<int>(static_cast<qreal>(curSendedSize) / static_cast<qreal>(sendingFile.size()) * 100));
                        } sendingFile.close();
                    }
                } break;
            }
        } sendList.clear();
    }
}

void MyThread::slotReadyRead(){
    socket = (QSslSocket*)sender();
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_12);
    //PACKET_DEFENCER
    if (dataSize == 0) {
        if (static_cast<size_t>(socket->bytesAvailable()) < sizeof(quint64)) return;
        in >> dataSize;
    }
    if (static_cast<quint64>(socket->bytesAvailable()) < dataSize) return;
    else dataSize = 0;
    //END
    quint8 command;
    in >> command;
    if(static_cast<int>(command) == Commands::SendMessage) {
        QString name, message, otherName; int cnt;
        in >> name >> message >> cnt;
        for (int i = 0; i < cnt; i++) {
            in >> otherName;
            sendList.push_back(otherName);
        } sendList.push_back(name);
        sendToClient(Commands::SendMessage, name, message);
    } else if(static_cast<int>(command) == Commands::Authentication){
        QString ip, port, name, status, date, time, path, customStatus, colorName;
        in >> ip >> port >> name >> status >> date >> time >> path >> customStatus >> colorName;
        MyClient *ptr = new MyClient(ip, port.toInt(), name, status.toInt(), date, time, socket, path, customStatus, colorName);
        emit threadAddClient(ptr);
        //Validation
        for (int i = 0; i < clients.size() - 1; i++) {
            if (clients[i]->username == clients[clients.size() - 1]->username) {
                qDebug() << clients[i]->username << " " << clients[clients.size() - 1]->username;
                sendToClient(Commands::ForceQuit);
                emit threadFailedValidation();
                return;
            }
        }
        //END
        emit threadNewConnection(ptr);
        sendToClient(Commands::Authentication, ptr->username, "Server: " + ptr->username + " join chat channel!");
    } else if (static_cast<int>(command) == Commands::Exit){
        QString name;
        in >> name;
        for(int i = 0; i < clients.size(); i++){
            if(name == clients[i]->username){
                emit threadClientDisconnected(clients[i]);
                emit threadRemoveClient(clients[i]);
                sendToClient(Commands::Exit, name, "Server: " + name + " leaved current session!");
                break;
            }
        }
    } else if (static_cast<int>(command) == Commands::UpdateDataBase) {
        sendToClient(Commands::UpdateDataBase);
        for (int i = 0; i < clients.size() - 1; i++)
            sendToClient(Commands::NewClient, QString::number(i));
    } else if (static_cast<int>(command) == Commands::DataChanged) {
        QString name, newData; in >> name >> newData;
        if (newData == QString::number(Status::Online) ||
            newData == QString::number(Status::NotInPlace) ||
            newData == QString::number(Status::NotDisturb)) {
            for (MyClient *ptr: clients){
                if (ptr->username == name) {
                    ptr->status = newData.toInt();
                    break;
                }
            } sendToClient(Commands::DataChanged, name, newData);
        } else if (newData == QString::number(Status::Other)) {
            QString customStatus; in >> customStatus;
            for (MyClient *ptr: clients){
                if (ptr->username == name) {
                    ptr->status = newData.toInt();
                    break;
                }
            } sendToClient(Commands::DataChanged, name, customStatus, Status::Other);
        } else if (newData == QString::number(Commands::PathChanged)) {
            QString path; in >> path;
            for (MyClient *ptr: clients){
                if (ptr->username == name) {
                    ptr->path = path;
                    break;
                }
            } sendToClient(Commands::DataChanged, name, path, Commands::PathChanged);
        } else if (newData == QString::number(Commands::ColorChanged)) {
            QString colorName; in >> colorName;
            for (MyClient *ptr: clients){
                if (ptr->username == name) {
                    ptr->colorName = colorName;
                    break;
                }
            } sendToClient(Commands::DataChanged, name, colorName, Commands::ColorChanged);
        } else {
            //Validation
            for (int i = 0; i < clients.size() - 1; i++) {
                if (clients[i]->username == newData) {
                    sendToClient(Commands::ForbiddenName, name, newData);
                    return;
                }
            }
            //END
            for (MyClient *ptr: clients){
                if (ptr->username == name) {
                    ptr->username = newData;
                    emit threadReNameOnUI(name, newData);
                    break;
                }
            } sendToClient(Commands::DataChanged, name, newData);
        }
    } else if (static_cast<int>(command) == Commands::SendImage) {
        QString name, image, otherName; int cnt;
        in >> name >> image >> cnt;
        for (int i = 0; i < cnt; i++) {
            in >> otherName;
            sendList.push_back(otherName);
        } sendList.push_back(name);
        sendToClient(Commands::SendImage, name, image);
    } else if (static_cast<int>(command) == Commands::SendFile) {
        quint64 fileSize, curFileSize = 0; QString name, fileName, otherName; int cnt;
        in >> name >> fileSize >> fileName >> cnt;
        for (int i = 0; i < cnt; i++) {
            in >> otherName;
            sendList.push_back(otherName);
        }
        sendToClient(Commands::Ready);
        QFile receivedFile(downloadPath + fileName);
        if (receivedFile.open(QFile::WriteOnly)) {
            QDataStream fileWrite(&receivedFile);
            fileWrite.setVersion(QDataStream::Qt_5_12);
            while (curFileSize < fileSize) {
                sendToClient(Commands::Ready);
                QByteArray packet = socket->readAll();
                curFileSize += fileWrite.writeRawData(packet.data(), packet.size());
                socket->waitForReadyRead(100);
                qDebug() << curFileSize << " vs " << fileSize << " |" << packet.size();
                emit threadUpdateProgressBar(static_cast<int>(static_cast<qreal>(curFileSize) / static_cast<qreal>(fileSize) * 100));
            } receivedFile.close(); sendToClient(Commands::FileAccepted, name, receivedFile.fileName());
        }
    }
}

void MyThread::slotThreadUpdateClients(QVector<MyClient *> tmp){
    clients = tmp;
}
void MyThread::slotCmdRestart(){
    sendToClient(Restart);
}
void MyThread::slotCmdExit(QString name, QString msg){
    sendToClient(Exit, name, msg);
}
