#!/usr/bin/env bash
set -e

# Config
ROOT="./bin/Toolchains/linux"
SYSROOT_FOLDER="sysroot"
BUILD_FOLDER="build"
LLVM_VERSION="21.1.0"
MUSL_VERSION="1.2.5"
JOBS=$(nproc)

# Initialize
echo "[+] Initialization"
mkdir -p "$ROOT"
cd "$ROOT"

mkdir -p "$SYSROOT_FOLDER" "$BUILD_FOLDER" 

# LLVM Downloading
echo "[+] Download LLVM ${LLVM_VERSION} binary"
cd "$BUILD_FOLDER"
wget -nc https://github.com/llvm/llvm-project/releases/download/llvmorg-${LLVM_VERSION}/LLVM-${LLVM_VERSION}-Linux-X64.tar.xz
tar -xf LLVM-${LLVM_VERSION}-Linux-X64.tar.xz
cd ..

# Move clang+llvm tosysroot
mkdir -p "$SYSROOT_FOLDER/clang"
mv $BUILD_FOLDER/LLVM-${LLVM_VERSION}-Linux-X64/* $SYSROOT_FOLDER/clang

# Install musl libc
echo "[+] Dawnloading and compiling musl"
cd "$BUILD_FOLDER"
wget -nc https://musl.libc.org/releases/musl-${MUSL_VERSION}.tar.gz
tar xf musl-${MUSL_VERSION}.tar.gz
cd "musl-${MUSL_VERSION}"
./configure --prefix=/usr --syslibdir=/lib
make -j"$JOBS"
make DESTDIR="../../$SYSROOT_FOLDER" install
cd ../..

echo "[+] Cleaning up"
rm -rf "$BUILD_FOLDER"

# Finished
echo
echo "========================================"
echo " Toolchain Clang ${LLVM_VERSION} installed !"
echo " Sysroot -> $ROOT/$SYSROOT_FOLDER"
echo " Clang   -> $ROOT/$SYSROOT_FOLDER/clang/bin/clang++"
echo "========================================"
