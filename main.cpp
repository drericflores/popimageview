#include <QApplication>
#include <QMainWindow> // Ensure QMainWindow is fully declared here
#include <QDebug>      // For qWarning
#include "ImageApplication.h" // Include your main application class

int main(int argc, char *argv[]) {
    // It's good practice to ensure QApplication is constructed before any QObject that uses its features
    ImageApplication app(argc, argv);
    app.InitializeGUI(); // Call your initialization function
    
    // Get the main window pointer
    QMainWindow* mainWindow = app.getMainWindow();
    if (mainWindow) {
        mainWindow->show();
    } else {
        qWarning("Failed to create main window!");
    }
    return app.exec();
}