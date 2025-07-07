#include "ImageApplication.h"
#include "ImageViewerWidget.h"
#include "ImageGalleryWidget.h"
#include "ImageDataManager.h"

// Explicit includes
#include <QMainWindow>
#include <QDebug>
#include <QFileDialog>
#include <QAction>
#include <QMenu>
#include <QToolBar>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QClipboard>
#include <QUndoStack>
#include <QMessageBox>
#include <QFile>
#include <QPalette>
#include <QApplication>
#include <QPainter>
#include <QMenuBar>
#include <QDockWidget>
#include <QKeySequence>
#include <QDir>
#include <QFileInfo>
#include <QPrintDialog>
#include <QPrinter>
#include <QStyle>
#include <QScreen>
#include <QCursor>
#include <QGuiApplication>
#include <QRectF>
#include <QTabWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QTextBrowser>
#include <QDialog>
#include <QPushButton>
#include <QHBoxLayout>

// --- NEW: Undo Command Implementations ---
ImageOperationCommand::ImageOperationCommand(ImageViewerWidget* viewer, const ImageViewerState& oldState, const ImageViewerState& newState, const QString& text)
    : QUndoCommand(text), m_viewer(viewer), m_oldState(oldState), m_newState(newState) {}

void ImageOperationCommand::undo() {
    if (m_viewer) {
        // Restore image data
        m_viewer->setImageOnly(m_oldState.image); // Use new setImageOnly
        // Restore viewer transformations
        m_viewer->setZoomFactor(m_oldState.zoomFactor);
        m_viewer->setRotationAngle(m_oldState.rotationAngle); // Use new setRotationAngle
        m_viewer->setFlipHorizontal(m_oldState.flippedHorizontal); // Use new setFlipHorizontal
        m_viewer->setFlipVertical(m_oldState.flippedVertical);   // Use new setFlipVertical
        m_viewer->setScrollOffset(m_oldState.scrollOffset);     // Use new setScrollOffset
        m_viewer->update(); // Request repaint
    }
}

void ImageOperationCommand::redo() {
    if (m_viewer) {
        // Apply image data
        m_viewer->setImageOnly(m_newState.image); // Use new setImageOnly
        // Apply viewer transformations
        m_viewer->setZoomFactor(m_newState.zoomFactor);
        m_viewer->setRotationAngle(m_newState.rotationAngle);
        m_viewer->setFlipHorizontal(m_newState.flippedHorizontal);
        m_viewer->setFlipVertical(m_newState.flippedVertical);
        m_viewer->setScrollOffset(m_newState.scrollOffset);
        m_viewer->update(); // Request repaint
    }
}

// Specific Undo Commands implementations
ImageTransformCommand::ImageTransformCommand(ImageViewerWidget* viewer, const ImageViewerState& oldState, const ImageViewerState& newState, const QString& text)
    : ImageOperationCommand(viewer, oldState, newState, text) {}

ImageFilterCommand::ImageFilterCommand(ImageViewerWidget* viewer, const ImageViewerState& oldState, const ImageViewerState& newState, FilterType filterType)
    : ImageOperationCommand(viewer, oldState, newState, "Apply Filter") {
    Q_UNUSED(filterType); // Use Q_UNUSED to suppress warning if filterType isn't used here
}
// --- END NEW: Undo Command Implementations ---


ImageApplication::ImageApplication(int& argc, char** argv) : QApplication(argc, argv) {
    mainWindow = nullptr;
    imageViewer = nullptr;
    imageGallery = nullptr;
    imageDataManager = nullptr;
    undoStack = nullptr;
    settings = nullptr;

    // Initialize all QAction pointers to nullptr
    openAct = nullptr; openDirAct = nullptr; exportAct = nullptr; printAct = nullptr; exitAct = nullptr;
    undoAct = nullptr; redoAct = nullptr; copyAct = nullptr; pasteAct = nullptr;
    zoomInAct = nullptr; zoomOutAct = nullptr; fitToScreenAct = nullptr; actualSizeAct = nullptr;
    fullScreenAct = nullptr; nextImageAct = nullptr; prevImageAct = nullptr; darkModeAct = nullptr;
    rotateRightAct = nullptr; rotateLeftAct = nullptr; flipHorzAct = nullptr; flipVertAct = nullptr;
    grayscaleAct = nullptr; sepiaAct = nullptr; negativeAct = nullptr; normalAct = nullptr;
    metadataAct = nullptr; aboutAct = nullptr;

    currentImageIndex = -1;

    QCoreApplication::setOrganizationName("DenebulaImaging");
    QCoreApplication::setApplicationName("ImageView");
}

ImageApplication::~ImageApplication() {
    delete undoStack;
    // Parented objects (imageGallery, imageViewer, imageDataManager, settings, mainWindow) are automatically deleted.
}

