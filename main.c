// main.c
#include <stdio.h>
#include <stdlib.h>
#include "partie.h"

int main(void) {
    printf("=== RUMMIKUB ===\n\n");
    
    if (partie_en_cours()) {
        printf("Une partie est en cours.\n");
        printf("1. Continuer\n");
        printf("2. Nouvelle partie\n");
        printf("0. Quitter\n");
        printf("Choix : ");
        
        int choix;
        scanf("%d", &choix);
        getchar();
        
        switch(choix) {
            case 1:
                // Reprendre la partie existante
                lancer_partie();
                return 0;
                
            case 2:
                // Nouvelle partie - supprimer les anciens fichiers
                remove("nombre_joueurs.txt");
                remove("pioche.json");
                remove("table.json");
                remove("table_virtuelle.json");
                remove("chevalet_virtuel.json");
                remove("tampon.json");
                for (int i = 1; i <= 4; i++) {
                    char filename[20];
                    sprintf(filename, "%d.json", i);
                    remove(filename);
                }
                // Pas de break : continue pour lancer nouvelle partie
                
            case 0:
                printf("Au revoir !\n");
                return 0;
                
            default:
                printf("Choix invalide.\n");
                return 1;
        }
    }
    
    // Nouvelle partie
    printf("\n--- NOUVELLE PARTIE ---\n");
    lancer_partie();
    
    return 0;
}