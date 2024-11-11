#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow mainWindow;

    mainWindow.showMaximized();  // Show the main window maximized

    return app.exec();
}
