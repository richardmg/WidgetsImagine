#include "mainwindow.h"

#include <QApplication>
#include <QProxyStyle>
#include <QDirIterator>
#include <QStyleOption>
#include <QPainter>

#include "NinePatchQt/ninepatch.h"

class QImagineStyle : public QProxyStyle
{
  public:

    QImagineStyle(const QString &imagePath)
    {
        // TODO: remove duplicates, only cache images of correct size (@2x, @3x etc).
        QDirIterator it(imagePath, { "*.9.png" }, QDir::Files);
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

    ~QImagineStyle() {
        qDeleteAll(m_ninePatchImages);
    }

    QString resolveFileName(const QString &baseName, const QStyleOption *option) const
    {
        QString fileName = QString(":/images/%1").arg(baseName);
        if (option->state & QStyle::State_Sunken)
            fileName += QLatin1String("-pressed");
        fileName += ".9.png";
        return fileName;
    }

    TNinePatch *resolve9pImage(const QString &baseName, const QStyleOption *option) const
    {
        QString fileName = resolveFileName(baseName, option);
        if (TNinePatch *npImage = m_ninePatchImages[fileName])
            return npImage;
        qWarning() << "Could not resolve image:" << fileName;
        return nullptr;
    }

    void drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const override
    {
        switch (element) {
        case CE_PushButton:
            if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(option)) {
                if (TNinePatch *npImage = resolve9pImage("button-background", option)) {
                    npImage->draw(*painter, option->rect);

                    QStyleOptionButton subopt = *btn;
                    subopt.rect = subElementRect(SE_PushButtonContents, btn, widget);
                    proxy()->drawControl(CE_PushButtonLabel, &subopt, painter, widget);
                }
                return;
            }
        case CE_CheckBox: {
            if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(option)) {
                if (TNinePatch *npImage = resolve9pImage("checkbox-indicator", option)) {
                    npImage->draw(*painter, option->rect);

                    QStyleOptionButton subopt = *btn;
                    subopt.rect = subElementRect(SE_PushButtonContents, btn, widget);
                    proxy()->drawControl(CE_PushButtonLabel, &subopt, painter, widget);
                }
                return;
            }

        }
        default:
            break;
        }

        QProxyStyle::drawControl(element, option, painter, widget);
    }

    QSize sizeFromContents(QStyle::ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const override
    {
        switch (type)  {
        case CT_PushButton: {
            if (TNinePatch *npImage = resolve9pImage("button-background", option))
                return npImage->m_image.size();
        }
        default:
            break;
        }

        return QProxyStyle::sizeFromContents(type, option, size, widget);
    }

private:
    QMap<QString, TNinePatch*> m_ninePatchImages;

};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyle(new QImagineStyle(QStringLiteral(":/images")));

    MainWindow w;
    w.show();

    return app.exec();
}
