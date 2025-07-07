#include "ImageViewerWidget.h"
#include <QPainter>
#include <QWheelEvent>
#include <QDebug>
#include <QtMath>
#include <QTransform>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QRgb> // For pixel manipulation

ImageViewerWidget::ImageViewerWidget(QWidget* parent)
    : QWidget(parent),
      m_zoomFactor(1.0),
      m_rotationAngle(0.0),
      m_flippedHorizontal(false),
      m_flippedVertical(false)
{
    setBackgroundRole(QPalette::Dark);
    setAutoFillBackground(true);
    setMouseTracking(true);
}

void ImageViewerWidget::setImage(const QImage& image) {
    m_originalImageSource = image; // Store the pristine original image
    m_originalImage = image;       // Current base image for filter operations
    resetTransformations();        // Reset all view transformations
    applyTransformations();        // Apply transformations (which are now default)
    fitImageToView();              // Fit to view initially
    update();                      // Request repaint
}

void ImageViewerWidget::setImageOnly(const QImage& image) {
    // This is for undo/redo: changes the base image data without resetting view transforms
    m_originalImage = image;
    // Don't touch m_originalImageSource here, as it's the very first loaded image
    applyTransformations(); // Re-apply current transformations to the new image data
    update();
}

void ImageViewerWidget::setZoomFactor(qreal factor) {
    m_zoomFactor = qMax(0.1, qMin(10.0, factor));
    applyTransformations();
    update();
}

// setRotationAngle, setFlipHorizontal, setFlipVertical, setScrollOffset are defined in header now

void ImageViewerWidget::zoomIn() {
    setZoomFactor(m_zoomFactor * 1.2);
}

void ImageViewerWidget::zoomOut() {
    setZoomFactor(m_zoomFactor / 1.2);
}

void ImageViewerWidget::rotate(int angle) {
    m_rotationAngle += angle;
    m_rotationAngle = fmod(m_rotationAngle, 360.0);
    if (m_rotationAngle < 0) m_rotationAngle += 360;
    applyTransformations();
    update();
}

void ImageViewerWidget::flipHorizontal() {
    m_flippedHorizontal = !m_flippedHorizontal;
    applyTransformations();
    update();
}

void ImageViewerWidget::flipVertical() {
    m_flippedVertical = !m_flippedVertical;
    applyTransformations();
    update();
}

void ImageViewerWidget::applyGrayscale() {
    if (m_originalImage.isNull()) return;
    // Filters operate on m_originalImage. The result becomes the new m_originalImage.
    QImage processedImage = m_originalImage.convertToFormat(QImage::Format_Grayscale8);
    m_originalImage = processedImage;
    applyTransformations(); // Re-apply view transforms to the new filtered image
    update();
}

void ImageViewerWidget::applySepia() {
    if (m_originalImage.isNull()) return;
    QImage processedImage = m_originalImage;

    // Convert to compatible format for pixel manipulation if needed
    if (processedImage.format() == QImage::Format_Indexed8 || processedImage.format() == QImage::Format_Grayscale8) {
        processedImage = processedImage.convertToFormat(QImage::Format_RGB32);
    } else if (processedImage.format() != QImage::Format_RGB32 && processedImage.format() != QImage::Format_ARGB32 && processedImage.format() != QImage::Format_ARGB32_Premultiplied) {
        processedImage = processedImage.convertToFormat(QImage::Format_ARGB32);
    }

    for (int y = 0; y < processedImage.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb*>(processedImage.scanLine(y));
        for (int x = 0; x < processedImage.width(); ++x) {
            int r = qRed(line[x]);
            int g = qGreen(line[x]);
            int b = qBlue(line[x]);

            int newR = qMin(255, static_cast<int>(0.393 * r + 0.769 * g + 0.189 * b));
            int newG = qMin(255, static_cast<int>(0.349 * r + 0.686 * g + 0.168 * b));
            int newB = qMin(255, static_cast<int>(0.272 * r + 0.534 * g + 0.131 * b));

            line[x] = qRgb(newR, newG, newB);
        }
    }
    m_originalImage = processedImage;
    applyTransformations();
    update();
}

