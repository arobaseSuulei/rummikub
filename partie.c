// partie.c
#include <stdio.h>
#include <stdlib.h>
#include "joueur.h"
#include "Tuile.h"
#include "table.h"
#include "menu.h"

/* ------------------------------------------------------------------------- */
int lire_nombre_joueurs(void) {
    FILE* f = fopen("nombre_joueurs.txt", "r");
    if (!f) return 0;
    
    int n;
    fscanf(f, "%d", &n);
    fclose(f);
    return n;
}

/* ------------------------------------------------------------------------- */
Joueur* charger_tous_joueurs(int nb_joueurs) {
    Joueur* players = malloc(nb_joueurs * sizeof(Joueur));
    for (int i = 0; i < nb_joueurs; i++) {
        sprintf(players[i].chevalet, "%d.json", i+1);
        charger_joueur(&players[i]);
    }
    return players;
}

/* ------------------------------------------------------------------------- */
void lancer_partie(void) {
    printf("=== BIENVENUE AU RUMMIKUB ===\n\n");
    
    creer_pioche();
    creer_table();
    distribuer_tuile();
    
    int nb_joueurs = lire_nombre_joueurs();
    Joueur* players = charger_tous_joueurs(nb_joueurs);
    int joueur_actuel = 0;
    
    while (1) {
        Joueur* j = &players[joueur_actuel];
        printf("\n\n=== TOUR DE %s ===\n", j->pseudo);
        
        int continuer = 1;
        while (continuer) {
            afficher_menu_principal();
            int choix = choisir_option_menu();
            
            if (choix == 0) {
                printf("Partie terminée.\n");
                free(players);
                return;
            }
            
            executer_option(choix, j);
            
            if (choix == 2 || choix == 5) {
                continuer = 0;
            }
        }
        
        joueur_actuel = (joueur_actuel + 1) % nb_joueurs;
    }
    
    free(players);
}

bool partie_en_cours(void) {
    // Si le fichier nombre_joueurs.txt existe, une partie était en cours
    FILE* f = fopen("nombre_joueurs.txt", "r");
    if (!f) return false;
    
    fclose(f);
    return true;
}