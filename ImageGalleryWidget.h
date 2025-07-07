#ifndef IMAGEGALLERYWIDGET_H
#define IMAGEGALLERYWIDGET_H

#include <QListWidget>
#include <QString>
#include <QListWidgetItem>
#include <QSize>       // For icon size
#include <QFuture>     // For QtConcurrent::run
#include <QIcon>       // For QIcon
#include <QPixmap>     // For QPixmap

class ImageGalleryWidget : public QListWidget {
    Q_OBJECT
public:
    ImageGalleryWidget(QWidget* parent = nullptr);
    void loadImagesFromDirectory(const QString& directory);
    QString currentImagePath() const;
    void selectImage(const QString& path); // Selects an image in the gallery by path

signals:
    void imageSelected(const QString& imagePath);

private slots:
    void onItemClicked(QListWidgetItem* item);
    // This will be run in a separate thread. Item lifetime must be managed carefully.
    void generateThumbnail(const QString& imagePath, QListWidgetItem* item);

private:
    QString m_currentDirectory;
};

#endif // IMAGEGALLERYWIDGET_H