void ImageApplication::InitializeGUI() {
    qDebug() << "Initializing GUI...";

    mainWindow = new QMainWindow();
    mainWindow->setWindowTitle("PopImageView - Qt Image Viewer");
    mainWindow->setMinimumSize(800, 600);

    imageDataManager = new ImageDataManager(this);
    imageViewer = new ImageViewerWidget(mainWindow);
    imageGallery = new ImageGalleryWidget(mainWindow);
    undoStack = new QUndoStack(this);
    settings = new QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName(), this);

    mainWindow->setCentralWidget(imageViewer);

    QDockWidget* galleryDock = new QDockWidget("Gallery", mainWindow);
    galleryDock->setWidget(imageGallery);
    galleryDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mainWindow->addDockWidget(Qt::LeftDockWidgetArea, galleryDock);

    createActions();
    createMenus();
    createToolbars();
    connectSignalsAndSlots();

    bool darkModeEnabled = settings->value("darkMode", false).toBool();
    EnableDarkMode(darkModeEnabled);

    QString defaultPicturesPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    if (!defaultPicturesPath.isEmpty() && QDir(defaultPicturesPath).exists()) {
        OpenImageDirectory(defaultPicturesPath);
    } else {
        OpenImageDirectory(QDir::homePath());
    }

    updateUIForImage();
}

// --- Implementation of Public Interface Methods ---

void ImageApplication::LoadLocalization(const QString& locale) { Q_UNUSED(locale); }
void ImageApplication::RegisterFileTypes() {}

void ImageApplication::OpenImageFile(const QString& path) {
    qDebug() << "Opening image file:" << path;
    QImage img = imageDataManager->loadImage(path);
    if (!img.isNull()) {
        undoStack->clear(); // Clear undo history when opening a new image
        imageViewer->setImage(img); // Sets m_originalImage and resets transformations

        QFileInfo fileInfo(path);
        mainWindow->setWindowTitle("imageview - " + fileInfo.fileName());

        // Fix for Next/Prev: Ensure currentDirectory and imageList are correctly populated
        // and currentImageIndex is set when a specific file is opened.
        QString dirPath = fileInfo.absolutePath();
        if (dirPath != currentDirectory) {
            // If opening a file from a new directory, update the gallery and list
            OpenImageDirectory(dirPath); // This will reload gallery and set list
            // However, after this, need to ensure THIS image is selected
            imageGallery->selectImage(path);
            currentImageIndex = imageList.indexOf(path);
        } else {
            // If it's from the current directory, just select it in gallery
            imageGallery->selectImage(path);
            currentImageIndex = imageList.indexOf(path);
        }
        // If image not found in list (e.g., just opened a random single file),
        // then navigation will be limited to just this file if we implement it that way.
        // For simplicity, ensure it's in list for navigation:
        if (currentImageIndex == -1 && !path.isEmpty()) {
            imageList.clear();
            imageList.append(path);
            currentImageIndex = 0;
        }

    } else {
        QMessageBox::warning(mainWindow, "Error", "Could not open image file:\n" + path);
    }
}

void ImageApplication::OpenImageDirectory(const QString& directory) {
    qDebug() << "Opening image directory:" << directory;
    currentDirectory = directory;
    imageGallery->loadImagesFromDirectory(directory); // This populates the gallery and internal list

    QDir dir(directory);
    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp" << "*.gif" << "*.tiff" << "*.tif";
    imageList = QVector<QString>::fromList(dir.entryList(filters, QDir::Files | QDir::NoDotAndDotDot, QDir::Name));

    currentImageIndex = -1; // Reset index

    if (!imageList.isEmpty()) {
        OpenImageFile(QDir(currentDirectory).filePath(imageList.first()));
    } else {
        imageViewer->setImage(QImage());
        mainWindow->setWindowTitle("imageview - No images in " + directory);
        undoStack->clear(); // Clear undo history if no images are loaded
    }
}

void ImageApplication::DisplayImage() {
    qDebug() << "Displaying current image.";
    updateUIForImage();
}

void ImageApplication::NavigateNext() {
    if (imageList.isEmpty() || currentImageIndex == -1) {
        qDebug() << "No images to navigate or no current image.";
        return;
    }
    int nextIndex = (currentImageIndex + 1) % imageList.size();
    if (nextIndex == currentImageIndex) { // Only one image in list
        qDebug() << "Only one image in directory, no next image.";
        return;
    }
    OpenImageFile(QDir(currentDirectory).filePath(imageList.at(nextIndex)));
}

void ImageApplication::NavigatePrevious() {
    if (imageList.isEmpty() || currentImageIndex == -1) {
        qDebug() << "No images to navigate or no current image.";
        return;
    }
    int prevIndex = (currentImageIndex - 1 + imageList.size()) % imageList.size();
    if (prevIndex == currentImageIndex) { // Only one image in list
        qDebug() << "Only one image in directory, no previous image.";
        return;
    }
    OpenImageFile(QDir(currentDirectory).filePath(imageList.at(prevIndex)));
}

void ImageApplication::ToggleFullscreen() {
    if (mainWindow->isFullScreen()) {
        mainWindow->showNormal();
    } else {
        mainWindow->showFullScreen();
    }
}

void ImageApplication::ToggleZoom(ZoomMode mode) {
    if (!imageViewer || imageViewer->currentImage().isNull()) return;
    if (mode == ActualSize) {
        imageViewer->setZoomFactor(1.0);
    } else if (mode == FitToScreen) {
        imageViewer->fitImageToView();
    }
}

