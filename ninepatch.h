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
    QRect GetContentArea(int  widht, int  height);
    QRect GetContentArea();
    void GetResizeArea();
    void GetFactor(int width, int height, double& factorX, double& factorY);
    void UpdateCachedImage(int width, int height);
    void DrawScaledPart(QRect oldRect, QRect newRect, QPainter& painter);
    void DrawConstPart(QRect oldRect, QRect newRect, QPainter& painter);
    void SetImageSize(int width, int height);

private:
    QImage m_image;
    int OldWidth = -1;
    int OldHeight = -1;
    QImage CachedImage;
    QVector<std::pair< int, int >>ResizeDistancesY;
    QVector<std::pair< int, int >>ResizeDistancesX;
    QRect ContentArea;
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
