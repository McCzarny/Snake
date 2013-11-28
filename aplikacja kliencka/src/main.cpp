#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    //QTextCodec::setCodecForCStrings(QTextCodec::codecForName ("Windows-1250"));
    //QTextCodec::setCodecForLocale(QTextCodec::codecForName ("Windows-1250"));
    w.show();

    return a.exec();
}
