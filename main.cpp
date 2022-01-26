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
                    QStyleNinePatchImage *npImage = new QStyleNinePatchImage(QImage(fileName));
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

    QSize imageSize(const QString &baseName, const QStyleOption *option) const
    {
        // try with different endings, .9., @2x. etc, and append .png

        static const QString nine = QStringLiteral(".9");
        static const QString png = QStringLiteral(".png");

        QString fileName = baseName + nine + png;
        if (QStyleNinePatchImage *npImage = m_ninePatchImages[fileName])
            return npImage->m_image.size();

        fileName = baseName + png;
        if (m_pixmaps.contains(fileName))
            return m_pixmaps[fileName].size();

        return QSize();
    }

    bool drawImage(const QString &baseName, const QStyleOption *option, QPainter *painter) const
    {
        // try with different endings, .9., @2x. etc, and append .png

        static const QString nine = QStringLiteral(".9");
        static const QString png = QStringLiteral(".png");

        QString fileName = baseName + nine + png;
        if (QStyleNinePatchImage *npImage = m_ninePatchImages[fileName]) {
            npImage->draw(*painter, option->rect);
            return true;
        }

        fileName = baseName + png;
        if (m_pixmaps.contains(fileName)) {
            painter->drawPixmap(option->rect.topLeft(), m_pixmaps[fileName]);
            return true;
        }

        qWarning() << "Could not resolve image:" << baseName;
        return false;
    }

    QString baseNameButton(const QStyleOptionButton *option) const
    {
        QString fileName = QStringLiteral(":/images/%1").arg(QStringLiteral("button-background"));
        if (option->state & QStyle::State_On)
            fileName += QStringLiteral("-checked");
        if (option->state & QStyle::State_Sunken)
            fileName += QLatin1String("-pressed");
        return fileName;
    }

    QString baseNameCheckBox(const QStyleOptionButton *option) const
    {
        QString fileName = QStringLiteral(":/images/%1").arg(QStringLiteral("checkbox-indicator"));
        if (option->state & QStyle::State_On)
            fileName += QStringLiteral("-checked");
        if (option->state & QStyle::State_Sunken)
            fileName += QLatin1String("-pressed");
        return fileName;
    }

    void drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const override
    {
        switch (element) {
        case CE_PushButton:
            if (const QStyleOptionButton *buttonOption = qstyleoption_cast<const QStyleOptionButton *>(option)) {
                if (drawImage(baseNameButton(buttonOption), option, painter)) {
                    QStyleOptionButton subopt = *buttonOption;
                    subopt.rect = subElementRect(SE_PushButtonContents, buttonOption, widget);
                    proxy()->drawControl(CE_PushButtonLabel, &subopt, painter, widget);
                }
                return;
            }
        case CE_CheckBox: {
            if (const QStyleOptionButton *buttonOption = qstyleoption_cast<const QStyleOptionButton *>(option)) {
                if (drawImage(baseNameCheckBox(buttonOption), buttonOption, painter)) {
                    QStyleOptionButton subopt = *buttonOption;
                    subopt.rect = subElementRect(SE_PushButtonContents, buttonOption, widget);
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
            if (const QStyleOptionButton *buttonOption = qstyleoption_cast<const QStyleOptionButton *>(option)) {
                const QSize pixmapSize = imageSize(baseNameButton(buttonOption), buttonOption);
                if (!pixmapSize.isNull())
                    return pixmapSize;
            }
        }
        default:
            break;
        }

        return QProxyStyle::sizeFromContents(type, option, size, widget);
    }

private:
    QMap<QString, QStyleNinePatchImage*> m_ninePatchImages;
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
