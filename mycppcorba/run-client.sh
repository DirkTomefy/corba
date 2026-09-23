#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BIN="$SCRIPT_DIR/build/client_test"
OMNI_LOG_FILE="/tmp/omniNames.log"

# ---------------------------------------------------------------
# Verifie que le client de test est compile
# ---------------------------------------------------------------
check_binary() {
    if [ ! -x "$BIN" ]; then
        echo "Erreur : client_test introuvable ou non executable." >&2
        echo "Lancez d'abord : ./build.sh" >&2
        exit 1
    fi
}

# ---------------------------------------------------------------
# Verifie que omniNames tourne
# ---------------------------------------------------------------
check_omninames() {
    if ! pgrep -f omniNames > /dev/null; then
        echo "Erreur : omniNames n'est pas en cours d'execution." >&2
        echo "Lancez-le avant de tester le client :" >&2
        echo "  mkdir -p ~/.omniORB" >&2
        echo "  omniNames -logdir ~/.omniORB &" >&2
        exit 1
    fi
}

# ---------------------------------------------------------------
# Lance le client
# ---------------------------------------------------------------
run_client() {
    echo "Lancement du client de test..."
    echo ""
    "$BIN"
}

# ---------------------------------------------------------------
# Point d'entree
# ---------------------------------------------------------------
main() {
    check_binary
    check_omninames
    run_client
}

main