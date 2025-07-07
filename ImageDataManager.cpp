#include "ImageDataManager.h"
#include <QDebug>
#include <QImageReader>
#include <QFile>
#include <QDateTime> // For file modification time metadata
#include <QFileInfo> // For file info
#include <QLocale>   // For QLocale::system().toString() to replace deprecated Qt::SystemLocaleLongDate

ImageDataManager::ImageDataManager(QObject* parent) : QObject(parent) {
    // Constructor logic
}

QImage ImageDataManager::loadImage(const QString& path) {
    QImageReader reader(path);
    if (reader.canRead()) {
        QImage image = reader.read();
        if (image.isNull()) {
            qDebug() << "Failed to read image:" << path << reader.errorString();
        } else {
            // Emit signal (though imageViewer is typically connected directly in ImageApplication)
            // emit imageLoaded(image); // This signal can be used if ImageDataManager needs to notify multiple listeners
        }
        return image;
    } else {
        qDebug() << "Cannot read image file (unsupported format or does not exist):" << path;
        return QImage();
    }
}

QMap<QString, QString> ImageDataManager::getImageMetadata(const QString& path) {
    QMap<QString, QString> metadata;
    QFile file(path);
    if (!file.exists()) {
        qWarning() << "File does not exist:" << path;
        return metadata;
    }

    // --- Basic File System Metadata ---
    QFileInfo fileInfo(path);
    metadata["Filename"] = fileInfo.fileName();
    metadata["Path"] = fileInfo.absolutePath();
    metadata["Size"] = QString::number(fileInfo.size() / 1024.0, 'f', 2) + " KB";
    // Fixed deprecated warning: Use QLocale for date/time formatting
    metadata["Last Modified"] = QLocale::system().toString(fileInfo.lastModified(), QLocale::LongFormat);
    metadata["Created"] = QLocale::system().toString(fileInfo.birthTime(), QLocale::LongFormat);
    metadata["Format"] = fileInfo.suffix().toUpper();

    QImageReader reader(path);
    if (reader.canRead()) {
        metadata["Dimensions"] = QString::number(reader.size().width()) + " x " + QString::number(reader.size().height()) + " pixels";
        // REMOVED THE LINE THAT CAUSED THE ERROR: metadata["Depth"] = QString::number(reader.imageFormat().depth()) + " bits";
        // QImageReader::imageFormat() returns an enum, not a class with a depth() method.
        // To get depth, you'd typically need to load the image first (QImage::depth()).
    }

// The following block for QExifImageReader is commented out as requested.
// #ifdef QT_MULTIMEDIA_LIB
//     // --- EXIF Metadata (requires QtMultimedia module) ---
//     QExifImageReader exifReader(path);
//     if (exifReader.canRead()) {
//         QImageMetaData exifData = exifReader.imageMetaData();
//         if (exifData.contains(QImageMetaData::Orientation)) {
//             metadata["Orientation"] = QString::number(exifData.value(QImageMetaData::Orientation).toInt());
//         }
//         if (exifData.contains(QImageMetaData::DateTimeOriginal)) {
//             metadata["Date Taken"] = exifData.value(QImageMetaData::DateTimeOriginal).toDateTime().toString(Qt::SystemLocaleLongDate);
//         }
//         if (exifData.contains(QImageMetaData::Make)) {
//             metadata["Camera Make"] = exifData.value(QImageMetaData::Make).toString();
//         }
//         if (exifData.contains(QImageMetaData::Model)) {
//             metadata["Camera Model"] = exifData.value(QImageMetaData::Model).toString();
//         }
//         if (exifData.contains(QImageMetaData::ExposureTime)) {
//             metadata["Exposure Time"] = exifData.value(QImageMetaData::ExposureTime).toString();
//         }
//         if (exifData.contains(QImageMetaData::FNumber)) {
//             metadata["F-Number"] = QString::number(exifData.value(QImageMetaData::FNumber).toFloat(), 'f', 1);
//         }
//         if (exifData.contains(QImageMetaData::ISOSpeedRatings)) {
//             metadata["ISO Speed"] = QString::number(exifData.value(QImageMetaData::ISOSpeedRatings).toInt());
//         }
//         if (exifData.contains(QImageMetaData::FocalLength)) {
//             metadata["Focal Length"] = QString::number(exifData.value(QImageMetaData::FocalLength).toFloat(), 'f', 1) + " mm";
//         }
//         // Add more EXIF tags as needed
//     } else {
//         qDebug() << "QExifImageReader cannot read EXIF for:" << path << (exifReader.error() == QExifImageReader::UnsupportedFormatError ? " (Unsupported format)" : "");
//     }
// #else
//     qDebug() << "QtMultimedia module not available for full EXIF support.";
// #endif

    emit metadataReady(metadata);
    return metadata;
}