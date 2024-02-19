#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QSerialPort *serialPort;
    QTimer *serialPortUpdateTimer;
    QLabel *dateTimeLabel;
    QTimer *dateTimeUpdateTimer;
    QString currentFilePath;
    void init();

private slots:
    void on_openserial_Button_clicked();

    void serialPortReadyRead_slot();

    void updateSerialPorts();

    void on_closeserial_Button_clicked();

    void on_clear_Button_clicked();

    void on_send_Button_clicked();

    void on_hex_stateChanged(int state);

    void updateDateTime();


    void on_openFileBtn_released();

    void on_sendFileBtn_released();

    void on_themeBtn_released();

private:
    Ui::MainWindow *ui;
    bool hexDisplayEnabled = false;
};
#endif // MAINWINDOW_H
