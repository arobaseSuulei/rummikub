// main.c
#include <stdio.h>
#include <stdlib.h>
#include "partie.h"
#include "Tuile.h"
#include "table.h"

int main(void) {
    printf("=== RUMMIKUB ===\n\n");
    
    if (partie_en_cours()) {
        printf("Une partie est en cours.\n");
        printf("1. Reprendre la partie\n");
        printf("2. Recommencer une nouvelle partie\n");
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
                for (int i = 1; i <= 4; i++) {
                    char filename[20];
                    sprintf(filename, "%d.json", i);
                    remove(filename);
                }
                // Continuer pour créer nouvelle partie
                break;
                
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
    creer_pioche();
    creer_table();
    lancer_partie();
    
    return 0;
}