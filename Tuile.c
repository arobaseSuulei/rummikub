#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include "struct.h"
#include "Tuile.h"

/*------------------------------------------------------------------------------------------------------------------------------------- */
// créer le tableau de tuiles avec ID
Tuile* initialiser_tuile() {

    // allouer dynamiquement la mémoire
    Tuile* t = malloc(MAX_TUILES * sizeof(Tuile));

    if (!t) {
        printf("Erreur d'allocation!");
        return NULL;
    } 

    int index = 0;
    int id = 1;
    char couleurs[] = {'B','R','O','V'};

    // on initialise les 104 tuiles ordinaires
    for (int exemplaire = 0; exemplaire < 2; exemplaire++) {
        for (int valeur = 1; valeur <= 13; valeur++) {
            for (int c = 0; c < 4; c++) {
                t[index++] = (Tuile){id++, valeur, couleurs[c], false};
            }
        }
    }

    // plus les 2 jokers
    t[index++] = (Tuile){id++, 0, 'J', true};
    t[index++] = (Tuile){id++, 0, 'J', true};

    /*--------------------------Fin tableau-----------------------------------*/
    return t;
}


/*------------------------------------------------------------------------------------------------------------------------------------- */
// Mélanger un tableau de tuiles (Fisher-Yates) en utilisant initialiser_tuile
Tuile* melanger_tuiles() {
    // créer et récupérer le tableau
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
// Créer pioche.json mélangé
void creer_pioche() {
    // créer et mélanger les tuiles
    Tuile* t = melanger_tuiles();
    if (!t) return;

    int nb_tuiles = MAX_TUILES;
    if (nb_tuiles == 0) {
        free(t);
        return;
    }

    FILE *f = fopen("pioche.json", "w");
    if (!f) {
        perror("Erreur ouverture pioche.json");
        free(t);
        return;
    }

    fprintf(f, "[\n");
    for (int i = 0; i < nb_tuiles; i++) {
        fprintf(f,
            "  {\"id\":%d,\"valeur\":%d,\"couleur\":\"%c\",\"joker\":%s}%s\n",
            t[i].id,
            t[i].valeur,
            t[i].couleur,
            t[i].joker ? "true" : "false",
            (i < nb_tuiles - 1) ? "," : ""
        );
    }
    fprintf(f, "]\n");
    printf("**********Pioche cree avec succes**********\n");

    fclose(f);
    free(t); // libération mémoire dynamique
}

/*----------------*/
