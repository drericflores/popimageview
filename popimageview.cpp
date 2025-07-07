/****************************************
 * Program by Dr. Eric Oliver Flores Toro
 * Coding date:  July 6th, 2025
 * Version:  1
 * Revsion:
 * C++/QT
 ****************************************/
## Project Plan: imageview - A C++ QT Image Viewer for Linux

This document outlines the development plan for **imageview**, a GUI-based image viewer application for Linux, built with C++ and the QT toolkit. The application prioritizes a user-friendly experience with a rich set of features for image viewing and basic manipulation.

---

### 1. Introduction

**imageview** aims to be a robust and intuitive image viewing solution for Linux users. It will leverage the power of C++ and the versatility of the QT framework to provide a performant and visually appealing application. The core focus is on providing essential image viewing features, basic editing capabilities, and a seamless user experience.

---

### 2. Architecture Overview

The application will adopt a **GUI-centric architecture** using QT Widgets. A Model-View-Controller (MVC) or similar pattern will be considered for better separation of concerns, especially for image data management and UI presentation.

**Core Components:**

* **MainWindow:** The primary application window, hosting the image display area, toolbar, menus, and gallery.
* **ImageViewerWidget:** A custom widget responsible for displaying the current image, handling zoom, rotation, and potentially basic filtering.
* **ImageGalleryWidget:** A widget displaying thumbnails of images in the current directory.
* **ImageDataManager:** Manages image loading, metadata extraction, and provides an interface for image manipulation.
* **UndoRedoStack:** Manages a history of operations for undo/redo functionality.
* **SettingsManager:** Handles application preferences, including dark mode, keyboard shortcuts, and default directories.

---

### 3. Functional Features

* **File I/O:**
    * **Open Image File:** Support for common image formats (JPEG, PNG, BMP, TIFF, GIF).
    * **Open Image Directory:** Allow users to navigate and view images within a selected directory.
    * **Default Directory:** Application will default to `/home/Pictures` but allow users to browse and select other directories.
    * **Save/Export Image:** Export current image to various formats (.bmp, .tiff, .png, .pdf).
* **Undo/Redo:** Implement a robust undo/redo mechanism for image transformations (rotation, flip, basic filters).
* **Tabbed Interface:** Allow opening multiple images or directories in separate tabs for concurrent viewing.
* **Clipboard Support:**
    * **Copy Image:** Copy the current image to the system clipboard.
    * **Paste Image:** Paste an image from the system clipboard into a new view or overwrite the current one (if applicable).
* **Image Metadata Display:** If available (e.g., EXIF data), display image metadata in a dedicated panel or dialog.
* **Image Zoom:** Zoom in and out of the image with keyboard shortcuts and toolbar buttons.
* **Image Rotation:** Rotate the image by 0 to 360 degrees.
* **Image Flip:** Flip the image horizontally and vertically.
* **Basic Image Processing (Filters):**
    * Grayscale
    * Sepia
    * Negative
    * Brightness/Contrast adjustment (basic)
* **Vertical Image Gallery:** A pane on the left side of the window displaying a vertical scrollable gallery of thumbnails from the current working directory. Clicking a thumbnail will display the image in the main viewer.

---

### 4. User Experience (UX) Features

* **Dark Mode:** A toggle option to switch between light and dark themes for improved visual comfort, especially in low-light environments.
* **Toolbar:** A customizable toolbar providing quick access to frequently used functions (open, save, zoom, rotate, copy, paste, undo/redo).
* **Context Menus:** Right-click context menus on images and gallery thumbnails for quick actions.
* **Resizable Windows:** The application window and its internal panes (e.g., gallery, metadata panel) will be resizable.
* **Keyboard Shortcuts:** Comprehensive keyboard shortcuts for all major functionalities, with potential for user customization.
* **Visual Preferences:** Save user preferences for theme, zoom level, and window layout.
* **Operational Behaviors:** Ensure smooth image loading, navigation, and responsiveness.

---

### 5. Non-Functional Requirements

* **Performance:** Optimized for quick image loading and smooth navigation, especially with large images. Asynchronous image loading will be implemented to prevent UI blocking.
* **Modularity and Readability:** Code will be structured into logical classes and functions, adhering to C++ best practices for maintainability and extensibility.
* **Security:** The application will not require elevated permissions, following the principle of least privilege.
* **Error Handling:** Robust error handling for file operations, invalid image formats, and other potential issues.

---

### 6. Development Milestones

* **Initial Design (2025-07-06):**
    * Complete architectural design.
    * Define core classes and their responsibilities.
    * Outline UI layout and major components.
    * Set up basic QT project structure.

* **Minimum Viable Product (MVP) (2025-07-06):**
    * Basic window with an image display area.
    * File -> Open Image functionality.
    * Image loading and display.
    * Basic zoom in/out.
    * Navigation (next/previous) within a directory (manual loading of directory via File -> Open Directory).
    * Initial implementation of Dark Mode toggle.

