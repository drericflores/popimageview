# popimageview
Another An Intuitive Image Viewer for Linux
PopPopImageView
Author: Dr. Eric O. Flores 
Version: 1.0.0 
Date: July 2025

PopImageView is a lightweight yet feature-rich image viewer application designed specifically for the Linux desktop environment. Built with C++ and the robust Qt toolkit, it offers a clean graphical user interface and a set of essential tools for Browse, viewing, and performing basic manipulations on your image collection.  Whether you're organizing your photo library, quickly previewing images, or applying simple edits, PopImageView provides a smooth and efficient experience without unnecessary complexities. This application is developed for educational purposes only.
Features
    • File Navigation: Easily open individual image files (JPG, PNG, BMP, TIFF, GIF) or entire directories.
    • Image Gallery: A convenient vertical sidebar displays thumbnails of all images in the current directory, allowing for quick selection.
    • Core Viewing Controls:
        ◦ Zoom In/Out (via menu, toolbar, and Ctrl + Mouse Wheel).
        ◦ Fit to Screen and Actual Size viewing modes.
        ◦ Full-screen toggle (F11).
        ◦ Panning large images with mouse drag.
    • Image Transformations:
        ◦ Rotate images 90° clockwise or counter-clockwise (Ctrl+R, Ctrl+L).
        ◦ Flip images horizontally (Ctrl+H) or vertically (Ctrl+V).
    • Basic Image Filters: Apply Grayscale, Sepia, or Negative effects.
    • Reset Filter: Easily revert images to their original, unfiltered state using the Normal filter option.
    • Undo/Redo History: Track and revert/re-apply image transformations and filter changes.
    • Clipboard Support: Copy the current image to the clipboard or paste images into the viewer.
    • Image Export: Save transformed images to various formats (BMP, TIFF, PNG).
    • Print Functionality: Send images directly to your configured physical printer.
    • Dark Mode: A toggleable dark theme for comfortable viewing.
    • Comprehensive Shortcuts: Intuitive keyboard shortcuts for all major operations.
    • About Dialog: Provides application information and lists technologies used.
Screenshots

Getting Started
These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.
Prerequisites
    • A Linux operating system.
    • A C++ compiler (e.g., G++).
    • Qt 5 Development Libraries: Ensure you have the necessary Qt 5 modules and their development headers installed.
        ◦ qtbase5-dev (core, gui, widgets)
        ◦ qtmultimedia5-dev (for image formats and metadata features)
        ◦ libqt5concurrent5-dev (for asynchronous operations)
        ◦ libqt5printsupport5-dev (for printing capabilities)
      Installation commands for common distributions:
        ◦ Debian/Ubuntu:
          sudo apt update
          sudo apt install qtbase5-dev qtmultimedia5-dev libqt5concurrent5-dev libqt5printsupport5-dev build-essential
        ◦ Fedora:
          sudo dnf install qt5-qtbase-devel qt5-qtmultimedia-devel qt5-qtconcurrent-devel qt5-qtprintsupport-devel gcc-c++
        ◦ Arch Linux:
          sudo pacman -S qt5-base qt5-multimedia qt5-concurrent qt5-printsupport
      (Note: Package names might vary slightly depending on your exact distribution version.)
Building the Application
    1. Clone the Repository:
       git clone https://github.com/YourGitHubUsername/PopImageView.git # Replace with your actual repo URL
       cd PopImageView
       (If you've downloaded the source as a zip, extract it and navigate into the extracted directory.)
    2. Generate Makefile: Use qmake to generate the build files.
       qmake
    3. Compile: Compile the source code using make.
       make
    4. Run the Application:
       ./PopImageView
Usage
For a detailed guide on how to use PopImageView, including navigating images, applying transformations, using filters, and understanding shortcuts, please refer to the User Manual (UserManual.md).
Contributing
Developing PopImageView has been a labor of love, crafted over many hours. While this application is provided completely free of charge and is open-source, your support is invaluable. If you find PopImageView useful and wish to support my continued efforts (perhaps 'buy me a coffee' to fuel future development!), any gesture is deeply appreciated. Thank you!
If you find a bug, have a feature request, or want to improve the codebase, please feel free to:
    1. Fork the repository.
    2. Create a new branch (git checkout -b feature/AmazingFeature).
    3. Commit your changes (git commit -m 'Add some AmazingFeature').
    4. Push to the branch (git push origin feature/AmazingFeature).
    5. Open a Pull Request.
License
This project is licensed under the GNU General Public License v3.0 - see the LICENSE file for details.
Donations
Developing PopImageView has been a labor of love, crafted over many hours. While this application is provided completely free of charge and is open-source, your support is invaluable. If you find PopImageView useful and wish to support my continued efforts (perhaps 'buy me a coffee' to fuel future development!), any gesture is deeply appreciated. Thank you!"
