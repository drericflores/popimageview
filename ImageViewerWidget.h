#ifndef IMAGEVIEWERWIDGET_H
#define IMAGEVIEWERWIDGET_H

#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QMatrix>
#include <QResizeEvent>
#include <QTransform>
#include <QPoint> // For QPoint

class ImageViewerWidget : public QWidget {
    Q_OBJECT
public:
    ImageViewerWidget(QWidget* parent = nullptr);

    // Primary setter for new images (resets transformations)
    void setImage(const QImage& image);
    // Setter for undo/redo (changes image data but preserves transformations)
    void setImageOnly(const QImage& image); // NEW: For undo/redo to change image data without resetting view transforms

    QImage currentImage() const { return m_displayedImage; }
    QImage getOriginalImage() const { return m_originalImage; } // Getter for current base image (after filters)
    QImage getOriginalImageSource() const { return m_originalImageSource; } // NEW: Getter for the pristine image when first loaded

    void setZoomFactor(qreal factor);
    qreal getZoomFactor() const { return m_zoomFactor; } // NEW: Getter for zoom factor

    void setRotationAngle(qreal angle) { m_rotationAngle = angle; applyTransformations(); update(); } // NEW: Setter for rotation angle
    qreal getRotationAngle() const { return m_rotationAngle; } // NEW: Getter for rotation angle

    void setFlipHorizontal(bool flipped) { m_flippedHorizontal = flipped; applyTransformations(); update(); } // NEW: Setter for horizontal flip
    bool getFlipHorizontal() const { return m_flippedHorizontal; } // NEW: Getter for horizontal flip

    void setFlipVertical(bool flipped) { m_flippedVertical = flipped; applyTransformations(); update(); } // NEW: Setter for vertical flip
    bool getFlipVertical() const { return m_flippedVertical; } // NEW: Getter for vertical flip

    void setScrollOffset(const QPoint& offset) { m_scrollOffset = offset; update(); } // NEW: Setter for scroll offset
    QPoint getScrollOffset() const { return m_scrollOffset; } // NEW: Getter for scroll offset


    void zoomIn();
    void zoomOut();
    void rotate(int angle); // This will call setRotationAngle internally
    void flipHorizontal();  // This will call setFlipHorizontal internally
    void flipVertical();    // This will call setFlipVertical internally

    // Basic image filters
    void applyGrayscale();
    void applySepia();
    void applyNegative();

    void fitImageToView();

protected:
    void paintEvent(QPaintEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;


private:
    QImage m_originalImageSource; // NEW: Stores the truly original image data as loaded from file
    QImage m_originalImage;       // The current base image data (after filters applied)
    QImage m_displayedImage;      // The image after applying transformations (zoom, rotate, flip)

    qreal m_zoomFactor;
    QPoint m_scrollOffset;
    qreal m_rotationAngle;
    bool m_flippedHorizontal;
    bool m_flippedVertical;
    QPoint m_lastMousePos;

    void applyTransformations();
    void resetTransformations(); // NEW: Helper to reset viewer state
};

#endif // IMAGEVIEWERWIDGET_H