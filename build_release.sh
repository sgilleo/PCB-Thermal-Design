#!/bin/bash

cmake --preset linux-x64-release && cmake --build --preset linux-x64-release
cd build/linux-release
rm -rf AppDir
mkdir -p AppDir/usr/bin
cp PCB_Thermal_Design AppDir/usr/bin/
cp -r fonts AppDir/usr/bin/
cp ../../res/icon.png AppDir/
cp ../../res/PCB_Thermal_Design.desktop AppDir/
cp ../../res/AppRun AppDir/
chmod +x AppDir/AppRun

appimagetool AppDir PCB_Thermal_Design.AppImage

