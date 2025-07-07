#ifndef IMAGEDATAMANAGER_H
#define IMAGEDATAMANAGER_H

#include <QObject>
#include <QImage>
#include <QMap>
#include <QString>
#include <QImageReader> // For QImageReader

// For EXIF metadata - Temporarily commented out due to "No such file or directory" error
// #ifdef QT_MULTIMEDIA_LIB
// #include <QExifImageReader> // Qt5 and newer
// #endif

class ImageDataManager : public QObject {
    Q_OBJECT
public:
    ImageDataManager(QObject* parent = nullptr);
    QImage loadImage(const QString& path);
    QMap<QString, QString> getImageMetadata(const QString& path);

signals:
    void imageLoaded(const QImage& image);
    void metadataReady(const QMap<QString, QString>& metadata);
};

#endif // IMAGEDATAMANAGER_H