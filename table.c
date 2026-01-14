/* table.c */
#include <stdio.h>
#include "table.h"

void creer_table(void) {
    FILE *f = fopen("table.json", "w");
    if (!f) return;

    /* table = liste de listes de tuiles (vide au départ) */
    fprintf(f,
        "[\n"
        "]\n"
    );

    fclose(f);
}
