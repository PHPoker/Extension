#!/bin/bash
# Simple build script for PHP poker extension

# Check if we're in the right directory with src subdirectory
if [ ! -d "src" ]; then
    echo "Error: src directory not found. Make sure you run this script from the root project directory."
    exit 1
fi

# Check for required source files
if [ ! -f "src/phpoker.c" ]; then
    echo "Error: src/phpoker.c not found. This is the main source file."
    exit 1
fi

if [ ! -f "src/arrays.h" ]; then
    echo "Error: src/arrays.h not found. This file is required and should contain all lookup tables."
    exit 1
fi

# Remove any previous build directory
rm -rf build

# Create new build directory
mkdir -p build

# Copy source files to build directory
echo "Copying source files to build directory..."
cp src/*.c src/*.h src/config.m4 build/

# Remove any previous dist directory
rm -rf dist

# Create new dist directory
mkdir -p dist

# Change to build directory
cd build

# Clean up previous build artifacts
echo "Cleaning previous build..."
make clean > /dev/null 2>&1
phpize --clean > /dev/null 2>&1

# Initialize the build system
echo "Initializing build system..."
phpize

# Configure
echo "Configuring..."
./configure --enable-phpoker

# Build
echo "Building extension..."
make

# Install the extension
echo "Installing extension..."
sudo make install

# Also copy the built extension to dist directory for reference
echo "Copying built extension to dist directory..."
cp modules/phpoker.so ../dist/

# Return to the project root
cd ..

echo ""
echo "Build and installation complete!"
echo "----------------------------------------------------------------"
echo "The extension has been installed to your PHP extension directory."
echo "A copy is also available at: dist/phpoker.so"
echo ""
echo "Then enable it in your php.ini:"
echo "extension=phpoker.so"
echo ""
echo "You can test the extension by running:"
echo "php test.php"
echo "----------------------------------------------------------------"