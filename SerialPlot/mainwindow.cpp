#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include <QFileDialog>
#include <QFile>
#include <QStandardPaths>
#include <QColorDialog>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init();

    // 初始化日期时间标签和定时器
    dateTimeLabel = new QLabel(this);
    dateTimeUpdateTimer = new QTimer(this);

    // 将标签添加到状态栏
    statusBar()->addPermanentWidget(dateTimeLabel); // 使用addPermanentWidget使得显示在右侧

    // 设置定时器触发的槽函数来更新时间
    connect(dateTimeUpdateTimer, &QTimer::timeout, this, &MainWindow::updateDateTime);
    dateTimeUpdateTimer->start(1000); // 每秒更新时间

    updateDateTime(); // 初始化显示

    QStringList serialNamePort;

    serialPortUpdateTimer = new QTimer(this);
    serialPortUpdateTimer->start(1000);
    connect(serialPortUpdateTimer,SIGNAL(timeout()),this,SLOT(updateSerialPorts()));

    connect(ui->hex,SIGNAL(stateChanged(int)),this,SLOT(on_hex_stateChanged(int)));

    serialPort = new QSerialPort(this);
    connect(serialPort,SIGNAL(readyRead()),this,SLOT(serialPortReadyRead_slot()));

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        serialNamePort<<info.portName();
        // serialNamePort<<info.portName()+ "：" +info.description();
        qDebug() << "Available port: " << info.portName();
    }
    ui->port_comboBox->addItems(serialNamePort);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init()
{

}

void MainWindow::serialPortReadyRead_slot()
{
    QByteArray data =serialPort->readAll();
    QString displayString;

    if(!data.isEmpty())
    {
        if(hexDisplayEnabled){
            displayString = data.toHex(' ').toUpper();
        }else{
            //displayString = QString::fromLocal8Bit(data);  //Local8bits会出现中文乱码
            displayString = QString::fromUtf8(data);
        }
    }
    ui->recieve_plainTextEdit->appendPlainText(displayString + "\r\n");
}


/*
 * open serila_button
 * */
void MainWindow::on_openserial_Button_clicked()
{
    QSerialPort::BaudRate baudRate;
    QSerialPort::DataBits dataBits;
    QSerialPort::StopBits stopBits;
    QSerialPort::Parity checkBits;

    /* baudRate */
    if(ui->baud_comboBox->currentText() == "1200"){
        baudRate = QSerialPort::Baud1200;
    }else if(ui->baud_comboBox->currentText() == "2400"){
        baudRate = QSerialPort::Baud2400;
    }else if(ui->baud_comboBox->currentText() == "4800"){
        baudRate = QSerialPort::Baud4800;
    }else if(ui->baud_comboBox->currentText() == "9600"){
        baudRate = QSerialPort::Baud9600;
    }else if(ui->baud_comboBox->currentText() == "19200"){
        baudRate = QSerialPort::Baud19200;
    }else if(ui->baud_comboBox->currentText() == "38400"){
        baudRate = QSerialPort::Baud38400;
    }else if(ui->baud_comboBox->currentText() == "57600"){
        baudRate = QSerialPort::Baud57600;
    }else if(ui->baud_comboBox->currentText() == "115200"){
        baudRate = QSerialPort::Baud115200;
    }else{
        baudRate = QSerialPort::Baud115200;
    }

    /* databits */
    if(ui->data_comboBox->currentText() == "5"){
        dataBits = QSerialPort::Data5;
    }else if(ui->data_comboBox->currentText() == "6"){
        dataBits = QSerialPort::Data6;
    }else if(ui->data_comboBox->currentText() == "7"){
        dataBits = QSerialPort::Data7;
    }else if(ui->data_comboBox->currentText() == "8"){
        dataBits = QSerialPort::Data8;
    }else {
        dataBits = QSerialPort::Data8;
    }

    /* stopbits */
    if(ui->stop_comboBox->currentText() == "1"){
        stopBits = QSerialPort::OneStop;
    }else if(ui->stop_comboBox->currentText() == "1.5"){
        stopBits = QSerialPort::OneAndHalfStop;
    }else if(ui->stop_comboBox->currentText() == "2"){
        stopBits = QSerialPort::TwoStop;
    }else{
        stopBits = QSerialPort::OneStop;
    }

    /* checkbits */
    if(ui->check_comboBox->currentText() == "none"){
        checkBits = QSerialPort::NoParity;
    }else{
        checkBits = QSerialPort::NoParity;
    }

    serialPort->setPortName(ui->port_comboBox->currentText());
    serialPort->setBaudRate(baudRate);
    serialPort->setDataBits(dataBits);
    serialPort->setStopBits(stopBits);
    serialPort->setParity(checkBits);

    /* check whether the serial port is successfully opened */
    if(serialPort->open(QIODevice::ReadWrite) == true){
        QMessageBox::information(this,"SerialPort","打开串口成功");
    }
    else{
        QMessageBox::warning(this,"SerialPort","打开串口失败:"+ serialPort->errorString());
    }

}