* **Final Delivery (2025-07-06):**
    * Full implementation of all functional and UX features as specified.
    * Thorough testing and bug fixing.
    * Comprehensive documentation.
    * Packaging for AppImage and Source distribution.

---

### 7. Pseudo-code Mapping and Implementation Details

The provided pseudo-code offers a strong foundation. The following outlines how these concepts will be mapped to the `imageview` application using QT.

```cpp
// =============================================================================
// ImageApplication.h - Core Specification for a Linux-based Image Application
// =============================================================================

#include <QApplication>
#include <QMainWindow>
#include <QImage>
#include <QVector>
#include <QDir>
#include <QFileInfo>
#include <QClipboard>
#include <QUndoStack>
#include <QUndoCommand>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QSettings>
#include <QExifImageReader> // For metadata if supported by Qt version or external lib
#include <QImageWriter>
#include <QPrintDialog>
#include <QPrinter>
#include <QPdfWriter> // For PDF export

// Forward declarations
class ImageViewerWidget;
class ImageGalleryWidget;
class ImageDataManager;

enum ZoomMode { FitToScreen, ActualSize, Custom };
enum SortMode { ByName, ByDate, BySize };
// FilterCriteria, Rect, Point, RotationAngle, FilterType, Shape, SelectionType
// will be defined as necessary structs or enums within their respective classes.

class ImageApplication : public QApplication {
    Q_OBJECT // Required for QT signal/slot mechanism

public:
    ImageApplication(int& argc, char** argv);
    ~ImageApplication();

    // Initialization and System Integration
    void InitializeGUI();                        // Setup MainWindow, widgets, and connections
    void LoadLocalization(const QString& locale); // Will use QTranslator
    void RegisterFileTypes();                    // Typically handled by desktop integration post-install (AppImage won't directly)

    // Core Image Viewing Features
    void OpenImageFile(const QString& path);    // Load image into ImageViewerWidget
    void OpenImageDirectory(const QString& directory); // Populate ImageGalleryWidget
    void DisplayImage();                         // Update ImageViewerWidget with current image
    void NavigateNext();                         // Cycle through images in ImageGalleryWidget's current list
    void NavigatePrevious();                     // Cycle through images in ImageGalleryWidget's current list
    void ToggleFullscreen();                     // Toggle QMainWindow's fullscreen state
    void ToggleZoom(ZoomMode mode);              // Call ImageViewerWidget's zoom methods
    void ShowImageMetadata();                    // Display metadata in a QDialog or side panel

    // File and Image Management
    void RenameImage(const QString& newName);    // File system operation + update gallery
    void DeleteImage();                          // File system operation (to trash) + update gallery
    void MoveImageTo(const QString& destinationPath); // File system operation + update gallery
    void SortImages(SortMode mode);              // ImageGalleryWidget will handle sorting
    void FilterImages(FilterCriteria criteria);  // ImageGalleryWidget will handle filtering
    void EnableDragAndDrop();                    // Implement drag/drop events on ImageViewer and Gallery

    // UI/UX Configuration
    void EnableDarkMode(bool enable);            // Apply QPalette changes or stylesheet
    void ConfigureShortcuts();                   // Use QAction shortcuts and QSettings for customization
    void ApplyNativeTheme();                     // Default QT behavior often follows native theme

    // Editing Tools
    void CropImage(QRect region);                // Handled by ImageViewerWidget
    void ResizeImage(int width, int height);     // Handled by ImageViewerWidget
    void RotateImage(int angle);                 // Handled by ImageViewerWidget, push to undo stack
    void AdjustBrightness(float value);          // Handled by ImageViewerWidget, push to undo stack
    void AdjustContrast(float value);            // Handled by ImageViewerWidget, push to undo stack
    // AddTextOverlay, DrawAnnotation will be advanced features, consider for future iterations or simpler versions initially

    // Advanced Editing Capabilities (Basic filters as required in prompt)
    void ApplyFilter(FilterType filter);         // Handled by ImageViewerWidget, push to undo stack

    // Export and Interoperability
    void ExportToFormat(const QString& format);  // Use QImageWriter
    void PrintImage();                           // Use QPrintDialog and QPrinter
    void ExportToPDF(const QString& outputPath); // Use QPdfWriter
    void CopyToClipboard();                      // Use QApplication::clipboard()
    void PasteFromClipboard();                   // Use QApplication::clipboard()
    // EnableBatchConversion will be a later advanced feature.

    // Optional Advanced Features
    // LoadRAWImage (using external library like libraw), CompareImages, EnableScriptingInterface, SyncWithCloudProvider
    // These are beyond the initial scope for MVP and Final Delivery based on the prompt's core requirements.

    // Internal Architecture and Standards
    // UseGdkPixbufOrVIPS(): QT's QImage handles most common formats internally, for advanced operations, potentially integrate OpenCV or custom algorithms.
    // FollowXDGDirectorySpecs(): QStandardPaths for config and data.
    // ComplyWithFreeDesktopThumbnailSpec(): QT might have helper classes or requires manual implementation.

    // Performance and Stability
    void EnableAsyncImageLoading();              // Use QFuture/QtConcurrent or QThreadPool for background loading
    void OptimizeMemoryUsage();                  // Careful management of QImage instances
    void DetectCorruptedImages();                // Error handling during QImage loading

    // Public getters for components if needed by other parts of the UI
    QMainWindow* getMainWindow() const { return mainWindow; }
    ImageViewerWidget* getImageViewer() const { return imageViewer; }
    ImageGalleryWidget* getImageGallery() const { return imageGallery; }

private slots:
    void handleOpenAction();
    void handleOpenDirectoryAction();
    void handleSaveAsAction();
    void handleExportAction();
    void handlePrintAction();
    void handleCopyAction();
    void handlePasteAction();
    void handleUndoAction();
    void handleRedoAction();
    void handleZoomIn();
    void handleZoomOut();
    void handleRotateRight();
    void handleRotateLeft();
    void handleFlipHorizontal();
    void handleFlipVertical();
    void handleApplyGrayscaleFilter();
    void handleApplySepiaFilter();
    void handleApplyNegativeFilter();
    void handleToggleDarkMode(bool checked);
    void handleShowMetadata();
    void handleNextImage();
    void handlePreviousImage();
    void handleThumbnailClicked(const QString& imagePath);

private:
    QMainWindow* mainWindow;
    ImageViewerWidget* imageViewer;
    ImageGalleryWidget* imageGallery;
    ImageDataManager* imageDataManager;
    QUndoStack* undoStack;
    QSettings* settings; // For persistent settings like dark mode, default directory

    // Helper functions
    void createActions();
    void createMenus();
    void createToolbars();
    void connectSignalsAndSlots();
    void updateUIForImage(); // Call after image changes (metadata, zoom, etc.)

    // Internal state
    QString currentDirectory;
    QVector<QString> imageList; // List of image paths in current directory
    int currentImageIndex;
};

// --- ImageViewerWidget.h ---
class ImageViewerWidget : public QWidget {
    Q_OBJECT
public:
    ImageViewerWidget(QWidget* parent = nullptr);
    void setImage(const QImage& image);
    void setZoomFactor(qreal factor);
    void zoomIn();
    void zoomOut();
    void rotate(int angle);
    void flipHorizontal();
    void flipVertical();
    void applyGrayscale();
    void applySepia();
    void applyNegative();
    // For Undo/Redo: Need to store previous image states or operations
    QImage currentImage();

protected:
    void paintEvent(QPaintEvent* event) override;
    void wheelEvent(QWheelEvent* event) override; // For zoom
    // Mouse events for pan or future selection tools

private:
    QImage m_image;
    QImage m_displayedImage; // The image after transformations
    qreal m_zoomFactor;
    QPoint m_scrollOffset; // For panning large images
    qreal m_rotationAngle;
    bool m_flippedHorizontal;
    bool m_flippedVertical;

    void applyTransformations(); // Applies current rotation, flip, zoom to m_image into m_displayedImage
};

// --- ImageGalleryWidget.h ---
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

private:
    QString m_currentDirectory;
    // Potentially use QThreadPool for thumbnail generation in background
};

// --- ImageDataManager.h ---
class ImageDataManager : public QObject {
    Q_OBJECT
public:
    ImageDataManager(QObject* parent = nullptr);
    QImage loadImage(const QString& path);
    QImage applyGrayscale(const QImage& original);
    QImage applySepia(const QImage& original);
    QImage applyNegative(const QImage& original);
    // Other image manipulation functions

    // Metadata functions
    QMap<QString, QString> getImageMetadata(const QString& path);

signals:
    void imageLoaded(const QImage& image);
    void metadataReady(const QMap<QString, QString>& metadata);

private:
    // Potentially use QExifImageReader or an external library for more robust metadata
};

// --- Undo/Redo Commands (Example for rotation) ---
class RotateCommand : public QUndoCommand {
public:
    RotateCommand(ImageViewerWidget* viewer, int angle, QUndoCommand* parent = nullptr)
        : QUndoCommand(parent), m_viewer(viewer), m_angle(angle) {
        setText(QString("Rotate %1 degrees").arg(angle));
    }

    void undo() override {
        m_viewer->rotate(-m_angle); // Apply inverse rotation
    }

    void redo() override {
        m_viewer->rotate(m_angle);
    }

private:
    ImageViewerWidget* m_viewer;
    int m_angle;
};

// --- main.cpp ---
int main(int argc, char *argv[]) {
    ImageApplication app(argc, argv);
    app.InitializeGUI();
    app.getMainWindow()->show(); // Show the main window
    return app.exec();
}