ImageViewerState ImageApplication::getCurrentImageViewerState() const {
    ImageViewerState state;
    if (imageViewer) {
        state.image = imageViewer->getOriginalImage(); // Get the base image
        state.zoomFactor = imageViewer->getZoomFactor();
        state.rotationAngle = imageViewer->getRotationAngle();
        state.flippedHorizontal = imageViewer->getFlipHorizontal();
        state.flippedVertical = imageViewer->getFlipVertical();
        state.scrollOffset = imageViewer->getScrollOffset(); // Assuming getScrollOffset
    }
    return state;
}

void ImageApplication::ShowImageMetadata() {
    qDebug() << "Attempting to show image metadata.";
    QString currentImagePath;
    if (!imageList.isEmpty() && currentImageIndex != -1 && currentImageIndex < imageList.size()) {
        currentImagePath = QDir(currentDirectory).filePath(imageList.at(currentImageIndex));
    } else if (imageViewer && !imageViewer->currentImage().isNull()) {
        // Fallback for pasted images or images opened individually not in a directory list
        // For these, we don't have a path, so basic file system metadata is unavailable.
        QMessageBox::information(mainWindow, "Image Metadata", "Metadata is available only for images opened from disk. Please save the image to disk first.");
        return;
    } else {
        QMessageBox::information(mainWindow, "Image Metadata", "No image open or selected to display metadata.");
        return;
    }


    if (!currentImagePath.isEmpty()) {
        QMap<QString, QString> metadata = imageDataManager->getImageMetadata(currentImagePath);
        QString metaInfo = "No metadata found.";
        if (!metadata.isEmpty()) {
            metaInfo.clear();
            for (auto it = metadata.begin(); it != metadata.end(); ++it) {
                metaInfo += "<b>" + it.key() + ":</b> " + it.value() + "<br>";
            }
        }
        QMessageBox msgBox(mainWindow);
        msgBox.setWindowTitle("Image Metadata - " + QFileInfo(currentImagePath).fileName());
        msgBox.setTextFormat(Qt::RichText);
        msgBox.setText(metaInfo);
        msgBox.exec();
    } else {
        QMessageBox::information(mainWindow, "Image Metadata", "No image open or selected to display metadata.");
    }
}

void ImageApplication::RenameImage(const QString& newName) { Q_UNUSED(newName); }
void ImageApplication::DeleteImage() {}
void ImageApplication::MoveImageTo(const QString& destinationPath) { Q_UNUSED(destinationPath); }
void ImageApplication::SortImages(SortMode mode) { Q_UNUSED(mode); }
void ImageApplication::FilterImages(FilterCriteria criteria) { Q_UNUSED(criteria); }
void ImageApplication::EnableDragAndDrop() {}

void ImageApplication::EnableDarkMode(bool enable) {
    if (enable) {
        QFile file(":/styles/dark.qss");
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            QString styleSheet = QLatin1String(file.readAll());
            qApp->setStyleSheet(styleSheet);
            qDebug() << "Dark mode enabled (from QSS).";
        } else {
            qWarning() << "Dark stylesheet not found (:/styles/dark.qss). Applying default dark palette.";
            QPalette darkPalette;
            darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
            darkPalette.setColor(QPalette::WindowText, Qt::white);
            darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
            darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
            darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
            darkPalette.setColor(QPalette::ToolTipText, Qt::white);
            darkPalette.setColor(QPalette::Text, Qt::white);
            darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
            darkPalette.setColor(QPalette::ButtonText, Qt::white);
            darkPalette.setColor(QPalette::BrightText, Qt::red);
            darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
            darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
            darkPalette.setColor(QPalette::HighlightedText, Qt::black);
            qApp->setPalette(darkPalette);
        }
    } else {
        qApp->setStyleSheet("");
        qApp->setPalette(qApp->style()->standardPalette());
        qDebug() << "Dark mode disabled.";
    }
    settings->setValue("darkMode", enable);
}

void ImageApplication::ConfigureShortcuts() {}
void ImageApplication::ApplyNativeTheme() {}
void ImageApplication::SetupMultiMonitorAwareness() {}

void ImageApplication::CropImage(Rect region) { Q_UNUSED(region); }
void ImageApplication::ResizeImage(int width, int height) { Q_UNUSED(width); Q_UNUSED(height); }

void ImageApplication::RotateImage(int angle) {
    if (!imageViewer || imageViewer->currentImage().isNull()) return;
    ImageViewerState oldState = getCurrentImageViewerState(); // Get state before operation
    imageViewer->rotate(angle);
    ImageViewerState newState = getCurrentImageViewerState(); // Get state after operation
    undoStack->push(new ImageTransformCommand(imageViewer, oldState, newState, "Rotate"));
}

void ImageApplication::AdjustBrightness(float value) { Q_UNUSED(value); }
void ImageApplication::AdjustContrast(float value) { Q_UNUSED(value); }
void ImageApplication::AddTextOverlay(const QString& text, Point position) { Q_UNUSED(text); Q_UNUSED(position); }
void ImageApplication::DrawAnnotation(Shape shape) { Q_UNUSED(shape); }

