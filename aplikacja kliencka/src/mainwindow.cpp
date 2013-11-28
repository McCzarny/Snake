#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    popup = new QMessageBox;
    okno_przegladania = new QFileDialog(0);
    okno_przegladania->setFileMode(QFileDialog::ExistingFile);
    connect(okno_przegladania, SIGNAL(fileSelected(QString)), this, SLOT(wybranoPlik(QString)));
    inicjalizacja_CRC();
}

void MainWindow::setPredkoscTransmisji(DWORD value)
{
    parametry_transmisji.BaudRate = value;
}

void MainWindow::setBityParzystosci(DWORD value)
{
    parametry_transmisji.Parity = value;
}

void MainWindow::setBityStopu(DWORD value)
{
    parametry_transmisji.StopBits = value;
}

MainWindow::~MainWindow()
{
    CloseHandle(uchwyt_do_portu);
    delete popup;
    delete okno_przegladania;
    delete ui;
}

void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    switch(index)
    {
    case 1: setPredkoscTransmisji(CBR_110); break;
    case 2: setPredkoscTransmisji(CBR_300); break;
    case 3: setPredkoscTransmisji(CBR_600); break;
    case 4: setPredkoscTransmisji(CBR_1200); break;
    case 5: setPredkoscTransmisji(CBR_2400); break;
    case 6: setPredkoscTransmisji(CBR_4800); break;
    case 7: setPredkoscTransmisji(CBR_9600); break;
    case 8: setPredkoscTransmisji(CBR_14400); break;
    case 9: setPredkoscTransmisji(CBR_19200); break;
    case 10: setPredkoscTransmisji(CBR_38400); break;
    case 11: setPredkoscTransmisji(CBR_57600); break;
    case 12: setPredkoscTransmisji(CBR_115200); break;
    case 13: setPredkoscTransmisji(CBR_128000); break;
    case 14: setPredkoscTransmisji(CBR_256000); break;
    }
}

void MainWindow::on_comboBox_2_currentIndexChanged(int index)
{
    switch(index)
    {
    case 1: setBityParzystosci(ODDPARITY); break;
    case 2: setBityParzystosci(EVENPARITY); break;
    case 3: setBityParzystosci(MARKPARITY); break;
    case 4: setBityParzystosci(SPACEPARITY); break;
    case 5: setBityParzystosci(NOPARITY); break;
    }
}



void MainWindow::on_comboBox_3_currentIndexChanged(int index)
{
    switch(index)
    {
    case 1: setBityStopu(ONESTOPBIT); break;
    case 2: setBityStopu(ONE5STOPBITS); break;
    case 3: setBityStopu(TWOSTOPBITS); break;
    }
}



