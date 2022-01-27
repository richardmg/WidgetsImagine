#pragma once

#include <QImage>
#include <QPainter>
#include <QString>
#include <exception>
#include <string>

class QImagineStyleImage {
public:
    virtual ~QImagineStyleImage() {};
    virtual void draw(QPainter* painter, const QRect &targetRect) const = 0;
    virtual QSize size() const = 0;
};

class QImagineStyleFixedImage : public QImagineStyleImage {
public:
    QImagineStyleFixedImage(const QPixmap &pixmap);
    void draw(QPainter *painter, const QRect &targetRect) const override;
    QSize size() const override;

public:
    QPixmap m_pixmap;
};

class QStyleNinePatchImage : public QImagineStyleImage {
public:
    QStyleNinePatchImage(const QImage& image);
    ~QStyleNinePatchImage();

    void draw(QPainter* painter, const QRect &targetRect) const override;
    QSize size() const override;

private:
    void updateContentArea();
    void updateResizeArea();
    void getFactor(int width, int height, double& factorX, double& factorY);
    void updateCachedImage(int width, int height);
    void drawScaledPart(QRect oldRect, QRect newRect, QPainter& painter);
    void drawConstPart(QRect oldRect, QRect newRect, QPainter& painter);
    void setImageSize(int width, int height);

private:
    QImage m_image;
    QImage m_cachedImage;

    QVector<std::pair< int, int >> m_resizeDistancesX;
    QVector<std::pair< int, int >> m_resizeDistancesY;

    QRect m_contentArea;
};

class NinePatchException : public std::exception {
public:
    virtual const char* what() const throw() override {
        return "Nine patch error";
    }
    mutable std::string m_str;
};

class ExceptionNot9Patch : public NinePatchException {
    virtual const char* what() const throw() override {
        return "It is not nine patch image";
    }
};
