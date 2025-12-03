#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QThread>
#include <QString>
#include <QDebug>

#define K40

// for VISA(SCPI) IDN? return msg
const QString model = "FSH-8";
const QString version = "0.1";

// todo
/*
real time data query sequence is below

1. FORM:DATA REAL,32 ( 실수 32 비트 데이터 타입 설정 )
2. INIT:CONT ON ( 초기화-다시시작 or 처음시작 )
3. TRAC1:DATA? TRACE1 (반복 명령)

*  measure 명령어 사용안해도 되는지
*/
// 40 added keyword
#ifdef K40
QStringList senseCmds     = {"SENse"}; // hiearachy root command
QStringList startStopSubCmds = {"STArt","STOP"}; // for frequency at display
QStringList centerSubCmds = {"CENTer"}; // for frequency at display
QStringList formCmds = {"FORM"}; // formatting data type
#endif
// set command without general commands
QStringList generalSetCmds   = { "REMOTE", "LOCAL", "PRESET","INIT" };
QStringList freqSpanSetCmds  = {"FREQ","FREQOFFS","SPAN","AUTOSPAN","CHANNEL","CHTABLE"};
QStringList amplitudeSetCmds = {"REFLVL","REFLVLOFFS","RANGE","DYNRANGE","UNIT","RFINPUT","PREAMP" };
QStringList bandwidthSetCmds = {"AUTORBW","RBW","AUTOVBW","VBW","AUTOCISPRBW","CISPRBW","BANDwidth"};
QStringList sweepSetCmds     = {"AUTOSWPTIME","SWPTIME","SWPCONT","TRIGSRC","TRIGLVL","TRIGDEL" };
QStringList traceSetCmds     = {"TRACEMODE","TRACEDET","TRACEMODE","TRACE","TRACEBIN","CTRACE","CTRACEBIN","MTRACE","MTRACEBIN","TRAC1"};
QStringList markerSetCmds    = {"MARK1ON","MARK1","DELTA1ON","DELTA1","MARKPK","MARKNXTPK","MARKMIN","MARKTOCENT","MARKTOLVL","MARKMODE"};

// Measure command
QStringList measureCmds = {"MEAS","TRD1","TRD2","LIMLOW","LIMUPP","LIMPASS","LIMCHKREMOTE","THRLOW","THRUPP","THRPASS","THROFF"};
QStringList trackingCmds = {};
QStringList pwrSensorCmds = {};
QStringList chPowerCmds = {};
QStringList obCmds = {};
QStringList tdmaPowerCmds = {};
QStringList distToFaultMeasureCmds = {};
QStringList receiverCmds = {};

// Setting SCPI commands table
// https://selected-mule-probable.ngrok-free.app/mil_prj/adj_ss/-/issues/8

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
    //return true;
    return false;
}

bool containsAny(const QByteArray &data, const QStringList &list)
{
    for (const QString &s : list) {
        if (data.contains(s.toUtf8()))
            return true;
    }
    return false;
}

void MainWindow::sendRetValue()
{
    QThread::msleep(10);
    socket->write("0");
    socket->flush();
    socket->waitForBytesWritten(50);

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


        } else if ( false == isDeviceBIRD() )
        {
              // ==== 설정 명령어 ==== //
            // FSH-8 : General SCPI commands case
            /*
            REMOTE
            ----------
            cmd
            0
            remote
            0
            */
#if 0
            if ( data.contains("REMOTE"))
            {
                sendRetValue();
            }else if ( data.contains("LOCAL"))
            {
                sendRetValue();
            }else if ( data.contains("PRESET"))
            {
                sendRetValue();
            }else if ( data.contains("INIT"))
            {
                sendRetValue();
            }
#else
            if ( containsAny(data,generalSetCmds))
            {
                sendRetValue();
            }

            if ( containsAny(data,freqSpanSetCmds))
            {
                sendRetValue();
            }

            if ( containsAny(data,amplitudeSetCmds))
            {
                sendRetValue();
            }

            if ( containsAny(data,bandwidthSetCmds))
            {
                sendRetValue();
            }

            if ( containsAny(data,sweepSetCmds))
            {
                sendRetValue();
            }

            if ( containsAny(data,traceSetCmds))
            {
                sendRetValue();
            }

            if ( containsAny(data,markerSetCmds))
            {
                sendRetValue();
            }

#endif
            // FSH-8 : Frequency and Span Settings
            // FSH-8 : Amplitude Settings
            // FSH-8 : Bandwidth Settings
            // FSH-8 : Sweep Settings
            // FSH-8 : Trace Settings
            // FSH-8 : Marker Settings

               // ==== 측정 명령어 ==== //
            // FSH-8 : Measurement
            if ( containsAny(data,measureCmds))
            {
                sendRetValue();
            }

            // FSH-8 : Tracking Generator
            if ( containsAny(data,trackingCmds))
            {
                sendRetValue();
            }

            // FSH-8 : Power Sensor
            if ( containsAny(data,pwrSensorCmds))
            {
                sendRetValue();
            }
            // FSH-8 : Channel Power
            if ( containsAny(data,chPowerCmds))
            {
                sendRetValue();
            }
            // FSH-8 : Occupied Bandwidth
            if ( containsAny(data,obCmds))
            {
                sendRetValue();
            }
            // FSH-8 : TDMA Power
            if ( containsAny(data,tdmaPowerCmds))
            {
                sendRetValue();
            }
            // FSH-8 : Distance To Fault Measurement
            if ( containsAny(data,distToFaultMeasureCmds))
            {
                sendRetValue();
            }

             // === 리시버 모드 명령어 === //
            // FSH-8 : Receiver Mode
            if ( containsAny(data,receiverCmds))
            {
                sendRetValue();
            }
// K40 base added command option
/*
            QStringList senseCmds     = {"SENse"}; // hiearachy root command
            QStringList startStopSubCmds = {"STArt","STOP"}; // for frequency at display
            QStringList centerSubCmds = {"CENTer"}; // for frequency at display
            QStringList formCmds = {"FORM"}; // formatting data type
*/
#ifdef K40
            if ( containsAny(data,senseCmds))
            {
                sendRetValue();
            }

            if ( containsAny(data,startStopSubCmds))
            {
                sendRetValue();
            }
            if ( containsAny(data,centerSubCmds))
            {
                sendRetValue();
            }
            if ( containsAny(data,formCmds))
            {
                sendRetValue();
            }
#endif
        }
        else {
            qDebug("Error : isDeviceBIRD is abnormal status");

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
    qDebug("cmd = %s",cmd.c_str());
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
