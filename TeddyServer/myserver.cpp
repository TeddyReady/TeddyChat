#include "myserver.h"

MyServer::MyServer() {}
void MyServer::deployServer(){
    if (this->listen(static_cast<QHostAddress>(ip), port)) {
        emit serverStarted(true);
    } else {
        emit serverStarted(false);
    }
}

void MyServer::incomingConnection(qintptr socketDescriptor){
    socket = new QSslSocket(this);
    if(socket->setSocketDescriptor(socketDescriptor)) {
    //KEY & CERT GENERATION
    EVP_PKEY * privKey = genKey();
       if (privKey == NULL){
          qDebug() << "NULL PRIVATE KEY";
          exit(-1);
       }

       X509 * certif = genX509(privKey);
       if (certif == NULL){
          qDebug() << "NULL CERT";
          exit(-1);
       }

       BIO* bio = BIO_new(BIO_s_mem());
       if (!PEM_write_bio_X509(bio, certif)){
          qDebug() << "NULL RETURNED BY PEM_write_bio_x509";
           exit(-1);
       }

       BUF_MEM* biostruct;
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
        connect(socket, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
        socket->startServerEncryption();
    } else {
        delete socket;
    }
}

void MyServer::sendToClient(int command, QString receiver, QString message, int option){
    QByteArray data; data.clear();
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_12);
    out << (quint16)0 << (quint8)command;
    if(command == Commands::SendMessage) {
        out << receiver << message
            << QDate::currentDate().toString()
            << QTime::currentTime().toString();
        out.device()->seek(0);
        out << quint16(data.size() - sizeof(quint16));
        for(int i = 0; i < clients.size(); i++){
            clients[i]->socket->write(data);
        }
    } else if(command == Commands::Authentication || command == Commands::Exit) {
        out << receiver << message;
        out.device()->seek(0);
        out << quint16(data.size() - sizeof(quint16));
        for(int i = 0; i < clients.size(); i++){
            clients[i]->socket->write(data);
        }
    } else if (command == Commands::UpdateDataBase) {
        out << (quint8)(clients.size() - 1);
        for (int i = 0; i < clients.size() - 1; i++){
            out << clients[i]->username
                << QString::number(clients[i]->status)
                << clients[i]->date
                << clients[i]->time
                << clients[i]->path
                << clients[i]->statusName;
        }
        out.device()->seek(0);
        out << quint16(data.size() - sizeof(quint16));
        clients[clients.size() - 1]->socket->write(data);
    } else if (command == Commands::NewClient) {
        out << clients[clients.size() - 1]->username
            << QString::number(clients[clients.size() - 1]->status)
            << clients[clients.size() - 1]->date
            << clients[clients.size() - 1]->time
            << clients[clients.size() - 1]->path
            << clients[clients.size() - 1]->statusName;

        out.device()->seek(0);
        out << quint16(data.size() - sizeof(quint16));
        clients[receiver.toInt()]->socket->write(data);
    } else if (command == Commands::Restart) {
        out.device()->seek(0);
        out << quint16(data.size() - sizeof(quint16));
        for(int i = 0; i < clients.size(); i++){
            clients[i]->socket->write(data);
            emit clientDisconnected(clients[i]);
        } clients.clear();

    } else if (command == Commands::DataChanged) {
        out << receiver;
        if (option == Status::Other)
            out << QString::number(option);
        out << message;
        out.device()->seek(0);
        out << quint16(data.size() - sizeof(quint16));
        for(int i = 0; i < clients.size(); i++){
            clients[i]->socket->write(data);
        }
    } else if (command == Commands::Image) {
        out << receiver << message;
        out.device()->seek(0);
        out << quint16(data.size() - sizeof(quint16));
        for(int i = 0; i < clients.size(); i++){
            clients[i]->socket->write(data);
        }
    } else if (command == Commands::ForceQuit) {
        out.device()->seek(0);
        out << quint16(data.size() - sizeof(quint16));
        clients[clients.size() - 1]->socket->write(data);
        clients.pop_back();
    }
}
void MyServer::slotReadyRead(){
    socket = (QSslSocket*)sender();
    QDataStream in(socket);
    //PACKET_DEFENCER
    if (dataSize == 0) {
        if (socket->bytesAvailable() < (int)sizeof(quint16)) return;
        in >> dataSize;
    }
    if (socket->bytesAvailable() < dataSize) return;
    else dataSize = 0;
    //END
    quint8 command;
    in >> command;
    if(static_cast<int>(command) == Commands::SendMessage) {
        QString name, message;
        in >> name >> message;
        sendToClient(Commands::SendMessage, name, message);
    } else if(static_cast<int>(command) == Commands::Authentication){
        QString ip, port, name, status, date, time, path, customStatus;
        in >> ip >> port >> name >> status >> date >> time >> path >> customStatus;
        MyClient *ptr = new MyClient(ip, port.toInt(), name, status.toInt(), date, time, socket, path, customStatus);
        clients.push_back(ptr);
        //Validation
        for (int i = 0; i < clients.size() - 1; i++) {
            if (clients[i]->username == clients[clients.size() - 1]->username) {
                sendToClient(Commands::ForceQuit);
                emit failedValidation();
                return;
            }
        }
        //END
        emit newConnection(ptr);
        sendToClient(Commands::Authentication, ptr->username, "Server: " + ptr->username + " join chat channel!");
    } else if (static_cast<int>(command) == Commands::Exit){
        QString name;
        in >> name;
        for(int i = 0; i < clients.size(); i++){
            if(name == clients[i]->username){
                emit clientDisconnected(clients[i]);
                clients.removeAt(i);
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
        } else {
            for (MyClient *ptr: clients){
                if (ptr->username == name) {
                    ptr->username = newData;
                    emit reNameOnUI(name, newData);
                    break;
                }
            } sendToClient(Commands::DataChanged, name, newData);
        }
    } else if (static_cast<int>(command) == Commands::Image) {
        QString name, image; in >> name >> image;
        sendToClient(Commands::Image, name, image);
    }
}