void MainWindow::on_closeserial_Button_clicked()
{
    if(serialPort->isOpen()){
        serialPort->close();
        ui->fileLineEdit->clear();
        QMessageBox::information(this,"SerialPort","串口已关闭");
    }else{
        QMessageBox::information(this,"SerialPort","串口尚未打开，无需关闭");
    }

}


void MainWindow::on_clear_Button_clicked()
{
    ui->recieve_plainTextEdit->clear();
}


void MainWindow::on_send_Button_clicked()
{
    if(serialPort->isOpen()){
        QString textToSend = ui->send_lineEdit->text();
        // 检查是否选中发送新行
        if(ui->sendnewline->isChecked()) {
            textToSend.append("\n"); // 在文本末尾追加换行符
        }
        //serialPort->write(textToSend.toLocal8Bit().data());  //Local8bits会出现中文乱码
        serialPort->write(textToSend.toUtf8().data());
    }else{
        QMessageBox::information(this,"SerialPort","串口尚未打开，无法发送");
    }

}

// 更新串口状态
void MainWindow::updateSerialPorts()
{
    // 保存当前选中的串口名称
    QString currentSelection = ui->port_comboBox->currentText();

    // 清空并重新填充串口列表
    ui->port_comboBox->clear();
    QStringList serialNamePort;
    QString portName;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        portName = info.portName();
        // portName = info.portName()+ "：" +info.description();
        serialNamePort << portName;
        qDebug() << "Available port: " << portName;
    }
    ui->port_comboBox->addItems(serialNamePort);

    // 如果之前选中的串口仍然存在于列表中，重新选择它
    int index = ui->port_comboBox->findText(currentSelection);
    if (index != -1) {
        ui->port_comboBox->setCurrentIndex(index);
    }
}

// 16进制显示转换
void MainWindow::on_hex_stateChanged(int state)
{
    hexDisplayEnabled = (state == Qt::Checked);
}

// 更新日期时间显示的槽函数
void MainWindow::updateDateTime()
{
    QDateTime currentTime = QDateTime::currentDateTime();
    QString dateTimeText = currentTime.toString("yyyy年MM月dd日 HH:mm:ss");
    dateTimeLabel->setText(dateTimeText);
}

void MainWindow::on_openFileBtn_released()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), tr("All Files (*)"));
    if (!fileName.isEmpty()) {
        ui->fileLineEdit->setText(fileName);
    }
}

void MainWindow::on_sendFileBtn_released()
{
    if(!ui->fileLineEdit->text().isEmpty() && serialPort->isOpen())
    {
        QFile file(ui->fileLineEdit->text());
        if(file.open(QIODevice::ReadOnly)){
            QByteArray fileData =file.readAll();

            serialPort->write(fileData);
            file.close();
            QMessageBox::information(this,"SerialPort","文件发送成功");
        }
        else{
            QMessageBox::information(this,"SerialPort","无法打开文件:"+file.errorString());
        }
    }
    else{
        QMessageBox::information(this,"SerialPort","文件路径为空或串口未打开！");
    }
}


void MainWindow::on_themeBtn_released()
{
    QColor color = QColorDialog::getColor(Qt::white, this, "选择颜色");
    if(color.isValid()) {
        QString qss = QString("background-color: %1").arg(color.name());
        this->setStyleSheet(qss);
    }
}

