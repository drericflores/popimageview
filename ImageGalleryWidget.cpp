#include "ImageGalleryWidget.h"
#include <QDir>
#include <QListWidgetItem>
#include <QDebug>
#include <QImage>
#include <QPixmap>
#include <QFuture>
#include <QtConcurrent> // For asynchronous thumbnail generation
#include <QMetaObject>  // For QMetaObject::invokeMethod
#include <QFileInfo>    // For QFileInfo to get filename

ImageGalleryWidget::ImageGalleryWidget(QWidget* parent) : QListWidget(parent) {
    connect(this, &QListWidget::itemClicked, this, &ImageGalleryWidget::onItemClicked);

    // Configure the list widget for image gallery appearance
    setViewMode(QListView::IconMode);
    setIconSize(QSize(128, 128)); // Adjust thumbnail size
    setResizeMode(QListView::Adjust);
    setWrapping(false);          // Important for a vertical gallery
    setFlow(QListView::TopToBottom); // Arrange items vertically
    setSpacing(5);               // Spacing between items
    setUniformItemSizes(true);   // Optimizes rendering for uniform items
}

void ImageGalleryWidget::loadImagesFromDirectory(const QString& directory) {
    clear(); // Clear existing items
    m_currentDirectory = directory;
    QDir dir(directory);
    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp" << "*.gif" << "*.tiff" << "*.tif"; // Common image formats
    QStringList entries = dir.entryList(filters, QDir::Files | QDir::NoDotAndDotDot, QDir::Name); // Sort by name

    for (const QString& fileName : entries) {
        QListWidgetItem* item = new QListWidgetItem(fileName);
        // Add placeholder icon immediately
        // Consider creating a simple placeholder icon resource file (e.g., icons.qrc)
        // For now, no icon, or replace with a default if you have one.
        // item->setIcon(QIcon(":/icons/placeholder.png"));
        addItem(item);

        // Asynchronously generate thumbnail
        // QtConcurrent::run returns a QFuture, which can be used to monitor completion.
        // The `item` pointer must be handled carefully as it could be deleted if the list changes.
        // For this example, we trust the `item` to exist long enough.
        QtConcurrent::run(this, &ImageGalleryWidget::generateThumbnail, QDir(m_currentDirectory).filePath(fileName), item);
    }
    qDebug() << "Loaded" << entries.size() << "images from" << directory;
}

void ImageGalleryWidget::generateThumbnail(const QString& imagePath, QListWidgetItem* item) {
    QImage image(imagePath);
    if (!image.isNull()) {
        QPixmap thumbnail = QPixmap::fromImage(image.scaled(iconSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        // Update the item's icon on the GUI thread using QMetaObject::invokeMethod
        // This is crucial for thread safety when modifying GUI elements
        QMetaObject::invokeMethod(this, [item, thumbnail]() {
            // Check if item is still valid before updating, especially if list can change dynamically
            if (item) {
                item->setIcon(QIcon(thumbnail));
            }
        });
    } else {
         qWarning() << "Failed to load image for thumbnail:" << imagePath;
    }
}


QString ImageGalleryWidget::currentImagePath() const {
    if (currentItem()) {
        return QDir(m_currentDirectory).filePath(currentItem()->text());
    }
    return QString();
}

void ImageGalleryWidget::selectImage(const QString& path) {
    QFileInfo fileInfo(path);
    QList<QListWidgetItem*> items = findItems(fileInfo.fileName(), Qt::MatchExactly);
    if (!items.isEmpty()) {
        setCurrentItem(items.first());
        scrollToItem(items.first()); // Scroll to the selected item
    } else {
        qDebug() << "Could not find item for path:" << path;
    }
}

void ImageGalleryWidget::onItemClicked(QListWidgetItem* item) {
    emit imageSelected(QDir(m_currentDirectory).filePath(item->text()));
}