#include "mainwindow.h"

#include <QApplication>
#include <QProxyStyle>
#include <QDirIterator>
#include <QStyleOption>
#include <QPainter>

#include "NinePatchQt/ninepatch.h"

class MyProxyStyle : public QProxyStyle
{
  public:

    void drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const override
    {
        switch (element) {
        case CE_PushButton: {
            QImage image(QStringLiteral(":/images/button-background.9.png"));
            TNinePatch npImage(image);
            npImage.draw(*painter, option->rect);
            break; }
        default:
            QProxyStyle::drawControl(element, option, painter, widget);
            break;
        }
    }

    QSize sizeFromContents(QStyle::ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const override
    {
        QSize widgetSize;

        switch (type)  {
        case CT_PushButton: {
            QPixmap pixmap(QStringLiteral(":/images/button-background.9.png"));
            widgetSize = pixmap.size();
            break;
        }
        default:
            widgetSize = QProxyStyle::sizeFromContents(type, option, size, widget);
            break;
        }
        return widgetSize;
    }
};

void loadAndCacheImages()
{
    QDirIterator it(QStringLiteral(":/images"), { "*.png" }, QDir::Files);
    while (it.hasNext()) {
        const QString fileName = it.next();
        qDebug() << fileName;
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    loadAndCacheImages();
    a.setStyle(new MyProxyStyle);
    MainWindow w;
    w.show();
    return a.exec();
}
