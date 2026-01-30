#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Tuile.h"
#include "table.h"
#include "joueur.h"
#include "menu.h"

int main(void) {
    printf("=== BIENVENUE AU RUMMIKUB ===\n\n");
    
    // Initialisation
    /*creer_pioche();          // crée pioche.json
    creer_table();           // crée table.json vide
    distribuer_tuile(); */   // crée X.json + distribue 14 tuiles à chaque joueur
    
    // Après distribuer_tuile(), les joueurs sont créés avec leurs fichiers JSON
    // Mais nous n'avons pas d'objet Joueur en mémoire...
    
    // SOLUTION 1: Créer un joueur test comme avant
    Joueur j;
    strcpy(j.chevalet, "2.json");  // tester sur le joueur 1
    
    // Charger toutes les infos du joueur depuis le fichier
    charger_joueur(&j);
    
    printf("\n=== TEST AVANT MENU ===\n");
    printf("Pseudo      : %s\n", j.pseudo);
    printf("Chevalet    : %s\n", j.chevalet);
    printf("Tour actif  : %d\n", j.tour);
    printf("Premier tour: %d\n", j.premier_tour);
    
    // Utiliser le menu
    int continuer = 1;
    while (continuer) {
        afficher_menu_principal();
        int choix = choisir_option_menu();
        
        if (choix == 0) {
            continuer = 0;
        } else {
            executer_option(choix, &j);
        }
    }
    
    printf("\nMerci d'avoir joué !\n");
    
    return 0;
}