void ImageApplication::ApplyFilter(FilterType filter) {
    if (!imageViewer || imageViewer->currentImage().isNull()) return;

    ImageViewerState oldState = getCurrentImageViewerState(); // Get state before filter

    if (filter == Grayscale) {
        imageViewer->applyGrayscale();
    } else if (filter == Sepia) {
        imageViewer->applySepia();
    } else if (filter == Negative) {
        imageViewer->applyNegative();
    } else if (filter == Normal) {
        // "Normal" means reset image data and all transformations
        imageViewer->setImage(imageViewer->getOriginalImageSource()); // Reset to original source image (unfiltered, un-transformed)
        // This action clears viewer's transformations by design, so no need to explicitly set them to 0 here.
        undoStack->clear(); // Clear undo history for "Normal" as it's a full reset point
        return; // Don't push to undo stack as it's a reset
    }

    ImageViewerState newState = getCurrentImageViewerState(); // Get state after filter
    undoStack->push(new ImageFilterCommand(imageViewer, oldState, newState, filter));
}

void ImageApplication::UseLayerSystem() {}
void ImageApplication::EnableSelectionTool(SelectionType type) { Q_UNUSED(type); }
void ImageApplication::LoadPlugins(const QString& pluginPath) { Q_UNUSED(pluginPath); }

void ImageApplication::ExportToFormat(const QString& format) {
    if (!imageViewer || imageViewer->currentImage().isNull()) {
        QMessageBox::warning(mainWindow, "Export Error", "No image to export.");
        return;
    }

    QString filter;
    if (format == "BMP") filter = "Bitmap (*.bmp)";
    else if (format == "TIFF") filter = "TIFF (*.tif *.tiff)";
    else if (format == "PNG") filter = "PNG (*.png)";
    else filter = "All Files (*)";

    QString filePath = QFileDialog::getSaveFileName(mainWindow, "Export Image",
                                                    QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
                                                    filter);
    if (filePath.isEmpty()) return;

    if (imageViewer->currentImage().save(filePath, qPrintable(format.toUpper()))) {
        QMessageBox::information(mainWindow, "Export Successful", "Image exported successfully.");
    } else {
        QMessageBox::warning(mainWindow, "Export Error", "Failed to export image to " + format + " format.");
    }
}

void ImageApplication::PrintImage() {
    if (!imageViewer || imageViewer->currentImage().isNull()) {
        QMessageBox::warning(mainWindow, "Print Error", "No image to print.");
        return;
    }

    QPrinter printer;
    QPrintDialog printDialog(&printer, mainWindow);
    if (printDialog.exec() == QDialog::Accepted) {
        QPainter painter(&printer);
        QImage img = imageViewer->currentImage();
        QRect rect = painter.viewport();
        QSize size = img.size();
        size.scale(rect.size(), Qt::KeepAspectRatio);
        painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
        painter.setWindow(img.rect());
        painter.drawImage(0, 0, img);
        QMessageBox::information(mainWindow, "Print", "Image sent to printer.");
    }
}

void ImageApplication::CopyToClipboard() {
    if (imageViewer && !imageViewer->currentImage().isNull()) {
        QApplication::clipboard()->setImage(imageViewer->currentImage());
        qDebug() << "Image copied to clipboard.";
    } else {
        qDebug() << "No image to copy to clipboard.";
    }
}

void ImageApplication::PasteFromClipboard() {
    QClipboard* clipboard = QApplication::clipboard();
    if (clipboard->mimeData()->hasImage()) {
        QImage pastedImage = clipboard->image();
        if (!pastedImage.isNull()) {
            qDebug() << "Image pasted from clipboard.";
            imageViewer->setImage(pastedImage);
            mainWindow->setWindowTitle("imageview - (Pasted Image)");
            imageGallery->clear();
            imageList.clear();
            currentImageIndex = -1;
            undoStack->clear(); // Clear undo history for pasted images
        }
    } else {
        qDebug() << "No image in clipboard.";
        QMessageBox::information(mainWindow, "Paste", "No image found in clipboard.");
    }
}

void ImageApplication::EnableBatchConversion(const QVector<QString>& files) { Q_UNUSED(files); }

void ImageApplication::LoadRAWImage(const QString& path) { Q_UNUSED(path); }
void ImageApplication::CompareImages(const QString& pathA, const QString& pathB) { Q_UNUSED(pathA); Q_UNUSED(pathB); }

void ImageApplication::EnableUndoRedo() {}
void ImageApplication::EnableScriptingInterface() {}
void ImageApplication::SyncWithCloudProvider(const QString& providerName) { Q_UNUSED(providerName); }

void ImageApplication::UseGdkPixbufOrVIPS() {}
void ImageApplication::FollowXDGDirectorySpecs() {}
void ImageApplication::ComplyWithFreeDesktopThumbnailSpec() {}

void ImageApplication::EnableAsyncImageLoading() {}
void ImageApplication::OptimizeMemoryUsage() {}
void ImageApplication::DetectCorruptedImages() {}
void ImageApplication::ApplySecuritySandboxing() {}