void MainWindow::on_pushButton_clicked()
{
    if (!SetCommState(uchwyt_do_portu, &parametry_transmisji))
    {
        popup->setText("B��d inicjalizacji portu");
        popup->show();
    }
    popup->setText("Inicjalizacja pomy�lna");
    popup->show();
    timeouts.ReadIntervalTimeout=50;
    timeouts.ReadTotalTimeoutConstant=50;
    timeouts.ReadTotalTimeoutMultiplier=10;
    timeouts.WriteTotalTimeoutConstant=50;
    timeouts.WriteTotalTimeoutMultiplier=10;
    if(!SetCommTimeouts(uchwyt_do_portu, &timeouts)){
        popup->setText("timeoutsetfault");
        popup->show();
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    okno_przegladania->show();
}

void MainWindow::wybranoPlik(QString text)
{
    ui->lineEdit->setText(text);
}

void MainWindow::nadawanie()
{
    QString nazwa = ui->lineEdit->text();
    string nazwaB = nazwa.toStdString();
    int rozmiar; // dla plikow mniejszych niz 2GB mozna uzyc typu int
    char *memblock; //bufor dla pliku
    ifstream plik(nazwaB.c_str(), ios::in|ios::binary|ios::ate);
    if(plik.is_open())
    {
        rozmiar = plik.tellg(); // wczytanie rozmiaru pliku
        memblock = new char[rozmiar];
        plik.seekg(0, ios::beg); // ustawienie kursora na poczatku pliku
        plik.read(memblock, rozmiar); // wczytanie pliku do bufora
        plik.close(); // zamkniecie pliku
    }

    //Wyliczenie liczby pakietow
    int liczba_pakietow = ceil(rozmiar / 128.0);

    //Sprawdzenie czy odbiornik jest gotowy do przyjecia pliku, tj. czy wyslany jest sygnal NAK
    bool brak_odpowiedzi = true;
    for(int i = 0; i < 20; i++)
    {
        unsigned char sprawdz = pobierzZnakzPortu();
        if(sprawdz == NAK || sprawdz == C)
        {
            brak_odpowiedzi = false;
            break;
        }
        else
        {
            Sleep(10);
        }
    }
    if(brak_odpowiedzi)
    {
        popup->setText("Brak odpowiedzi odbiornika");
        popup->show();
    }
    else
    {
        unsigned char nr_pakietu = 1;
        unsigned char zmienna;
        unsigned char naglowek[3];
        unsigned char checksum;
        ushort CRC_16;
        for(int i = 0; i < liczba_pakietow; )
        {
            checksum = 0;
            CRC_16 = 0;
            naglowek[0] = SOH;
            naglowek[1] = nr_pakietu;
            naglowek[2] = 255 - nr_pakietu;
            wyslijZnakdoPortu(naglowek[0]);
            wyslijZnakdoPortu(naglowek[1]);
            wyslijZnakdoPortu(naglowek[2]);
            for(int j = 0; j < 128; j++)
            {
                if(((nr_pakietu - 1) * 128 + j) < rozmiar)
                {
                    wyslijZnakdoPortu(memblock[(nr_pakietu - 1) * 128 + j]);
                    if(ui->checkBox->isChecked())
                    {
                        CRC_16 = (CRC_16 << 8) ^ Tablica_crc16[(CRC_16 >> 8) ^ memblock[(nr_pakietu - 1) * 128 + j]];
                    }
                    else
                    {
                        checksum += memblock[(nr_pakietu - 1) * 128 + j];
                    }
                }
                else
                {
                    wyslijZnakdoPortu(26);
                    if(ui->checkBox->isChecked())
                    {
                        CRC_16 = (CRC_16 << 8) ^ Tablica_crc16[(CRC_16 >> 8) ^ 26];
                    }
                    else
                    {
                        checksum += 26;
                    }
                }
            }
            if(ui->checkBox->isChecked())
            {
                wyslijZnakdoPortu(CRC_16 >> 8);
                wyslijZnakdoPortu(CRC_16);
            }
            else
            {
                wyslijZnakdoPortu(checksum);
            }
            while(zmienna != ACK && zmienna != NAK && zmienna != C)
            {
                zmienna = pobierzZnakzPortu();
            }
            if(zmienna == ACK || zmienna == C)
            {
                nr_pakietu++;
                i++;
            }
        }
        do
        {
            wyslijZnakdoPortu(EOT);
            Sleep(10);
        }while(pobierzZnakzPortu() != ACK);
        popup->setText("Nadajnik:Wys�ano");
        popup->show();
    }
}

void MainWindow::odbieranie()
{
    bool wysylanie = false;
    int i = 0;
    unsigned char czy_koniec;
    while(!wysylanie && i < 5)
    {
        wyslijZnakdoPortu(NAK);
        Sleep(10000);
        if(pobierzZnakzPortu() == SOH)
        {
            wysylanie = true;
            czy_koniec = SOH;
        }
        else
        {
            i++;
        }
    }

    if(wysylanie)
    {
        int rozmiar = 16;
        char *memblock = new char[rozmiar];
        while(czy_koniec != EOT)
        {
            unsigned char nr_pakietu = pobierzZnakzPortu();
            unsigned char dopelnienie = pobierzZnakzPortu();
            unsigned char checksum = 0;
            ushort CRC_16 = 0;
            for(int i = 0; i < 16; i++)
            {
                memblock[i] = pobierzZnakzPortu();
                if(ui->checkBox->isChecked())
                {
                    CRC_16 = (CRC_16 << 8) ^ Tablica_crc16[(CRC_16 >> 8) ^ memblock[i]];
                }
                else
                {
                    checksum += memblock[i];
                }
            }
            if(ui->checkBox->isChecked())
            {
                ushort buffer = (pobierzZnakzPortu() << 8);
                buffer = buffer | pobierzZnakzPortu();
                if(buffer == CRC_16)
                {
                    if(dopelnienie != (255 - nr_pakietu))
                    {
                        wyslijZnakdoPortu(NAK);
                    }
                    else
                    {
                        wyslijZnakdoPortu(ACK);
                        ofstream plik("C:/b.bin", ios::out|ios::binary|ios::app);
                        plik.write(memblock, rozmiar);
                        plik.close();
                    }
                }
                else
                {
                    wyslijZnakdoPortu(NAK);
                }
            }
            else
            {
                if(pobierzZnakzPortu() == checksum)
                {
                    if(dopelnienie != (255 - nr_pakietu))
                    {
                        wyslijZnakdoPortu(NAK);
                    }
                    else
                    {
                        wyslijZnakdoPortu(ACK);
                        ofstream plik("C:/b.bin", ios::out|ios::binary|ios::app);
                        plik.write(memblock, rozmiar);
                        plik.close();
                    }
                }
                else
                {
                    wyslijZnakdoPortu(NAK);
                }
            }
            czy_koniec = pobierzZnakzPortu();
        }
        wyslijZnakdoPortu(ACK);
        popup->setText("Odbiornik:Odebrano");
        popup->show();
    }
    else
    {
        popup->setText("Nadajnik nie rozpocz�� wysy�ania");
        popup->show();
    }
}

unsigned char MainWindow::pobierzZnakzPortu()
{
    unsigned char buffer;
    DWORD Bajty_odczytane = 0;
    if(!ReadFile(uchwyt_do_portu, &buffer, 1, &Bajty_odczytane, NULL))
    {
        popup->setText("hahah");
        popup->show();
    }
    return buffer;
}

void MainWindow::wyslijZnakdoPortu(unsigned char znak)
{

    DWORD Bajty_odczytane = 0;
    if(!WriteFile(uchwyt_do_portu, &znak, 1, &Bajty_odczytane, NULL))
    {
        popup->setText("hahah");
        popup->show();
    }
}

void MainWindow::on_pushButton_3_clicked()
{
    if(ui->comboBox_4->currentIndex() == 0)
    {
        nadawanie();
    }
    else
    {
        odbieranie();
    }
}

void MainWindow::on_comboBox_5_currentIndexChanged(int index)
{
    switch(index)
    {
    case 1: uchwyt_do_portu = CreateFileA("//./COM16",
                                          GENERIC_READ | GENERIC_WRITE,
                                          0,
                                          0,
                                          OPEN_EXISTING,
                                          FILE_ATTRIBUTE_NORMAL,
                                          0); break;
    case 2: uchwyt_do_portu = CreateFileA("//./COM17",
                                          GENERIC_READ | GENERIC_WRITE,
                                          0,
                                          0,
                                          OPEN_EXISTING,
                                          FILE_ATTRIBUTE_NORMAL,
                                          0); break;
    case 3: uchwyt_do_portu = CreateFileA("//./COM18",
                                          GENERIC_READ | GENERIC_WRITE,
                                          0,
                                          0,
                                          OPEN_EXISTING,
                                          FILE_ATTRIBUTE_NORMAL,
                                          0); break;
    }
    if(uchwyt_do_portu == INVALID_HANDLE_VALUE)
    {
        if(GetLastError()==ERROR_FILE_NOT_FOUND){
            popup->setText("Nie znaleziono portu COM");
            popup->show();
        }
        popup->setText("B��d portu COM");
        popup->show();
    }

    //Implemetacja mozliwosci zmiany parametrow portu
    parametry_transmisji.DCBlength = sizeof(DCB);
    if(!GetCommState(uchwyt_do_portu, &parametry_transmisji))
    {
        popup->setText("B��d wczytania konfiguracji");
        popup->show();
    }
    parametry_transmisji.ByteSize = 8;
    ui->comboBox->setEnabled(1);
    ui->comboBox_2->setEnabled(1);
    ui->comboBox_3->setEnabled(1);
    ui->comboBox_4->setEnabled(1);
    ui->pushButton->setEnabled(1);
}

void MainWindow::inicjalizacja_CRC() //tworzenie sumy CRC dla ka�dych 8 bit�w
{
    ushort crc_znak;
    ushort crc;
    for(int i = 0; i < 256; i++)
    {
        crc_znak = i;
        crc = 0;
        for(int j = 8; j > 0; j--)
        {
            if(j != 8)
            {
                crc_znak <<= 1;
            }
            if( crc_znak & 0x80)
            {
                crc_znak = crc_znak ^ 0x88;
                crc <<= 1;
                crc = crc ^ 0x1021;
            }
            else
            {
                crc <<= 1;
            }
        }
        Tablica_crc16[i] = crc;
    }
}
