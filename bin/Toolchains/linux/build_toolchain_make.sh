#!/usr/bin/env bash
set -e

# Configuration
ROOT="$HOME/Projets/Perso/VectemEngine/VectemBuildTool/bin/Toolchains/linux"
SYSROOT="$ROOT/sysroot"
BUILD="$ROOT/build"
LLVM_VERSION="21.1.0"
MUSL_VERSION="1.2.5"
JOBS=$(nproc)

#echo "[+] Initialisation"
#rm -rf "$BUILD" "$SYSROOT"
#mkdir -p "$BUILD" "$SYSROOT"

# Télécharger le binaire LLVM
echo "[+] Téléchargement LLVM ${LLVM_VERSION} binaire"
cd "$BUILD"
wget -nc https://github.com/llvm/llvm-project/releases/download/llvmorg-${LLVM_VERSION}/LLVM-${LLVM_VERSION}-Linux-X64.tar.xz
tar -xf LLVM-${LLVM_VERSION}-Linux-X64.tar.xz

# Déplacer clang+llvm dans le sysroot
mkdir -p "$SYSROOT/clang"
mv LLVM-${LLVM_VERSION}-Linux-X64/* "$SYSROOT/clang"

# Installer musl libc
echo "[+] Téléchargement et compilation de musl"
wget -nc https://musl.libc.org/releases/musl-${MUSL_VERSION}.tar.gz
tar xf musl-${MUSL_VERSION}.tar.gz
cd "$BUILD/musl-${MUSL_VERSION}"
./configure --prefix=/usr --syslibdir=/lib
make -j"$JOBS"
make DESTDIR="$SYSROOT" install

# Terminé
echo
echo "========================================"
echo " Toolchain Clang ${LLVM_VERSION} installée !"
echo " Sysroot -> $SYSROOT"
echo " Clang   -> $SYSROOT/clang/bin/clang++"
echo "========================================"
