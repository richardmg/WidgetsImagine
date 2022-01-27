#ifndef QIMAGINESTYLE_H
#define QIMAGINESTYLE_H

#include <QApplication>
#include <QScreen>
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

            const bool is2x = fileName.contains(QLatin1String("@2x"));
            const bool is3x = fileName.contains(QLatin1String("@3x"));
            const bool is9p = fileName.contains(QLatin1String(".9."));

            if (is9p) {
                try {
                    // does this leak if exception is thrown?
                    QImage image(fileName);
                    image.setDevicePixelRatio(is2x ? 2.0 : is3x ? 3.0 : 1.0);
                    m_images.insert(fileName, new QStyleNinePatchImage(image));
                } catch (NinePatchException *exception) {
                    qDebug() << "load, exception:" << exception->what();
                }
            } else {
                QPixmap pixmap(fileName);
                pixmap.setDevicePixelRatio(is2x ? 2.0 : is3x ? 3.0 : 1.0);
                m_images.insert(fileName, new QImagineStyleFixedImage(pixmap));
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
        static const QString scale2x = QStringLiteral("@2x");

        // Works for now, but scale factor should really be depending on QPainter paint device dpr?
        QString scale;
        const int dpr = 1.0;//qApp->primaryScreen()->devicePixelRatio();
        if (dpr == 2)
            scale = scale2x;

        QString fileName = baseName + scale + nine + png;
//        qDebug() << "looking for:" << fileName;
        if (const auto imagineImage = m_images[fileName])
            return imagineImage;

        fileName = baseName + scale + png;
//        qDebug() << "looking for:" << fileName;
        if (const auto imagineImage = m_images[fileName])
            return imagineImage;

        qWarning() << "Could not find image:" << baseName;

        return nullptr;
    }

    // -----------------------------------------------------------------------

    QString baseNameButton(const QString &subType, const QStyleOptionButton *option) const
    {
        QString fileName = QStringLiteral(":/images/%1").arg(subType);
        if (option->state & QStyle::State_On)
            fileName += QStringLiteral("-checked");
        else if (option->state & QStyle::State_Sunken)
            fileName += QLatin1String("-pressed");
        return fileName;
    }

    QString baseNameSliderBackground(const QStyleOptionSlider *option) const
    {
        QString fileName = QStringLiteral(":/images/slider-background");
        if (option->state & QStyle::State_Horizontal)
            fileName += QStringLiteral("-horizontal");
        if (option->state & QStyle::State_On)
            fileName += QStringLiteral("-checked");
        return fileName;
    }

    QString baseNameSliderHandle(const QStyleOptionSlider *option) const
    {
        QString fileName = QStringLiteral(":/images/slider-handle");
        if (option->state & QStyle::State_Sunken)
            fileName += QLatin1String("-pressed");
        return fileName;
    }

// -----------------------------------------------------------------------

    void drawPrimitive(
            PrimitiveElement element,
            const QStyleOption *option,
            QPainter *painter,
            const QWidget *widget = nullptr) const override
    {
        switch (element) {
        case PE_IndicatorCheckBox:
            if (const QStyleOptionButton *buttonOption = qstyleoption_cast<const QStyleOptionButton *>(option)) {
                const QString baseName = baseNameButton(QStringLiteral("checkbox-indicator"), buttonOption);
                if (const auto imagineImage = resolveImage(baseName, buttonOption)) {
                    imagineImage->draw(painter, buttonOption->rect);
                    return;
                }
            }
        case PE_IndicatorRadioButton:
            if (const QStyleOptionButton *buttonOption = qstyleoption_cast<const QStyleOptionButton *>(option)) {
                const QString baseName = baseNameButton(QStringLiteral("radiobutton-indicator"), buttonOption);
                if (const auto imagineImage = resolveImage(baseName, buttonOption)) {
                    imagineImage->draw(painter, buttonOption->rect);
                    return;
                }
            }
        default:
            break;
        }

        QProxyStyle::drawPrimitive(element, option, painter, widget);
    }

// -----------------------------------------------------------------------

    void drawControl(
            QStyle::ControlElement element,
            const QStyleOption *option,
            QPainter *painter,
            const QWidget *widget = nullptr) const override
    {
        switch (element) {
        case CE_PushButtonBevel:
            if (const QStyleOptionButton *buttonOption = qstyleoption_cast<const QStyleOptionButton *>(option)) {
                const QString baseName = baseNameButton(QStringLiteral("button-background"), buttonOption);
                if (const auto imagineImage = resolveImage(baseName, buttonOption)) {
                    imagineImage->draw(painter, buttonOption->rect);
                    return;
                }
            }
        case CE_FocusFrame:
            // TODO: adjust size to be outside option->rect
            return;
        default:
            break;
        }

        QProxyStyle::drawControl(element, option, painter, widget);
    }

    void drawComplexControl(
            ComplexControl element,
            const QStyleOptionComplex *option,
            QPainter *painter,
            const QWidget *widget) const override
    {
        switch (element) {
        case CC_Slider:
            if (const auto *sliderOption = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
                const QString baseNameBackground = baseNameSliderBackground(sliderOption);
                if (const auto imagineImage = resolveImage(baseNameBackground, sliderOption)) {
                    imagineImage->draw(painter, sliderOption->rect);
                    const QString baseNameHandle = baseNameSliderHandle(sliderOption);
                    if (const auto imagineImage = resolveImage(baseNameHandle, sliderOption))
                        imagineImage->draw(painter, sliderOption->rect);
                    return;
                }
            }
        default:
            break;
        }

        QProxyStyle::drawComplexControl(element, option, painter, widget);
    }

// -----------------------------------------------------------------------

    QSize sizeFromContents(
            QStyle::ContentsType type,
            const QStyleOption *option,
            const QSize &size,
            const QWidget *widget) const override
    {
        switch (type)  {
        case CT_PushButton: {
            if (const auto *buttonOption = qstyleoption_cast<const QStyleOptionButton *>(option)) {
                const QString baseName = baseNameButton(QStringLiteral("button-background"), buttonOption);
                if (const auto imagineImage = resolveImage(baseName, buttonOption))
                    return imagineImage->size();
            }
            break;
        }
        case CT_CheckBox: {
            if (const auto *buttonOption = qstyleoption_cast<const QStyleOptionButton *>(option)) {
                const QString baseName = baseNameButton(QStringLiteral("checkbox-indicator"), buttonOption);
                if (const auto imagineImage = resolveImage(baseName, buttonOption))
                    return imagineImage->size();
            }
            break;
        }
        case CT_RadioButton: {
            if (const auto *buttonOption = qstyleoption_cast<const QStyleOptionButton *>(option)) {
                const QString baseName = baseNameButton(QStringLiteral("radiobutton-indicator"), buttonOption);
                if (const auto imagineImage = resolveImage(baseName, buttonOption))
                    return imagineImage->size();
            }
            break;
        }
        default:
            break;
        }

        return QProxyStyle::sizeFromContents(type, option, size, widget);
    }

private:
    QMap<QString, QImagineStyleImage*> m_images;
};

#endif // QIMAGINESTYLE_H
