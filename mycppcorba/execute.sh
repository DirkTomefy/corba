#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_SCRIPT="$SCRIPT_DIR/build.sh"
BIN="$SCRIPT_DIR/build/mycppcorba"
OMNI_LOG_DIR="$HOME/.omniORB"
OMNI_LOG_FILE="/tmp/omniNames.log"

# ---------------------------------------------------------------
# Lance la compilation via build.sh
# ---------------------------------------------------------------
run_build() {
    echo "Lancement du build..."
    "$BUILD_SCRIPT"
}

# ---------------------------------------------------------------
# Verifie que le binaire existe avant de le lancer
# ---------------------------------------------------------------
check_binary() {
    if [ ! -x "$BIN" ]; then
        echo "Erreur : binaire introuvable ou non executable : $BIN" >&2
        exit 1
    fi
}

# ---------------------------------------------------------------
# Demarre omniNames s'il n'est pas deja en cours d'execution
# ---------------------------------------------------------------
start_omninames() {
    if pgrep -x omniNames > /dev/null; then
        echo "omniNames deja en cours d'execution."
        return 0
    fi

    echo "Demarrage d'omniNames..."
    mkdir -p "$OMNI_LOG_DIR"

    omniNames -start -logdir "$OMNI_LOG_DIR" > "$OMNI_LOG_FILE" 2>&1 &

    # Laisse le temps au service de s'initialiser
    sleep 1

    if ! pgrep -x omniNames > /dev/null; then
        echo "Erreur : omniNames n'a pas demarre." >&2
        echo "Consultez $OMNI_LOG_FILE pour plus de details." >&2
        exit 1
    fi

    echo "omniNames demarre (log : $OMNI_LOG_FILE)."
}

# ---------------------------------------------------------------
# Lance le serveur CORBA
# ---------------------------------------------------------------
run_server() {
    echo "Lancement du serveur CORBA..."
    echo "(Ctrl+C pour arreter)"
    echo ""
    "$BIN"
}

# ---------------------------------------------------------------
# Point d'entree
# ---------------------------------------------------------------
main() {
    run_build
    check_binary
    start_omninames
    run_server
}

main