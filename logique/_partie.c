// partie.c - version interface graphique
#include <stdio.h>
#include <stdlib.h>
#include <cjson/cJSON.h>
#include "_joueur.h"
#include "_Tuile.h"
#include "_table.h"

bool partie_en_cours(void) {
    FILE* f = fopen("nombre_joueurs.txt", "r");
    if (!f) return false;
    fclose(f);
    return true;
}

bool scores_existent(void) {
    FILE* f = fopen("scores.json", "r");
    if (!f) return false;
    fclose(f);
    return true;
}