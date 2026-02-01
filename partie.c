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
    
    FILE* test = fopen("1.json", "r");
    if (!test) {
        printf("--- INITIALISATION NOUVELLE PARTIE ---\n");
        creer_pioche();
        creer_table();
        distribuer_tuile();
    } else {
        fclose(test);
        printf("--- REPRISE DE PARTIE ---\n");
    }
    
    int nb_joueurs = lire_nombre_joueurs();
    Joueur* players = charger_tous_joueurs(nb_joueurs);
    
    // Trouver joueur actuel (celui avec tour==true dans son fichier)
    int joueur_actuel = 0;
    for (int i = 0; i < nb_joueurs; i++) {
        charger_joueur(&players[i]); // Recharger pour avoir tour à jour
        if (players[i].tour) {
            joueur_actuel = i;
            break;
        }
    }
    
    printf("C'est à %s de jouer.\n", players[joueur_actuel].pseudo);
    
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
            
            bool tour_termine = executer_option(choix, j);
            
            if (tour_termine) {
                int next = (joueur_actuel + 1) % nb_joueurs;
                passer_tour(j, &players[next]);
                continuer = 0;
            }
            // Sinon, reste au même joueur
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