#!/bin/bash
# Build Arch Linux package in Docker container
# This script runs inside the Arch Linux Docker container

set -e

# Check if VERSION is set
if [ -z "${VERSION}" ]; then
    echo "Error: VERSION environment variable is not set"
    exit 1
fi

echo "Building Arch package for version ${VERSION}"

# Update system and install dependencies
echo "Updating system and installing dependencies..."
pacman -Syu --noconfirm
pacman -S --noconfirm base-devel cmake fftw libsndfile libao libsamplerate gtk2 ncurses python pybind11 python-numpy python-setuptools

# Create build user (makepkg doesn't run as root)
echo "Creating build user..."
useradd -m builder
echo "builder ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

# Copy source to builder's home
echo "Copying source files..."
cp -r /src /home/builder/waon
chown -R builder:builder /home/builder/waon

# Update PKGBUILD with version
echo "Updating PKGBUILD version to ${VERSION}..."
cd /home/builder/waon
sed -i "s/pkgver=.*/pkgver=${VERSION}/" packaging/arch/PKGBUILD
# Remove source and sha256sums since we're building from local files
sed -i 's/source=.*/source=()/' packaging/arch/PKGBUILD
sed -i 's/sha256sums=.*/sha256sums=()/' packaging/arch/PKGBUILD
# Fix all source directory paths to use local directory instead of extracted tarball
sed -i 's|$srcdir/WaoN-$pkgver|/home/builder/waon|g' packaging/arch/PKGBUILD

# Replace version placeholder in setup.py
echo "Replacing version placeholder in setup.py..."
sed -i "s/@PROJECT_VERSION@/${VERSION}/" python/setup.py

# Copy README.md to python directory to avoid access issues
echo "Copying README.md to avoid path access issues..."
cp README.md python/

# Build package as builder user
echo "Building package..."
su builder -c "cd packaging/arch && makepkg -s --noconfirm"

# Copy result to output directory
echo "Copying built package to output..."
cp packaging/arch/*.pkg.tar.zst /output/

echo "Arch package build complete!"