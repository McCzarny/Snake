#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <windows.h>
#include <QMessageBox>
#include <QFileDialog>
#include <fstream>
#include <iostream>
#include <QDebug>
#include <cmath>
#include <QTextCodec>


#define SOH 1
#define NAK 21
#define ACK 6
#define EOT 4
#define CAN 24
#define C 67


namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    void setPredkoscTransmisji(DWORD value);
    void setBityParzystosci(DWORD value);
    void setBityStopu(DWORD value);
    void nadawanie();
    void odbieranie();
    unsigned char pobierzZnakzPortu();
    void wyslijZnakdoPortu(unsigned char znak);
    void inicjalizacja_CRC();
    ~MainWindow();

private slots:
    void on_comboBox_currentIndexChanged(int index);

    void on_comboBox_2_currentIndexChanged(int index);

    void on_comboBox_3_currentIndexChanged(int index);

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void wybranoPlik(QString text);

    void on_pushButton_3_clicked();

    void on_comboBox_5_currentIndexChanged(int index);

private:
    Ui::MainWindow *ui;
    HANDLE uchwyt_do_portu;
    DCB parametry_transmisji;
    COMMTIMEOUTS timeouts;
    QMessageBox *popup;
    QFileDialog *okno_przegladania;
    ushort Tablica_crc16[256];

};

#endif // MAINWINDOW_H
