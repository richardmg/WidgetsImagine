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
                    m_images.insert(fileName, new QStyleNinePatchImage(QImage(fileName)));
                } catch (NinePatchException *exception) {
                    qDebug() << "load, exception:" << exception->what();
                }
            } else {
                m_images.insert(fileName, new QImagineStyleFixedImage(QPixmap(fileName)));
            }
        }
    }

    ~QImagineStyle() {
        qDeleteAll(m_images);
    }

    QImagineStyleImage *resolveImage(const QString &baseName, const QStyleOption *option) const
    {
        Q_UNUSED(option);
        // try with different endings, .9., @2x. etc, and append .png

        static const QString nine = QStringLiteral(".9");
        static const QString png = QStringLiteral(".png");

        QString fileName = baseName + nine + png;
        if (const auto imagineImage = m_images[fileName])
            return imagineImage;

        fileName = baseName + png;
        if (const auto imagineImage = m_images[fileName])
            return imagineImage;

        return nullptr;
    }

    // -----------------------------------------------------------------------

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

    // -----------------------------------------------------------------------

    void drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const override
    {
        switch (element) {
        case CE_PushButton:
            if (const QStyleOptionButton *buttonOption = qstyleoption_cast<const QStyleOptionButton *>(option)) {
                if (const auto imagineImage = resolveImage(baseNameButton(buttonOption), buttonOption)) {
                    imagineImage->draw(painter, buttonOption->rect);
                    QStyleOptionButton subopt = *buttonOption;
                    subopt.rect = subElementRect(SE_PushButtonContents, buttonOption, widget);
                    proxy()->drawControl(CE_PushButtonLabel, &subopt, painter, widget);
                }
                return;
            }
        case CE_CheckBox: {
            if (const QStyleOptionButton *buttonOption = qstyleoption_cast<const QStyleOptionButton *>(option)) {
                if (const auto imagineImage = resolveImage(baseNameCheckBox(buttonOption), buttonOption)) {
                    imagineImage->draw(painter, buttonOption->rect);
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
                if (const auto imagineImage = resolveImage(baseNameButton(buttonOption), buttonOption))
                    return imagineImage->size();
            }
        }
        default:
            break;
        }

        return QProxyStyle::sizeFromContents(type, option, size, widget);
    }

private:
    QMap<QString, QImagineStyleImage*> m_images;
};

// -----------------------------------------------------------------------

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyle(new QImagineStyle(QStringLiteral(":/images")));

    MainWindow w;
    w.show();

    return app.exec();
}
