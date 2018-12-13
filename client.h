#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QTextStream>
#include <QFile>
#include <QDataStream>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#define BUFFER_SIZE 225280

class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = 0);

    void Connect();
    void selectionMenu();
    void downloadData();
    void listFiles();

signals:

public slots:

private:
    QTcpSocket *socket;
};

#endif // CLIENT_H