// --- Private Helper Functions ---
void ImageApplication::createActions() {
    // File Menu
    openAct = new QAction("&Open...", mainWindow);
    openAct->setShortcut(QKeySequence::Open);
    connect(openAct, &QAction::triggered, this, &ImageApplication::handleOpenAction);

    openDirAct = new QAction("Open &Directory...", mainWindow);
    openDirAct->setShortcut(QKeySequence("Ctrl+Shift+O"));
    connect(openDirAct, &QAction::triggered, this, &ImageApplication::handleOpenDirectoryAction);

    exportAct = new QAction("&Export As...", mainWindow);
    exportAct->setShortcut(QKeySequence("Ctrl+E"));
    connect(exportAct, &QAction::triggered, this, &ImageApplication::handleExportAction);

    printAct = new QAction("&Print...", mainWindow);
    printAct->setShortcut(QKeySequence::Print);
    connect(printAct, &QAction::triggered, this, &ImageApplication::handlePrintAction);

    exitAct = new QAction("E&xit", mainWindow);
    exitAct->setShortcut(QKeySequence::Quit);
    connect(exitAct, &QAction::triggered, mainWindow, &QMainWindow::close);

    // Edit Menu
    undoAct = undoStack->createUndoAction(this, "&Undo");
    undoAct->setShortcut(QKeySequence::Undo);

    redoAct = undoStack->createRedoAction(this, "&Redo");
    redoAct->setShortcut(QKeySequence::Redo);

    copyAct = new QAction("&Copy", mainWindow);
    copyAct->setShortcut(QKeySequence::Copy);
    connect(copyAct, &QAction::triggered, this, &ImageApplication::handleCopyAction);

    pasteAct = new QAction("&Paste", mainWindow);
    pasteAct->setShortcut(QKeySequence::Paste);
    connect(pasteAct, &QAction::triggered, this, &ImageApplication::handlePasteAction);

    // View Menu
    zoomInAct = new QAction("Zoom &In", mainWindow);
    zoomInAct->setShortcut(QKeySequence::ZoomIn);
    connect(zoomInAct, &QAction::triggered, this, &ImageApplication::handleZoomIn);

    zoomOutAct = new QAction("Zoom &Out", mainWindow);
    zoomOutAct->setShortcut(QKeySequence::ZoomOut);
    connect(zoomOutAct, &QAction::triggered, this, &ImageApplication::handleZoomOut);

    fitToScreenAct = new QAction("Fit to Screen", mainWindow);
    fitToScreenAct->setShortcut(QKeySequence("Ctrl+F"));
    connect(fitToScreenAct, &QAction::triggered, [this](){ ToggleZoom(FitToScreen); });

    actualSizeAct = new QAction("Actual Size", mainWindow);
    actualSizeAct->setShortcut(QKeySequence("Ctrl+0"));
    connect(actualSizeAct, &QAction::triggered, [this](){ ToggleZoom(ActualSize); });

    fullScreenAct = new QAction("&Fullscreen", mainWindow);
    fullScreenAct->setShortcut(QKeySequence::FullScreen);
    fullScreenAct->setCheckable(true);
    connect(fullScreenAct, &QAction::triggered, this, &ImageApplication::ToggleFullscreen);

    nextImageAct = new QAction("&Next Image", mainWindow);
    nextImageAct->setShortcut(Qt::ControlModifier + Qt::Key_Right);
    connect(nextImageAct, &QAction::triggered, this, &ImageApplication::handleNextImage);

    prevImageAct = new QAction("&Previous Image", mainWindow);
    prevImageAct->setShortcut(Qt::ControlModifier + Qt::Key_Left);
    connect(prevImageAct, &QAction::triggered, this, &ImageApplication::handlePreviousImage);

    darkModeAct = new QAction("&Dark Mode", mainWindow);
    darkModeAct->setCheckable(true);
    darkModeAct->setChecked(settings->value("darkMode", false).toBool());
    connect(darkModeAct, &QAction::toggled, this, &ImageApplication::handleToggleDarkMode);

    // Image Menu
    rotateRightAct = new QAction("Rotate &Right (90°)", mainWindow);
    rotateRightAct->setShortcut(QKeySequence("Ctrl+R"));
    connect(rotateRightAct, &QAction::triggered, this, &ImageApplication::handleRotateRight);

    rotateLeftAct = new QAction("Rotate &Left (-90°)", mainWindow);
    rotateLeftAct->setShortcut(QKeySequence("Ctrl+L"));
    connect(rotateLeftAct, &QAction::triggered, this, &ImageApplication::handleRotateLeft);

    flipHorzAct = new QAction("Flip &Horizontal", mainWindow);
    flipHorzAct->setShortcut(QKeySequence("Ctrl+H"));
    connect(flipHorzAct, &QAction::triggered, this, &ImageApplication::handleFlipHorizontal);

    flipVertAct = new QAction("Flip &Vertical", mainWindow);
    flipVertAct->setShortcut(QKeySequence("Ctrl+V"));
    connect(flipVertAct, &QAction::triggered, this, &ImageApplication::handleFlipVertical);

    grayscaleAct = new QAction("Grayscale", mainWindow);
    connect(grayscaleAct, &QAction::triggered, this, &ImageApplication::handleApplyGrayscaleFilter);

    sepiaAct = new QAction("Sepia", mainWindow);
    connect(sepiaAct, &QAction::triggered, this, &ImageApplication::handleApplySepiaFilter);

    negativeAct = new QAction("Negative", mainWindow);
    connect(negativeAct, &QAction::triggered, this, &ImageApplication::handleApplyNegativeFilter);

    normalAct = new QAction("&Normal", mainWindow);
    connect(normalAct, &QAction::triggered, this, &ImageApplication::handleApplyNormalFilter);

    metadataAct = new QAction("&Metadata", mainWindow);
    metadataAct->setShortcut(QKeySequence("F10"));
    connect(metadataAct, &QAction::triggered, this, &ImageApplication::handleShowMetadata);

    aboutAct = new QAction("&About...", mainWindow);
    connect(aboutAct, &QAction::triggered, this, &ImageApplication::handleAboutAction);


    // Add actions to main window (for shortcut propagation and parentage)
    mainWindow->addAction(openAct);
    mainWindow->addAction(openDirAct);
    mainWindow->addAction(exportAct);
    mainWindow->addAction(printAct);
    mainWindow->addAction(exitAct);

    mainWindow->addAction(undoAct);
    mainWindow->addAction(redoAct);
    mainWindow->addAction(copyAct);
    mainWindow->addAction(pasteAct);

    mainWindow->addAction(zoomInAct);
    mainWindow->addAction(zoomOutAct);
    mainWindow->addAction(fitToScreenAct);
    mainWindow->addAction(actualSizeAct);
    mainWindow->addAction(fullScreenAct);
    mainWindow->addAction(nextImageAct);
    mainWindow->addAction(prevImageAct);
    mainWindow->addAction(darkModeAct);

    mainWindow->addAction(rotateRightAct);
    mainWindow->addAction(rotateLeftAct);
    mainWindow->addAction(flipHorzAct);
    mainWindow->addAction(flipVertAct);
    mainWindow->addAction(grayscaleAct);
    mainWindow->addAction(sepiaAct);
    mainWindow->addAction(negativeAct);
    mainWindow->addAction(normalAct);
    mainWindow->addAction(metadataAct);
    mainWindow->addAction(aboutAct);
}

