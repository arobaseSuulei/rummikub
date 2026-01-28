#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <cjson/cJSON.h>
#include "struct.h"
#include "Tuile.h"

/*------------------------------------------------------------------------------------------------------------------------------------- */
// créer le tableau de tuiles avec ID
Tuile* initialiser_tuile() {
    Tuile* t = malloc(MAX_TUILES * sizeof(Tuile));
    if (!t) {
        printf("Erreur d'allocation!");
        return NULL;
    } 

    int index = 0;
    int id = 1;
    char couleurs[] = {'B','R','O','V'};

    for (int exemplaire = 0; exemplaire < 2; exemplaire++) {
        for (int valeur = 1; valeur <= 13; valeur++) {
            for (int c = 0; c < 4; c++) {
                t[index++] = (Tuile){id++, valeur, couleurs[c], false};
            }
        }
    }

    // 2 jokers
    t[index++] = (Tuile){id++, 0, 'J', true};
    t[index++] = (Tuile){id++, 0, 'J', true};

    return t;
}

/*------------------------------------------------------------------------------------------------------------------------------------- */
// Mélanger un tableau de tuiles (Fisher-Yates)
Tuile* melanger_tuiles() {
    Tuile* t = initialiser_tuile();
    if (!t) return NULL;

    int nb_tuiles = MAX_TUILES;
    srand(time(NULL));

    for (int i = nb_tuiles - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Tuile tmp = t[i];
        t[i] = t[j];
        t[j] = tmp;
    }

    return t;
}

/*------------------------------------------------------------------------------------------------------------------------------------- */
// Créer pioche.json mélangé avec cJSON
void creer_pioche() {
    Tuile* t = melanger_tuiles();
    if (!t) return;

    cJSON *root = cJSON_CreateArray();
    for (int i = 0; i < MAX_TUILES; i++) {
        cJSON *item = cJSON_CreateObject();
        cJSON_AddNumberToObject(item, "id", t[i].id);
        cJSON_AddNumberToObject(item, "valeur", t[i].valeur);
        char str_color[2] = {t[i].couleur, 0};
        cJSON_AddStringToObject(item, "couleur", str_color);
        cJSON_AddBoolToObject(item, "joker", t[i].joker);
        cJSON_AddItemToArray(root, item);
    }

    char *str = cJSON_Print(root);
    FILE *f = fopen("pioche.json", "w");
    if (f) {
        fprintf(f, "%s", str);
        fclose(f);
    } else {
        perror("Erreur ouverture pioche.json");
    }

    cJSON_Delete(root);
    free(str);
    free(t);

    printf("********** Pioche cree avec succes **********\n");
}
