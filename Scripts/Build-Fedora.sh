#!/bin/sh

# Set up:
VRUI_VERSION=2.2
VRUI_RELEASE=002

# Get prerequisite packages:
PREREQUISITE_PACKAGES="libusb1-devel zlib-devel libpng-devel libjpeg-devel libtiff-devel alsa-lib-devel speex-devel openal-soft-devel libv4l-devel libdc1394-devel libogg-devel libtheora-devel bluez-libs-devel mesa-libGL-devel mesa-libGLU-devel"
echo "Please enter your password to install Vrui's prerequisite packages"
sudo yum install $PREREQUISITE_PACKAGES
INSTALL_RESULT=$?

if [ $INSTALL_RESULT -ne 0 ]; then
	echo "Problem while downloading prerequisite packages; please fix the issue and try again"
	exit $INSTALL_RESULT
fi

# Create src directory:
echo "Creating source code directory $HOME/src"
cd $HOME
mkdir src
cd src
CD_RESULT=$?

if [ $CD_RESULT -ne 0 ]; then
	echo "Could not create source code directory $HOME/src. Please fix the issue and try again"
	exit $CD_RESULT
fi

# Download Vrui tarball:
echo "Downloading Vrui-$VRUI_VERSION-$VRUI_RELEASE into $HOME/src"
wget http://stout.idav.ucdavis.edu/Vrui-$VRUI_VERSION-$VRUI_RELEASE.tar.gz
DOWNLOAD_RESULT=$?

if [ $DOWNLOAD_RESULT -ne 0 ]; then
	echo "Problem while downloading Vrui tarball; please check your network connection and try again"
	exit $DOWNLOAD_RESULT
fi

# Unpack Vrui tarball:
echo "Unpacking Vrui tarball into $HOME/src/Vrui-$VRUI_VERSION-$VRUI_RELEASE"
tar xfz Vrui-$VRUI_VERSION-$VRUI_RELEASE.tar.gz
cd Vrui-$VRUI_VERSION-$VRUI_RELEASE
UNPACK_RESULT=$?

if [ $UNPACK_RESULT -ne 0 ]; then
	echo "Problem while unpacking Vrui tarball; please fix the issue and try again"
	exit $UNPACK_RESULT
fi

# Determine the number of CPUs on the host computer:
NUM_CPUS=`cat /proc/cpuinfo | grep processor | wc -l`

# Build Vrui:
echo "Building Vrui on $NUM_CPUS CPUs"
make -j$NUM_CPUS
BUILD_RESULT=$?

if [ $BUILD_RESULT -ne 0 ]; then
	echo "Build unsuccessful; please fix any reported errors and try again"
	exit $BUILD_RESULT
fi

# Install Vrui
echo "Build successful; installing Vrui in $HOME/Vrui-$VRUI_VERSION"
make install
INSTALL_RESULT=$?

if [ $INSTALL_RESULT -ne 0 ]; then
	echo "Could not install Vrui in $HOME/Vrui-$VRUI_VERSION. Please fix the issue and try again"
	exit $INSTALL_RESULT
fi

# Build Vrui example applications
cd ExamplePrograms
echo "Building Vrui example programs on $NUM_CPUS CPUs"
make -j$NUM_CPUS
BUILD_RESULT=$?

if [ $BUILD_RESULT -ne 0 ]; then
	echo "Build unsuccessful; please fix any reported errors and try again"
	exit $BUILD_RESULT
fi

# Run ShowEarthModel
echo "Running ShowEarthModel application. Press Esc or close the window to exit."
./bin/ShowEarthModel