void ImageApplication::createMenus() {
    QMenu* fileMenu = mainWindow->menuBar()->addMenu("&File");
    fileMenu->addAction(openAct);
    fileMenu->addAction(openDirAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exportAct);
    fileMenu->addAction(printAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    QMenu* editMenu = mainWindow->menuBar()->addMenu("&Edit");
    editMenu->addAction(undoAct);
    editMenu->addAction(redoAct);
    editMenu->addSeparator();
    editMenu->addAction(copyAct);
    editMenu->addAction(pasteAct);

    QMenu* viewMenu = mainWindow->menuBar()->addMenu("&View");
    viewMenu->addAction(zoomInAct);
    viewMenu->addAction(zoomOutAct);
    viewMenu->addAction(fitToScreenAct);
    viewMenu->addAction(actualSizeAct);
    viewMenu->addSeparator();
    viewMenu->addAction(fullScreenAct);
    viewMenu->addSeparator();
    viewMenu->addAction(nextImageAct);
    viewMenu->addAction(prevImageAct);
    viewMenu->addSeparator();
    viewMenu->addAction(darkModeAct);

    QMenu* imageMenu = mainWindow->menuBar()->addMenu("&Image");
    imageMenu->addAction(rotateRightAct);
    imageMenu->addAction(rotateLeftAct);
    imageMenu->addAction(flipHorzAct);
    imageMenu->addAction(flipVertAct);
    imageMenu->addSeparator();
    QMenu* filtersMenu = imageMenu->addMenu("&Filters");
    filtersMenu->addAction(grayscaleAct);
    filtersMenu->addAction(sepiaAct);
    filtersMenu->addAction(negativeAct);
    filtersMenu->addAction(normalAct);
    imageMenu->addSeparator();
    imageMenu->addAction(metadataAct);

    QMenu* helpMenu = mainWindow->menuBar()->addMenu("&Help");
    helpMenu->addAction(aboutAct);
}

void ImageApplication::createToolbars() {
    QToolBar* fileToolbar = mainWindow->addToolBar("File");
    fileToolbar->addAction(openAct);
    fileToolbar->addAction(openDirAct);
    fileToolbar->addAction(exportAct);
    // printAct can also be added to toolbar if desired: fileToolbar->addAction(printAct);

    QToolBar* navigationToolbar = mainWindow->addToolBar("Navigation");
    navigationToolbar->addAction(prevImageAct);
    navigationToolbar->addAction(nextImageAct);
    navigationToolbar->addSeparator();
    navigationToolbar->addAction(zoomInAct);
    navigationToolbar->addAction(zoomOutAct);
    navigationToolbar->addAction(fitToScreenAct);
    navigationToolbar->addAction(actualSizeAct);
    navigationToolbar->addSeparator();
    navigationToolbar->addAction(fullScreenAct);

    QToolBar* editToolbar = mainWindow->addToolBar("Edit");
    editToolbar->addAction(undoAct);
    editToolbar->addAction(redoAct);
    editToolbar->addSeparator();
    editToolbar->addAction(copyAct);
    editToolbar->addAction(pasteAct);
    editToolbar->addSeparator();
    editToolbar->addAction(rotateRightAct);
    editToolbar->addAction(rotateLeftAct);
    editToolbar->addAction(flipHorzAct);
    editToolbar->addAction(flipVertAct);
}

void ImageApplication::connectSignalsAndSlots() {
    connect(imageDataManager, &ImageDataManager::imageLoaded, imageViewer, &ImageViewerWidget::setImage);
    connect(imageGallery, &ImageGalleryWidget::imageSelected, this, &ImageApplication::handleThumbnailClicked);

    connect(undoStack, &QUndoStack::canUndoChanged, undoAct, &QAction::setEnabled);
    connect(undoStack, &QUndoStack::canRedoChanged, redoAct, &QAction::setEnabled);
    undoAct->setEnabled(false);
    redoAct->setEnabled(false);
}

void ImageApplication::updateUIForImage() {
    if (imageViewer && !imageViewer->currentImage().isNull()) {
        QFileInfo fileInfo(imageGallery->currentImagePath());
        if (!fileInfo.fileName().isEmpty()) {
            mainWindow->setWindowTitle("imageview - " + fileInfo.fileName());
        } else {
            mainWindow->setWindowTitle("imageview");
        }
    } else {
        mainWindow->setWindowTitle("imageview");
    }
}

// --- Slot Implementations ---
void ImageApplication::handleOpenAction() {
    QString filePath = QFileDialog::getOpenFileName(mainWindow, "Open Image",
                                                    QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
                                                    "Images (*.png *.jpg *.jpeg *.bmp *.gif *.tiff *.tif);;All Files (*)");
    if (!filePath.isEmpty()) {
        OpenImageFile(filePath);
    }
}

void ImageApplication::handleOpenDirectoryAction() {
    QString directory = QFileDialog::getExistingDirectory(mainWindow, "Open Directory",
                                                          QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    if (!directory.isEmpty()) {
        OpenImageDirectory(directory);
    }
}

void ImageApplication::handleSaveAsAction() {
    ExportToFormat("PNG");
}

void ImageApplication::handleExportAction() {
    QMenu formatMenu(mainWindow);
    QAction* bmpActLocal = formatMenu.addAction("BMP");
    QAction* tiffActLocal = formatMenu.addAction("TIFF");
    QAction* pngActLocal = formatMenu.addAction("PNG");

    QAction* selectedAction = formatMenu.exec(QCursor::pos());

    if (selectedAction == bmpActLocal) ExportToFormat("BMP");
    else if (selectedAction == tiffActLocal) ExportToFormat("TIFF");
    else if (selectedAction == pngActLocal) ExportToFormat("PNG");
}

void ImageApplication::handlePrintAction() {
    PrintImage();
}

void ImageApplication::handleCopyAction() {
    CopyToClipboard();
}

void ImageApplication::handlePasteAction() {
    PasteFromClipboard();
}

void ImageApplication::handleUndoAction() {
    if (undoStack->canUndo()) {
        undoStack->undo();
        qDebug() << "Undo performed.";
    }
}

void ImageApplication::handleRedoAction() {
    if (undoStack->canRedo()) {
        undoStack->redo();
        qDebug() << "Redo performed.";
    }
}

void ImageApplication::handleZoomIn() {
    if (!imageViewer || imageViewer->getOriginalImage().isNull()) return; // Added check for image presence
    // No undo command for zoom as it's a view transform, not image data change.
    // If zoom needs to be undoable, its own command would be needed.
    imageViewer->zoomIn();
}

void ImageApplication::handleZoomOut() {
    if (!imageViewer || imageViewer->getOriginalImage().isNull()) return; // Added check for image presence
    // No undo command for zoom.
    imageViewer->zoomOut();
}

void ImageApplication::handleRotateRight() {
    if (!imageViewer || imageViewer->getOriginalImage().isNull()) return; // Added check for image presence
    ImageViewerState oldState = getCurrentImageViewerState();
    imageViewer->rotate(90);
    ImageViewerState newState = getCurrentImageViewerState();
    undoStack->push(new ImageTransformCommand(imageViewer, oldState, newState, "Rotate Right (90°)"));
}

void ImageApplication::handleRotateLeft() {
    if (!imageViewer || imageViewer->getOriginalImage().isNull()) return; // Added check for image presence
    ImageViewerState oldState = getCurrentImageViewerState();
    imageViewer->rotate(-90);
    ImageViewerState newState = getCurrentImageViewerState();
    undoStack->push(new ImageTransformCommand(imageViewer, oldState, newState, "Rotate Left (-90°)"));
}

void ImageApplication::handleFlipHorizontal() {
    if (!imageViewer || imageViewer->getOriginalImage().isNull()) return; // Added check for image presence
    ImageViewerState oldState = getCurrentImageViewerState();
    imageViewer->flipHorizontal();
    ImageViewerState newState = getCurrentImageViewerState();
    undoStack->push(new ImageTransformCommand(imageViewer, oldState, newState, "Flip Horizontal"));
}

void ImageApplication::handleFlipVertical() {
    if (!imageViewer || imageViewer->getOriginalImage().isNull()) return; // Added check for image presence
    ImageViewerState oldState = getCurrentImageViewerState();
    imageViewer->flipVertical();
    ImageViewerState newState = getCurrentImageViewerState();
    undoStack->push(new ImageTransformCommand(imageViewer, oldState, newState, "Flip Vertical"));
}

void ImageApplication::handleApplyGrayscaleFilter() {
    ApplyFilter(Grayscale);
}

void ImageApplication::handleApplySepiaFilter() {
    ApplyFilter(Sepia);
}

void ImageApplication::handleApplyNegativeFilter() {
    ApplyFilter(Negative);
}

void ImageApplication::handleApplyNormalFilter() {
    ApplyFilter(Normal);
}

void ImageApplication::handleToggleDarkMode(bool checked) {
    EnableDarkMode(checked);
}

void ImageApplication::handleShowMetadata() {
    ShowImageMetadata();
}

void ImageApplication::handleNextImage() {
    NavigateNext();
}

void ImageApplication::handlePreviousImage() {
    NavigatePrevious();
}

void ImageApplication::handleThumbnailClicked(const QString& imagePath) {
    OpenImageFile(imagePath);
}

void ImageApplication::handleAboutAction() {
    QDialog* aboutDialog = new QDialog(mainWindow);
    aboutDialog->setWindowTitle("About imageview");
    aboutDialog->setMinimumSize(400, 300);
    QVBoxLayout* dialogLayout = new QVBoxLayout(aboutDialog);

    QTabWidget* tabWidget = new QTabWidget(aboutDialog);

    // --- Tab 1: Program Info ---
    QWidget* infoTab = new QWidget(tabWidget);
    QVBoxLayout* infoLayout = new QVBoxLayout(infoTab);

    QLabel* programLabel = new QLabel("<b>Program:</b> imageview", infoTab);
    QLabel* authorLabel = new QLabel("<b>By:</b> Dr. Eric O. Flores", infoTab);
    QLabel* versionLabel = new QLabel("<b>Version:</b> 1.0.0", infoTab);
    QLabel* dateLabel = new QLabel("<b>Date:</b> July 2025", infoTab);

    infoLayout->addWidget(programLabel);
    infoLayout->addWidget(authorLabel);
    infoLayout->addWidget(versionLabel);
    infoLayout->addWidget(dateLabel);
    infoLayout->addStretch(1);

    infoTab->setLayout(infoLayout);
    tabWidget->addTab(infoTab, "Info");

    // --- Tab 2: Technologies Used ---
    QWidget* techTab = new QWidget(tabWidget);
    QVBoxLayout* techLayout = new QVBoxLayout(techTab);

    QTextBrowser* techBrowser = new QTextBrowser(techTab);
    techBrowser->setReadOnly(true);
    techBrowser->setOpenExternalLinks(true);
    techBrowser->setHtml(R"(
        <h3>Core Technologies:</h3>
        <ul>
            <li><b>C++:</b> The primary programming language.</li>
            <li><b>Qt Toolkit:</b> For GUI development, cross-platform compatibility, and core functionalities (Widgets, Core, GUI, Multimedia, Concurrent, PrintSupport).</li>
        </ul>
        <h3>Qt Modules Used:</h3>
        <ul>
            <li><b>Qt Core:</b> Fundamental non-GUI classes (event loop, signals & slots, I/O, threading).</li>
            <li><b>Qt GUI:</b> Base classes for graphical user interfaces (QImage, QPixmap, QPainter).</li>
            <li><b>Qt Widgets:</b> Standard UI components (QMainWindow, QPushButton, QLabel, QListWidget).</li>
            <li><b>Qt Multimedia:</b> For image reading/writing capabilities (QImageReader, QImageWriter, potentially QExifImageReader).</li>
            <li><b>Qt Concurrent:</b> For asynchronous operations like thumbnail generation (QtConcurrent::run).</li>
            <li><b>Qt PrintSupport:</b> For printing images to physical printers (QPrinter, QPrintDialog).</li>
        </ul>
        <h3>Other Libraries/Standards:</h3>
        <ul>
            <li><b>Standard C++ Library:</b> For general programming tasks.</li>
            <li><b>XDG Base Directory Specification:</b> For configuration and cache management (via QStandardPaths).</li>
        </ul>
        <br>
        <p>This application leverages the power and flexibility of the Qt framework to provide a robust and user-friendly image viewing experience on Linux.</p>
    )");

    techLayout->addWidget(techBrowser);
    techTab->setLayout(techLayout);
    tabWidget->addTab(techTab, "Technologies");

    dialogLayout->addWidget(tabWidget);

    QPushButton* okButton = new QPushButton("OK", aboutDialog);
    connect(okButton, &QPushButton::clicked, aboutDialog, &QDialog::accept);
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(okButton);
    dialogLayout->addLayout(buttonLayout);

    aboutDialog->setLayout(dialogLayout);

    aboutDialog->exec();
}