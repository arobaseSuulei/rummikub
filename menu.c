// menu.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "menu.h"
#include "joueur.h"
#include "Tuile.h"  
#include "table.h"
#include "manipulation.h"

/* ------------------------------------------------------------------------- */
void afficher_menu_principal(void) {
    printf("\n=== MENU PRINCIPAL - RUMMIKUB ===\n");
    printf("1. Jouer une combinaison\n");
    printf("2. Récupérer un joker\n");
    printf("3. Étendre une suite et récupérer une tuile\n");
    printf("4. Remplacer une tuile dans une combinaison\n");
    printf("5. Diviser une suite en deux\n");
    printf("6. Retirer une tuile d'une combinaison\n");
    printf("7. Piocher une tuile et passer mon tour\n");
    printf("8. Afficher mon chevalet\n");
    printf("9. Ajouter une tuile à une combinaison existante\n");  // NOUVEAU
    printf("0. Quitter la partie\n");
    printf("---------------------------------\n");
}

/* ------------------------------------------------------------------------- */
int choisir_option_menu(void) {
    int choix;
    printf("Votre choix (0-9) : ");  // Retour à 0-9
    
    while(1) {
        if (scanf("%d", &choix) != 1) {
            printf("Entrée invalide. Réessayez : ");
            while(getchar() != '\n');
            continue;
        }
        
        if (choix >= 0 && choix <= 9) {  // 0-9
            getchar();
            return choix;
        }
        
        printf("Choix invalide. Entrez un nombre entre 0 et 9 : ");
    }
}

/* ------------------------------------------------------------------------- */
void executer_option(int choix, Joueur* j) {
    // Afficher la table avant chaque action (sauf affichage chevalet et pioche)
    if (choix != 8 && choix != 0 && choix != 7 && choix != 9) {
        printf("\n=== TABLE ACTUELLE ===\n");
        afficher_combinaisons_table();
        printf("=======================\n");
    }
    
    switch(choix) {
        case 0: // Quitter
            printf("Merci d'avoir joué ! À bientôt.\n");
            exit(0);
            break;
            
        case 1: // Jouer une combinaison
            printf("\n>>> JOUER UNE COMBINAISON\n");
            jouer_combinaison(j);
            break;
            
        case 2: // Récupérer un joker
            printf("\n>>> RÉCUPÉRATION D'UN JOKER\n");
            if (!traiter_joker(j)) {
                printf("Récupération du joker annulée ou échouée.\n");
            }
            break;
            
        case 3: // Étendre une suite
            printf("\n>>> EXTENSION DE SUITE\n");
            if (!traiter_extension_suite(j)) {
                printf("Extension de suite annulée ou échouée.\n");
            }
            break;
            
        case 4: // Remplacer une tuile
            printf("\n>>> REMPLACEMENT DE TUILE\n");
            if (!traiter_remplacement(j)) {
                printf("Remplacement de tuile annulé ou échoué.\n");
            }
            break;
            
        case 5: // Diviser une suite
            printf("\n>>> DIVISION DE SUITE\n");
            if (!traiter_division(j)) {
                printf("Division de suite annulée ou échouée.\n");
            }
            break;
            
        case 6: // Retirer une tuile
            printf("\n>>> RETRAIT DE TUILE\n");
            if (!traiter_retrait(j)) {
                printf("Retrait de tuile annulé ou échoué.\n");
            }
            break;
            
        case 7: // Piocher une tuile ET passer le tour
            printf("\n>>> PIOCHE ET FIN DE TOUR\n");
            piocher_tuile(j);
            {
                Tuile tuiles[MAX_TUILES];
                int nb_tuiles = 0;
                charger_chevalet(j->chevalet, tuiles, &nb_tuiles);
                printf("Vous avez pioché. Vous avez maintenant %d tuiles.\n", nb_tuiles);
                printf("Votre tour est terminé.\n");
            }
            break;
            
        case 8: // Afficher mon chevalet
            printf("\n>>> MON CHEVALET\n");
            {
                Tuile tuiles[MAX_TUILES];
                int nb_tuiles = 0;
                charger_chevalet(j->chevalet, tuiles, &nb_tuiles);
                if (nb_tuiles == 0) {
                    printf("Félicitations ! Vous n'avez plus de tuiles !\n");
                } else {
                    afficher_tuiles(tuiles, nb_tuiles);
                }
            }
            break;
            
        case 9: // Ajouter une tuile à une combinaison existante
            printf("\n>>> AJOUTER UNE TUILE À UNE COMBINAISON EXISTANTE\n");
            // Afficher la table pour cette option
            printf("\n=== TABLE ACTUELLE ===\n");
            afficher_combinaisons_table();
            printf("=======================\n");
            
            if (!ajouter_tuile_combinaison_existante(j)) {
                printf("Ajout annulé ou échoué.\n");
            }
            break;
            
        default:
            printf("Option non reconnue.\n");
            break;
    }
}

/* ------------------------------------------------------------------------- */
void executer_boucle_jeu(Joueur* joueurs, int nb_joueurs) {
    int joueur_actuel = 0;
    bool partie_terminee = false;
    
    while (!partie_terminee) {
        printf("\n\n=== TOUR DE %s ===\n", joueurs[joueur_actuel].pseudo);
        
        // Charger les infos du joueur actuel
        charger_joueur(&joueurs[joueur_actuel]);
        
        // Afficher le menu et exécuter les actions
        afficher_menu_principal();
        int choix = choisir_option_menu();
        executer_option(choix, &joueurs[joueur_actuel]);
        
        // Vérifier si le joueur a gagné (plus de tuiles)
        {
            Tuile tuiles[MAX_TUILES];
            int nb_tuiles = 0;
            charger_chevalet(joueurs[joueur_actuel].chevalet, tuiles, &nb_tuiles);
            
            if (nb_tuiles == 0) {
                printf("\n\n🎉 FÉLICITATIONS %s ! 🎉\n", joueurs[joueur_actuel].pseudo);
                printf("Vous avez posé toutes vos tuiles !\n");
                partie_terminee = true;
                break;
            }
        }
        
        // Passer au joueur suivant
        joueur_actuel = (joueur_actuel + 1) % nb_joueurs;
    }
    
    printf("\n=== PARTIE TERMINÉE ===\n");
}