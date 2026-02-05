#!/usr/bin/env bash
set -e

exit 0 # Temporary disable until we have a better solution for downloading and hosting the toolchain files (LLVM, glibc, libstdc++)

# =======================
# Config
# =======================
ROOT="$(pwd)/bin/Toolchains/linux"
SYSROOT_FOLDER="sysroot"
BUILD_FOLDER="build"
LLVM_VERSION="21.1.0"

# Detect host triple (Ubuntu/Debian)
HOST_TRIPLE="x86_64-linux-gnu"
GCC_VERSION=$(gcc -dumpversion)

GLIBC_TAR="https://example.com/glibc-${GLIBC_VERSION}-x86_64.tar.xz"
LIBCXX_TAR="https://example.com/libcxx-${LIBCXX_VERSION}-x86_64.tar.xz"

# =======================
# Init
# =======================
echo "[+] Initialization"
mkdir -p "$ROOT"
cd "$ROOT"

rm -rf "$SYSROOT_FOLDER" "$BUILD_FOLDER"
mkdir -p "$SYSROOT_FOLDER" "$BUILD_FOLDER"

# =======================
# LLVM
# =======================
echo "[+] Download LLVM ${LLVM_VERSION}"
cd "$BUILD_FOLDER"

wget -nc https://github.com/llvm/llvm-project/releases/download/llvmorg-${LLVM_VERSION}/LLVM-${LLVM_VERSION}-Linux-X64.tar.xz
tar -xf LLVM-${LLVM_VERSION}-Linux-X64.tar.xz

cd ..

echo "[+] Installing clang into sysroot"
mkdir -p "$SYSROOT_FOLDER/clang"
cp -r "$BUILD_FOLDER/LLVM-${LLVM_VERSION}-Linux-X64/"* "$SYSROOT_FOLDER/clang"

# =======================
# glibc + GCC runtime
# =======================
echo "[+] Installing glibc and GCC runtime into sysroot"

mkdir -p "$SYSROOT_FOLDER/usr"
mkdir -p "$SYSROOT_FOLDER/lib"

# glibc (libs + loader)
cp -a /lib/$HOST_TRIPLE          "$SYSROOT_FOLDER/lib/"
cp -a /usr/lib/$HOST_TRIPLE      "$SYSROOT_FOLDER/usr/lib/"
cp -a /usr/include               "$SYSROOT_FOLDER/usr/"

cp -a /usr/include/c++ "$SYSROOT_FOLDER/usr/include/"
cp -a /usr/include/$HOST_TRIPLE/c++ "$SYSROOT_FOLDER/usr/include/$HOST_TRIPLE/"

# GCC runtime (crt + libgcc + libstdc++)
cp -a /usr/lib/gcc/$HOST_TRIPLE  "$SYSROOT_FOLDER/usr/lib/gcc/"

# Symlink loader
ln -sf /lib/$HOST_TRIPLE/ld-linux-x86-64.so.2 \
       "$SYSROOT_FOLDER/lib/ld-linux-x86-64.so.2"

# =======================
# Cleanup
# =======================
echo "[+] Cleaning up"
rm -rf "$BUILD_FOLDER"

# =======================
# Done
# =======================
echo
echo "========================================"
echo " Toolchain Clang ${LLVM_VERSION} installed"
echo " Sysroot  -> $ROOT/$SYSROOT_FOLDER"
echo " Clang    -> $ROOT/$SYSROOT_FOLDER/clang/bin/clang++"
echo " libc     -> glibc (host)"
echo " runtime  -> GCC (libstdc++)"
echo "========================================"
