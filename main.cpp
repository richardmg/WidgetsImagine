#include <QApplication>

#include "mainwindow.h"
#include "qimaginestyle.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyle(new QImagineStyle(QStringLiteral(":/images")));

    MainWindow w;
    w.show();

    return app.exec();
}
