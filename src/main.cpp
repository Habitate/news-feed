#include "mainwindow.h"

#include <QApplication>

#include <curlpp/cURLpp.hpp>

int main(int argc, char *argv[]){
    QApplication a(argc, argv);

    curlpp::initialize();

    MainWindow w;
    w.show();

    return a.exec();
}
