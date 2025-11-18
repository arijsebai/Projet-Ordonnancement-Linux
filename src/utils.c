// src/utils.c
// Fichier utilitaire – on le garde pour l'instant (on y mettra des fonctions plus tard)
// Le simple fait d'avoir une fonction empêche l'erreur "empty translation unit"

#include <stdio.h>

void dummy_utils_function(void) {
    // Cette fonction ne fait rien, mais permet au fichier de compiler avec -Werror=pedantic
    (void)0;
}
