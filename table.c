#include <stdio.h>
#include <stdlib.h>
#include <cjson/cJSON.h>
#include "table.h"

void creer_table(void) {
    cJSON *root = cJSON_CreateArray();   // table vide

    char *str = cJSON_Print(root);
    FILE *f = fopen("table.json", "w");
    if (f) {
        fprintf(f, "%s", str);
        fclose(f);
    }

    cJSON_Delete(root);
    free(str);
}
