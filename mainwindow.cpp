#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QThread>
#include <QString>
#include <QDebug>
// for VISA(SCPI) IDN? return msg
const QString model = "FSH-8";
const QString version = "0.1";


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);


}

MainWindow::~MainWindow()
{
    delete ui;
}
bool isDeviceBIRD()
{
    return true;
}
void MainWindow::openSocket()
{
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

            if ( isDeviceBIRD() )
            {
                socket->write("BIRD,7022 (Simulator),,v1\n");   // or write something back based on the received msg
            }else
            {
                socket->write("R&S,FSH-8 (Simulator),,v1\n");
            }
            socket->flush();
            socket->waitForBytesWritten(50);
        }

        if ( isDeviceBIRD() )
        {
            if ( data.contains("CLS") )
            {

                QThread::msleep(10);
                socket->write("OK for CLS(clear screen) ");
                socket->flush();
                socket->waitForBytesWritten(50);
            }

            if ( data.contains("RST"))
            {

                QThread::msleep(10);
                socket->write("OK for RST(reset) ");
                socket->flush();
                socket->waitForBytesWritten(50);

            }

            if ( data.contains(":TRAC:APOW?"))
            {
                QString result;
                result = result.append("reading:Measurement.ForwardAveragePower");
                result = result.append(",value:25.5"); // Forward Average Power

              //  result = result.append("reading:Measurement.reflectedAveragePower");
              //  result = result.append(",value:25.5");
                QString timeStamp;
                result = result.append("reading:Measurement.Match");
                result = result.append(",value:0"); // Forward Average Power
                result = result.append(":timestamp:23123123123");


                socket->write(result.toStdString().c_str());
                socket->flush();
                socket->waitForBytesWritten(50);
                qDebug(" ");
            }


        }
        else {

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

void MainWindow::on_pushButton_clicked()
{
    openSocket();
}
