#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QThread>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    server = new QTcpServer(this);
    connect(server, SIGNAL(newConnection()), this, SLOT(new_tcp_connection()));
    if(!server->listen(QHostAddress::Any, default_port))
    {
        qDebug() << "Server could not start";
    }
    else
    {
        qDebug() << "Server started!";
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeSocket()
{
    for (QTcpSocket* socket : findChildren<QTcpSocket*>()) {
        disconnect(socket, nullptr, this, nullptr);
        if (socket->state() == QAbstractSocket::ConnectedState)
            socket->disconnectFromHost();

        socket->close();
        socket->deleteLater();
    }

    // 2. 서버 닫기
    if (server->isListening())
        server->close();

    qDebug() << "close socket";
}
void MainWindow::new_tcp_connection()
{
    // need to grab the socket
    socket = server->nextPendingConnection();
    socket->setReadBufferSize(512);
    connect(socket, SIGNAL(readyRead()), SLOT(read_data_from_socket()));
}

void MainWindow::read_data_from_socket()
{
    QByteArray data;
    string str_data;

    if (socket->bytesAvailable())
    {
        data = socket->readAll();
        str_data = data.constData();
        qDebug() << "Incoming socket data: " << data;

        if ( data.contains("IDN?"))
        {

            QThread::msleep(100);
            socket->write("Envox,EEZ H24005 (Simulator),0000000,v1.1\n");   // or write something back based on the received msg
            socket->flush();
            socket->waitForBytesWritten(50);
        }
       // socket->close();    // closing the socket manually to avoid memory leaks

        handle_tcp_command(str_data);   // pass the msg
    }
    /*
    else
    {
        socket->write("could not receive data");
        socket->flush();
        socket->waitForBytesWritten(50);
        //socket->close();
    }
    */
}

void MainWindow::handle_tcp_command(string cmd)
{
    // you can do anything you want in this function based on the given msg
}

void MainWindow::on_pushButton_2_clicked()
{
    closeSocket();
}
