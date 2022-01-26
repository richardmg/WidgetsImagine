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

    MyProxyStyle()
    {
        loadAndCacheImages();
    }

    ~MyProxyStyle() {
        qDeleteAll(m_ninePatchImages);
    }

    void loadAndCacheImages()
    {
        // TODO: remove duplicates, only cache images of correct size (@2x, @3x etc).
        QDirIterator it(QStringLiteral(":/images"), { "*.9.png" }, QDir::Files);
        while (it.hasNext()) {
            const QString fileName = it.next();
            const QImage image(fileName);
            try {
                // does this leak if exception is thrown?
                TNinePatch *npImage = new TNinePatch(image);
                m_ninePatchImages.insert(fileName, npImage);
            } catch (NinePatchException *exception) {
                qDebug() << "load, exception:" << exception->what();
            }
        }
    }

    void drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const override
    {
        switch (element) {
        case CE_PushButton: {
            if (TNinePatch *npImage = m_ninePatchImages[":/images/button-background.9.png"]) {
                npImage->draw(*painter, option->rect);

                if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(option)) {
                    QStyleOptionButton subopt = *btn;
                    subopt.rect = subElementRect(SE_PushButtonContents, btn, widget);
                    proxy()->drawControl(CE_PushButtonLabel, &subopt, painter, widget);
                }
                return;
            }
            break; }
        default:
            break;
        }

        QProxyStyle::drawControl(element, option, painter, widget);
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

private:
    QMap<QString, TNinePatch*> m_ninePatchImages;

};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    loadAndCacheImages();
    a.setStyle(new MyProxyStyle);
    MainWindow w;
    w.show();
    return a.exec();
}
