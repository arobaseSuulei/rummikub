#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include "struct.h"
#include "Tuile.h"

#define MAX_TUILES 106

// Initialiser le paquet complet
void initialiser_tuile(Tuile t[], int *nb_tuiles){
    int index = 0;
    char couleurs[] = {'B','R','O','V'};

    for(int exemplaire = 0; exemplaire < 2; exemplaire++){
        for(int valeur = 1; valeur <= 13; valeur++){
            for(int c = 0; c < 4; c++){
                t[index++] = (Tuile){valeur, couleurs[c], false};
            }
        }
    }

    // 2 jokers
    t[index++] = (Tuile){0, 'J', true};
    t[index++] = (Tuile){0, 'J', true};

    *nb_tuiles = index; // 106
}

// Sauvegarder un tableau de tuiles en JSON
void stocker_tuile(Tuile t[], int nb_tuiles, const char *filename){
    if(!filename) return;

    FILE *f = fopen(filename, "r");
    if(f){ // fichier existe déjà
        fclose(f);
        printf("%s existe déjà, pas de création.\n", filename);
        return;
    }

    f = fopen(filename, "w");
    if(!f){
        perror("Erreur ouverture fichier");
        return;
    }

    fprintf(f, "[\n");
    for(int i = 0; i < nb_tuiles; i++){
        fprintf(f,
            "  {\"valeur\":%d,\"couleur\":\"%c\",\"joker\":%s}%s\n",
            t[i].valeur,
            t[i].couleur,
            t[i].joker ? "true" : "false",
            (i < nb_tuiles - 1) ? "," : ""
        );
    }
    fprintf(f, "]\n");
    fclose(f);
}

// Mélanger un tableau de tuiles (Fisher-Yates)
void melanger_tuiles(Tuile t[], int nb_tuiles){
    srand(time(NULL));
    for(int i = nb_tuiles - 1; i > 0; i--){
        int j = rand() % (i + 1);
        Tuile tmp = t[i];
        t[i] = t[j];
        t[j] = tmp;
    }
}

// Lire paquet.json
int lire_paquet_json(Tuile t[]){
    FILE *f = fopen("paquet.json", "r");
    if(!f){
        perror("Impossible d'ouvrir paquet.json");
        return 0;
    }

    char ligne[256];
    int index = 0;
    while(fgets(ligne, sizeof(ligne), f) && index < MAX_TUILES){
        int valeur;
        char couleur;
        if(sscanf(ligne, " {\"valeur\":%d,\"couleur\":\"%c\",", &valeur, &couleur) == 2){
            t[index].valeur = valeur;
            t[index].couleur = couleur;
            t[index].joker = (strstr(ligne, "\"joker\":true") != NULL);
            index++;
        }
    }

    fclose(f);
    return index;
}

// Créer pioche.json mélangé
void creer_pioche(){
    Tuile t[MAX_TUILES];
    int nb_tuiles = lire_paquet_json(t);
    if(nb_tuiles == 0) return;

    melanger_tuiles(t, nb_tuiles);

    FILE *f = fopen("pioche.json", "w");
    if(!f){
        perror("Erreur ouverture pioche.json");
        return;
    }

    fprintf(f, "[\n");
    for(int i = 0; i < nb_tuiles; i++){
        fprintf(f,
            "  {\"valeur\":%d,\"couleur\":\"%c\",\"joker\":%s}%s\n",
            t[i].valeur,
            t[i].couleur,
            t[i].joker ? "true" : "false",
            (i < nb_tuiles - 1) ? "," : ""
        );
    }
    fprintf(f, "]\n");
    fclose(f);
}
