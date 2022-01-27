#include "ninepatch.h"
#include <QRect>

QStyleNinePatchImage::QStyleNinePatchImage(const QImage &image)
    : m_image(image)
{
    updateContentArea();
    getResizeArea();
    if (!m_resizeDistancesX.size() || !m_resizeDistancesY.size()) {
        throw new ExceptionNot9Patch;
    }
}

QStyleNinePatchImage::~QStyleNinePatchImage()
{
}

void QStyleNinePatchImage::draw(QPainter *painter, const QRect &targetRect) const
{
    const QPoint pos = targetRect.topLeft();
    const QSize size = targetRect.size();
    const_cast<QStyleNinePatchImage *>(this)->setImageSize(size.width(), size.height());
    painter->drawImage(pos.x(), pos.y(), m_cachedImage);
}

QSize QStyleNinePatchImage::size() const
{
    return m_image.size();
}

void QStyleNinePatchImage::setImageSize(int width, int height) {
    int resizeWidth = 0;
    int resizeHeight = 0;

    for (int i = 0; i < m_resizeDistancesX.size(); i++) {
          resizeWidth += m_resizeDistancesX[i].second;
    }

    for (int i = 0; i < m_resizeDistancesY.size(); i++) {
          resizeHeight += m_resizeDistancesY[i].second;
    }

    width = qMax(width, (m_image.width() - 2 - resizeWidth));
    height = qMax(height, (m_image.height() - 2 - resizeHeight));

    if (width != OldWidth || height != OldHeight) {
        OldWidth = width;
        OldHeight = height;
        updateCachedImage(width, height);
    }
}

void QStyleNinePatchImage::drawScaledPart(QRect oldRect, QRect newRect, QPainter& painter) {
    if (newRect.width() && newRect.height()) {
        QImage img = m_image.copy(oldRect);
        img = img.scaled(newRect.width(), newRect.height());
        painter.drawImage(newRect.x(), newRect.y(), img, 0, 0, newRect.width(), newRect.height());
    }
}

void QStyleNinePatchImage::drawConstPart(QRect oldRect, QRect newRect, QPainter& painter) {
    QImage img = m_image.copy(oldRect);
    painter.drawImage(newRect.x(), newRect.y(), img, 0, 0, newRect.width(), newRect.height());
}

inline bool pixelsBlack(QRgb color)
{
    const auto r = qRed(color);
    const auto g = qGreen(color);
    const auto b = qBlue(color);
    const auto a = qAlpha(color);
    if (a < 128)
        return false;

    return (r < 128 && g < 128 && b < 128);
}

void QStyleNinePatchImage::updateContentArea()
{
    const int bottomLine = m_image.height() - 1;
    const int rightLine = m_image.width() - 1;

    // Find start and end pixel for the bottom line
    int left = 0, right = 0;
    for (int x = 0; x < m_image.width(); x++) {
        if (pixelsBlack(m_image.pixel(x, bottomLine))) {
            if (left == 0)
                left = x;
            else
                right = x;
        }
    }

    int top = 0, bottom = 0;
    for (int y = 0; y < m_image.height(); y++) {
        if (pixelsBlack(m_image.pixel(rightLine, y))) {
            if (top == 0)
                top = y;
            else
                bottom = y;
        }
    }

    if (right == 0)
        right = left;
    if (bottom == 0)
        bottom = top;

    left -= 1; // why do we do this?
    top -= 1;

    m_contentArea = QRect(left, top, right - left, bottom - top);
}

void QStyleNinePatchImage::getResizeArea() {
    int  j = 0;
    int  left = 0;
    int  right = 0;
    for(int  i = 0; i < m_image.width(); i++) {
        if (pixelsBlack(m_image.pixel(i, j)) && left == 0) {
            left = i;
        }
        if (left && pixelsBlack(m_image.pixel(i, j)) && !pixelsBlack(m_image.pixel(i+1, j))) {
            right = i;
            left -= 1;
            m_resizeDistancesX.push_back(std::make_pair(left, right - left));
            right = 0;
            left = 0;
        }
    }
    int  i = 0;
    int  top = 0;
    int  bot = 0;
    for(int  j = 0; j < m_image.height(); j++) {
        if (pixelsBlack(m_image.pixel(i, j)) && top == 0) {
            top = j;
        }
        if (top && pixelsBlack(m_image.pixel(i, j)) && !pixelsBlack(m_image.pixel(i, j+1))) {
            bot = j;
            top -= 1;
            m_resizeDistancesY.push_back(std::make_pair(top, bot - top));
            top = 0;
            bot = 0;
        }
    }
}

