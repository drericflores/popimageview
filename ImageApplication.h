#ifndef IMAGEAPPLICATION_H
#define IMAGEAPPLICATION_H

#include <QApplication>
#include <QMainWindow>
#include <QVector>
#include <QString>
#include <QObject>
#include <QSettings>
#include <QUndoStack>
#include <QDockWidget>
#include <QMenuBar>
#include <QPrintDialog>
#include <QPrinter>
#include <QClipboard>
#include <QMimeData>
#include <QKeySequence>
#include <QAction>
#include <QUndoCommand> // For Undo/Redo commands

// Forward declarations
class ImageViewerWidget;
class ImageGalleryWidget;
class ImageDataManager;

// Define basic enums
enum ZoomMode { FitToScreen, ActualSize, Custom };
enum SortMode { ByName, ByDate, BySize };

// Placeholder structs/enums
struct FilterCriteria {};
struct Rect {};
struct Point {};
enum RotationAngle { R0, R90, R180, R270 };
enum FilterType { Grayscale, Sepia, Negative, Normal };
struct Shape {}; // THIS IS LINE 35
enum SelectionType { Lasso, Rectangle }; // THIS IS LINE 36

// Base Undo Command for Image Operations
struct ImageViewerState {
    QImage image;
    qreal zoomFactor;
    qreal rotationAngle;
    bool flippedHorizontal;
    bool flippedVertical;
    QPoint scrollOffset;
};

// Base Undo Command for Image Operations
class ImageOperationCommand : public QUndoCommand {
public:
    ImageOperationCommand(ImageViewerWidget* viewer, const ImageViewerState& oldState, const ImageViewerState& newState, const QString& text);
    void undo() override;
    void redo() override;
private:
    ImageViewerWidget* m_viewer;
    ImageViewerState m_oldState;
    ImageViewerState m_newState;
};

// Specific Undo Commands (inheriting from ImageOperationCommand)
class ImageTransformCommand : public ImageOperationCommand {
public:
    ImageTransformCommand(ImageViewerWidget* viewer, const ImageViewerState& oldState, const ImageViewerState& newState, const QString& text);
};

class ImageFilterCommand : public ImageOperationCommand {
public:
    ImageFilterCommand(ImageViewerWidget* viewer, const ImageViewerState& oldState, const ImageViewerState& newState, FilterType filterType);
};


class ImageApplication : public QApplication {
    Q_OBJECT

public:
    ImageApplication(int& argc, char** argv);
    ~ImageApplication();

    // Initialization and System Integration
    void InitializeGUI();
    void LoadLocalization(const QString& locale);
    void RegisterFileTypes();

    // Core Image Viewing Features
    void OpenImageFile(const QString& path);
    void OpenImageDirectory(const QString& directory);
    void DisplayImage();
    void NavigateNext();
    void NavigatePrevious();
    void ToggleFullscreen();
    void ToggleZoom(ZoomMode mode);
    void ShowImageMetadata();

    // File and Image Management
    void RenameImage(const QString& newName);
    void DeleteImage();
    void MoveImageTo(const QString& destinationPath);
    void SortImages(SortMode mode);
    void FilterImages(FilterCriteria criteria);
    void EnableDragAndDrop();

    // UI/UX Configuration
    void EnableDarkMode(bool enable);
    void ConfigureShortcuts();
    void ApplyNativeTheme();
    void SetupMultiMonitorAwareness();

    // Editing Tools
    void CropImage(Rect region);
    void ResizeImage(int width, int height);
    void RotateImage(int angle);
    void AdjustBrightness(float value);
    void AdjustContrast(float value);
    void AddTextOverlay(const QString& text, Point position);
    void DrawAnnotation(Shape shape); // Usage here, expecting line ~114-115
    //                      ^^^^^
    // Advanced Editing Capabilities (Basic filters)
    void ApplyFilter(FilterType filter);
    void UseLayerSystem();
    void EnableSelectionTool(SelectionType type); // Usage here, expecting line ~119-120
    //                          ^^^^^^^^^^^^^
    void LoadPlugins(const QString& pluginPath);

    // Export and Interoperability
    void ExportToFormat(const QString& format);
    void PrintImage();
    void CopyToClipboard();
    void PasteFromClipboard();
    void EnableBatchConversion(const QVector<QString>& files);

    // Optional Advanced Features
    void LoadRAWImage(const QString& path);
    void CompareImages(const QString& pathA, const QString& pathB);
    void EnableUndoRedo();
    void EnableScriptingInterface();
    void SyncWithCloudProvider(const QString& providerName);

    // Internal Architecture and Standards (Placeholder)
    void UseGdkPixbufOrVIPS();
    void FollowXDGDirectorySpecs();
    void ComplyWithFreeDesktopThumbnailSpec();

    // Performance and Stability (Placeholder)
    void EnableAsyncImageLoading();
    void OptimizeMemoryUsage();
    void DetectCorruptedImages();
    void ApplySecuritySandboxing();

    // Public getters for components
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
    void handleApplyNormalFilter();
    void handleToggleDarkMode(bool checked);
    void handleShowMetadata();
    void handleNextImage();
    void handlePreviousImage();
    void handleThumbnailClicked(const QString& imagePath);
    void handleAboutAction();

private:
    QMainWindow* mainWindow;
    ImageViewerWidget* imageViewer;
    ImageGalleryWidget* imageGallery;
    ImageDataManager* imageDataManager;
    QUndoStack* undoStack;
    QSettings* settings;

    // QAction pointers as member variables
    QAction* openAct;
    QAction* openDirAct;
    QAction* exportAct;
    QAction* printAct;
    QAction* exitAct;
    QAction* undoAct;
    QAction* redoAct;
    QAction* copyAct;
    QAction* pasteAct;
    QAction* zoomInAct;
    QAction* zoomOutAct;
    QAction* fitToScreenAct;
    QAction* actualSizeAct;
    QAction* fullScreenAct;
    QAction* nextImageAct;
    QAction* prevImageAct;
    QAction* darkModeAct;
    QAction* rotateRightAct;
    QAction* rotateLeftAct;
    QAction* flipHorzAct;
    QAction* flipVertAct;
    QAction* grayscaleAct;
    QAction* sepiaAct;
    QAction* negativeAct;
    QAction* normalAct;
    QAction* metadataAct;
    QAction* aboutAct;

    // Internal state
    QString currentDirectory;
    QVector<QString> imageList;
    int currentImageIndex;

    // Helper functions
    void createActions();
    void createMenus();
    void createToolbars();
    void connectSignalsAndSlots();
    void updateUIForImage();

    // Helper to get current ImageViewerWidget state
    ImageViewerState getCurrentImageViewerState() const;
};

#endif // IMAGEAPPLICATION_H