void ImageViewerWidget::applyNegative() {
    if (m_originalImage.isNull()) return;
    QImage processedImage = m_originalImage;
    processedImage.invertPixels();
    m_originalImage = processedImage;
    applyTransformations();
    update();
}

void ImageViewerWidget::fitImageToView() {
    if (m_originalImage.isNull() || width() == 0 || height() == 0) {
        m_zoomFactor = 1.0;
        return;
    }

    QTransform rotationTransform;
    rotationTransform.rotate(m_rotationAngle);
    QRectF transformedRect = rotationTransform.mapRect(m_originalImage.rect());

    qreal widgetRatio = (qreal)width() / height();
    qreal imageRatio = transformedRect.width() / transformedRect.height();

    if (imageRatio > widgetRatio) {
        m_zoomFactor = (qreal)width() / transformedRect.width();
    } else {
        m_zoomFactor = (qreal)height() / transformedRect.height();
    }

    m_zoomFactor = qMax(0.01, qMin(100.0, m_zoomFactor));
    applyTransformations();
    update();
}

void ImageViewerWidget::applyTransformations() {
    if (m_originalImage.isNull()) {
        m_displayedImage = QImage();
        return;
    }

    QImage tempImage = m_originalImage;

    // Apply Flip
    if (m_flippedHorizontal) {
        tempImage = tempImage.mirrored(true, false);
    }
    if (m_flippedVertical) {
        tempImage = tempImage.mirrored(false, true);
    }

    // Apply Rotation
    QTransform transform;
    transform.rotate(m_rotationAngle);
    tempImage = tempImage.transformed(transform, Qt::SmoothTransformation);

    // Apply Zoom
    int newWidth = qRound(tempImage.width() * m_zoomFactor);
    int newHeight = qRound(tempImage.height() * m_zoomFactor);

    if (newWidth <= 0 || newHeight <= 0) {
        m_displayedImage = QImage();
        return;
    }

    m_displayedImage = tempImage.scaled(newWidth, newHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void ImageViewerWidget::resetTransformations() {
    m_zoomFactor = 1.0;
    m_rotationAngle = 0.0;
    m_flippedHorizontal = false;
    m_flippedVertical = false;
    m_scrollOffset = QPoint(0,0);
}

void ImageViewerWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.fillRect(rect(), palette().window());

    if (!m_displayedImage.isNull()) {
        int x = (width() - m_displayedImage.width()) / 2 + m_scrollOffset.x();
        int y = (height() - m_displayedImage.height()) / 2 + m_scrollOffset.y();
        painter.drawImage(x, y, m_displayedImage);
    }
}

void ImageViewerWidget::wheelEvent(QWheelEvent* event) {
    if (m_originalImage.isNull()) return;

    if (event->modifiers() & Qt::ControlModifier) {
        if (event->angleDelta().y() > 0) {
            zoomIn();
        } else {
            zoomOut();
        }
        event->accept();
    } else {
        m_scrollOffset += event->pixelDelta().isNull() ? event->angleDelta() / 8 : event->pixelDelta();
        update();
        event->accept();
    }
}

void ImageViewerWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_lastMousePos = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    }
}

void ImageViewerWidget::mouseMoveEvent(QMouseEvent* event) {
    if (event->buttons() & Qt::LeftButton) {
        QPoint delta = event->pos() - m_lastMousePos;
        m_scrollOffset += delta;
        m_lastMousePos = event->pos();
        update();
        event->accept();
    }
}

void ImageViewerWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        setCursor(Qt::ArrowCursor);
        event->accept();
    }
}

void ImageViewerWidget::resizeEvent(QResizeEvent* event) {
    Q_UNUSED(event);
}