void QStyleNinePatchImage::getFactor(int width, int height, double& factorX, double& factorY) {
    int topResize = width - (m_image.width() - 2);
    int leftResize = height - (m_image.height() - 2);
    for (int i = 0; i < m_resizeDistancesX.size(); i++) {
        topResize += m_resizeDistancesX[i].second;
        factorX += m_resizeDistancesX[i].second;
    }
    for (int i = 0; i < m_resizeDistancesY.size(); i++) {
        leftResize += m_resizeDistancesY[i].second;
        factorY += m_resizeDistancesY[i].second;
    }
    factorX = (double)topResize / factorX;
    factorY = (double)leftResize / factorY;
}

void QStyleNinePatchImage::updateCachedImage(int width, int height) {
    m_cachedImage =  QImage(width, height, QImage::Format_ARGB32_Premultiplied);
    m_cachedImage.fill(QColor(0,0,0,0));
    QPainter painter(&m_cachedImage);
    double factorX = 0.0;
    double factorY = 0.0;
    getFactor(width, height, factorX, factorY);
    double lostX = 0.0;
    double lostY = 0.0;
    int x1 = 0; //for image parts X
    int y1 = 0; //for image parts Y
    int widthResize; //width for image parts
    int heightResize; //height for image parts
    int resizeX = 0;
    int resizeY = 0;
    int offsetX = 0;
    int offsetY = 0;
    for (int  i = 0; i < m_resizeDistancesX.size(); i++) {
        y1 = 0;
        offsetY = 0;
        lostY = 0.0;
        for (int  j = 0; j < m_resizeDistancesY.size(); j++) {
            widthResize = m_resizeDistancesX[i].first - x1;
            heightResize = m_resizeDistancesY[j].first - y1;

            drawConstPart(QRect(x1 + 1, y1 + 1, widthResize, heightResize),
                          QRect(x1 + offsetX, y1 + offsetY, widthResize, heightResize), painter);

            int  y2 = m_resizeDistancesY[j].first;
            heightResize = m_resizeDistancesY[j].second;
            resizeY = round((double)heightResize * factorY);
            lostY += resizeY - ((double)heightResize * factorY);
            if (fabs(lostY) >= 1.0) {
                if (lostY < 0) {
                    resizeY += 1;
                    lostY += 1.0;
                } else {
                    resizeY -= 1;
                    lostY -= 1.0;
                }
            }
            drawScaledPart(QRect(x1 + 1, y2 + 1, widthResize, heightResize),
                           QRect(x1 + offsetX, y2 + offsetY, widthResize, resizeY), painter);

            int  x2 = m_resizeDistancesX[i].first;
            widthResize = m_resizeDistancesX[i].second;
            heightResize = m_resizeDistancesY[j].first - y1;
            resizeX = round((double)widthResize * factorX);
            lostX += resizeX - ((double)widthResize * factorX);
            if (fabs(lostX) >= 1.0) {
                if (lostX < 0) {
                    resizeX += 1;
                    lostX += 1.0;
                } else {
                    resizeX -= 1;
                    lostX -= 1.0;
                }
            }
            drawScaledPart(QRect(x2 + 1, y1 + 1, widthResize, heightResize),
                           QRect(x2 + offsetX, y1 + offsetY, resizeX, heightResize), painter);

            heightResize = m_resizeDistancesY[j].second;
            drawScaledPart(QRect(x2 + 1, y2 + 1, widthResize, heightResize),
                           QRect(x2 + offsetX, y2 + offsetY, resizeX, resizeY), painter);

            y1 = m_resizeDistancesY[j].first + m_resizeDistancesY[j].second;
            offsetY += resizeY - m_resizeDistancesY[j].second;
        }
        x1 = m_resizeDistancesX[i].first + m_resizeDistancesX[i].second;
        offsetX += resizeX - m_resizeDistancesX[i].second;
    }
    x1 = m_resizeDistancesX[m_resizeDistancesX.size() - 1].first + m_resizeDistancesX[m_resizeDistancesX.size() - 1].second;
    widthResize = m_image.width() - x1 - 2;
    y1 = 0;
    lostX = 0.0;
    lostY = 0.0;
    offsetY = 0;
    for (int i = 0; i < m_resizeDistancesY.size(); i++) {
        drawConstPart(QRect(x1 + 1, y1 + 1, widthResize, m_resizeDistancesY[i].first - y1),
                      QRect(x1 + offsetX, y1 + offsetY, widthResize, m_resizeDistancesY[i].first - y1), painter);
        y1 = m_resizeDistancesY[i].first;
        resizeY = round((double)m_resizeDistancesY[i].second * factorY);
        lostY += resizeY - ((double)m_resizeDistancesY[i].second * factorY);
        if (fabs(lostY) >= 1.0) {
            if (lostY < 0) {
                resizeY += 1;
                lostY += 1.0;
            } else {
                resizeY -= 1;
                lostY -= 1.0;
            }
        }
        drawScaledPart(QRect(x1 + 1, y1 + 1, widthResize, m_resizeDistancesY[i].second),
                       QRect(x1 + offsetX, y1 + offsetY, widthResize, resizeY), painter);
        y1 = m_resizeDistancesY[i].first + m_resizeDistancesY[i].second;
        offsetY += resizeY - m_resizeDistancesY[i].second;
    }
    y1 = m_resizeDistancesY[m_resizeDistancesY.size() - 1].first + m_resizeDistancesY[m_resizeDistancesY.size() - 1].second;
    heightResize = m_image.height() - y1 - 2;
    x1 = 0;
    offsetX = 0;
    for (int i = 0; i < m_resizeDistancesX.size(); i++) {
        drawConstPart(QRect(x1 + 1, y1 + 1, m_resizeDistancesX[i].first - x1, heightResize),
                      QRect(x1 + offsetX, y1 + offsetY, m_resizeDistancesX[i].first - x1, heightResize), painter);
        x1 = m_resizeDistancesX[i].first;
        resizeX = round((double)m_resizeDistancesX[i].second * factorX);
        lostX += resizeX - ((double)m_resizeDistancesX[i].second * factorX);
        if (fabs(lostX) >= 1.0) {
            if (lostX < 0) {
                resizeX += 1;
                lostX += 1.0;
            } else {
                resizeX -= 1;
                lostX += 1.0;
            }
        }
        drawScaledPart(QRect(x1 + 1, y1 + 1, m_resizeDistancesX[i].second, heightResize),
                       QRect(x1 + offsetX, y1 + offsetY, resizeX, heightResize), painter);
        x1 = m_resizeDistancesX[i].first + m_resizeDistancesX[i].second;
        offsetX += resizeX - m_resizeDistancesX[i].second;
    }
    x1 = m_resizeDistancesX[m_resizeDistancesX.size() - 1].first + m_resizeDistancesX[m_resizeDistancesX.size() - 1].second;
    widthResize = m_image.width() - x1 - 2;
    y1 = m_resizeDistancesY[m_resizeDistancesY.size() - 1].first + m_resizeDistancesY[m_resizeDistancesY.size() - 1].second;
    heightResize = m_image.height() - y1 - 2;
    drawConstPart(QRect(x1 + 1, y1 + 1, widthResize, heightResize),
                  QRect(x1 + offsetX, y1 + offsetY, widthResize, heightResize), painter);
}

QImagineStyleFixedImage::QImagineStyleFixedImage(const QPixmap &pixmap)
    : QImagineStyleImage()
    , m_pixmap(pixmap)
{
}

void QImagineStyleFixedImage::draw(QPainter *painter, const QRect &targetRect) const
{
    painter->drawPixmap(targetRect.topLeft(), m_pixmap);
}

QSize QImagineStyleFixedImage::size() const
{
    return m_pixmap.size();
}
