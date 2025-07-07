# imageview.pro

# Project name
TARGET = imageview

# Application type: app for GUI applications
TEMPLATE = app

# QT modules required for the application
# core: Core non-GUI classes
# gui: GUI base classes
# widgets: Standard Qt widgets
# multimedia: For QImageReader/Writer, potentially QExifImageReader for metadata
# printsupport: For general printing (QPrinter, QPrintDialog)
# NEW: concurrent for QtConcurrent (asynchronous operations like thumbnail generation)
QT += core gui widgets multimedia printsupport concurrent

# Standard C++ version (recommend C++17 or later for modern features)
CONFIG += c++17

# Add debug and release builds
CONFIG += debug_and_release

# Input files (headers)
HEADERS += \
    ImageApplication.h \
    ImageViewerWidget.h \
    ImageGalleryWidget.h \
    ImageDataManager.h

# Input files (sources)
SOURCES += \
    main.cpp \
    ImageApplication.cpp \
    ImageViewerWidget.cpp \
    ImageGalleryWidget.cpp \
    ImageDataManager.cpp

# Optional: Add resources like icons, stylesheets if you plan to use them.
# For example, if you have a file called 'app_resources.qrc':
# RESOURCES += app_resources.qrc

# Optional: Set the destination directory for the executable
# DESTDIR = bin

# Optional: Add include paths for external libraries (e.g., libraw for RAW images)
# INCLUDEPATH += /path/to/libraw/include
# LIBS += -L/path/to/libraw/lib -llibraw