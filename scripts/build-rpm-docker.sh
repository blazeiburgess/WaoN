#!/bin/bash
# Build RPM package in Docker container
# This script runs inside a Fedora Docker container

set -e

# Check if VERSION is set
if [ -z "${VERSION}" ]; then
    echo "Error: VERSION environment variable is not set"
    exit 1
fi

echo "Building RPM package for version ${VERSION}"

# Install RPM build tools and dependencies
echo "Installing build dependencies..."
dnf install -y rpm-build rpmdevtools
dnf install -y cmake gcc gcc-c++ make
dnf install -y fftw3-devel libsndfile-devel libao-devel libsamplerate-devel
dnf install -y gtk2-devel ncurses-devel
dnf install -y python3-devel pybind11-devel python3-numpy chrpath

# Set up RPM build environment
echo "Setting up RPM build environment..."
rpmdev-setuptree

# Copy source to build directory
echo "Preparing source tarball..."
cd /src
tar czf ~/rpmbuild/SOURCES/waon-${VERSION}.tar.gz \
    --transform "s,^,waon-${VERSION}/," \
    --exclude='.git*' \
    --exclude='build*' \
    --exclude='*.o' \
    --exclude='*.so' \
    *

# Update spec file with version
echo "Updating spec file version to ${VERSION}..."
sed "s/Version:.*/Version: ${VERSION}/" packaging/rpm/waon.spec > ~/rpmbuild/SPECS/waon.spec

# Build RPM packages
echo "Building RPM packages..."
cd ~/rpmbuild
rpmbuild -ba SPECS/waon.spec

# Copy built packages to output
echo "Copying built packages to output..."
cp RPMS/x86_64/*.rpm /output/
cp SRPMS/*.src.rpm /output/ || true

echo "RPM package build complete!"
ls -la /output/*.rpm