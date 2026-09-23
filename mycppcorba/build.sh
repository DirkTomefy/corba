#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
GENERATED_DIR="$SCRIPT_DIR/src/generated"
IDL_DIR="$SCRIPT_DIR/idl"
BIN_NAME="mycppcorba"

# ---------------------------------------------------------------
# Génère les stubs CORBA à partir du fichier IDL
# ---------------------------------------------------------------
generate_stubs() {
    echo "Generation des stubs CORBA..."
    mkdir -p "$GENERATED_DIR"
    omniidl -bcxx -Wbh=.h -Wbs=.cpp \
            -I"$IDL_DIR" \
            -C"$GENERATED_DIR" \
            "$IDL_DIR/etudiant.idl"
    echo "Stubs generes dans $GENERATED_DIR"
}

# ---------------------------------------------------------------
# Nettoie et prépare le répertoire de build
# ---------------------------------------------------------------
prepare_build_dir() {
    echo "Preparation du repertoire de build..."
    rm -rf "$BUILD_DIR"
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
}

# ---------------------------------------------------------------
# Configure et compile le projet
# ---------------------------------------------------------------
compile() {
    echo "Configuration de CMake..."
    cmake ..

    echo "Compilation..."
    make
}

# ---------------------------------------------------------------
# Point d'entrée
# ---------------------------------------------------------------
main() {
    generate_stubs
    prepare_build_dir
    compile

    echo ""
    echo "Build termine. Binaire disponible dans : $BUILD_DIR/$BIN_NAME"
}

main