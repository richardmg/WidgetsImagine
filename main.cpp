#include "mainwindow.h"

#include <QApplication>
#include <QProxyStyle>
#include <QDirIterator>
#include <QStyleOption>
#include <QPainter>

#include "ninepatch.h"

class QImagineStyle : public QProxyStyle
{
  public:

    QImagineStyle(const QString &imagePath)
    {
        // TODO: remove duplicates, only cache images of correct size (@2x, @3x etc).
        QDirIterator it(imagePath, { "*.png" }, QDir::Files);
        while (it.hasNext()) {
            const QString fileName = it.next();

            if (fileName.contains(QLatin1String(".9."))) {
                try {
                    // does this leak if exception is thrown?
                    TNinePatch *npImage = new TNinePatch(QImage(fileName));
                    m_ninePatchImages.insert(fileName, npImage);
                } catch (NinePatchException *exception) {
                    qDebug() << "load, exception:" << exception->what();
                }
            } else {
                m_pixmaps.insert(fileName, QPixmap(fileName));
                qDebug() << "TODO: Load normal image:" << fileName;
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
        return fileName;
    }

    QSize imageSize(const QString &baseName, const QStyleOption *option) const
    {
        QString fileName9p = resolveFileName(baseName, option) + QStringLiteral(".9.png");
        if (TNinePatch *npImage = m_ninePatchImages[fileName9p]) {
            return npImage->m_image.size();
        }

        QString fileNameNon9p = resolveFileName(baseName, option) + QStringLiteral(".png");
        if (m_pixmaps.contains(fileNameNon9p)) {
            return m_pixmaps[fileNameNon9p].size();
        }

        return QSize();
    }

    bool drawImage(const QString &baseName, const QStyleOption *option, QPainter *painter) const
    {
        QString fileName9p = resolveFileName(baseName, option) + QStringLiteral(".9.png");
        if (TNinePatch *npImage = m_ninePatchImages[fileName9p]) {
            npImage->draw(*painter, option->rect);
            return true;
        }

        QString fileNameNon9p = resolveFileName(baseName, option) + QStringLiteral(".png");
        if (m_pixmaps.contains(fileNameNon9p)) {
            painter->drawPixmap(option->rect.topLeft(), m_pixmaps[fileNameNon9p]);
            return true;
        }

        qWarning() << "Could not resolve image:" << resolveFileName(baseName, option);
        return false;
    }

    void drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const override
    {
        switch (element) {
        case CE_PushButton:
            if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(option)) {
                if (drawImage(QStringLiteral("button-background"), option, painter)) {
                    QStyleOptionButton subopt = *btn;
                    subopt.rect = subElementRect(SE_PushButtonContents, btn, widget);
                    proxy()->drawControl(CE_PushButtonLabel, &subopt, painter, widget);
                }
                return;
            }
        case CE_CheckBox: {
            if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(option)) {
                if (drawImage(QStringLiteral("checkbox-indicator"), option, painter)) {
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
            const QSize pixmapSize = imageSize("button-background", option);
            if (!pixmapSize.isNull())
                return pixmapSize;
        }
        default:
            break;
        }

        return QProxyStyle::sizeFromContents(type, option, size, widget);
    }

private:
    QMap<QString, TNinePatch*> m_ninePatchImages;
    QMap<QString, QPixmap> m_pixmaps;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyle(new QImagineStyle(QStringLiteral(":/images")));

    MainWindow w;
    w.show();

    return app.exec();
}
