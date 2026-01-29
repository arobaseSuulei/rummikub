#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "menu.h"
#include "joueur.h"
#include "Tuile.h"  
#include "table.h"
/* ------------------------------------------------------------------------- */
void afficher_menu_principal(void) {
    printf("\n=== MENU PRINCIPAL - RUMMIKUB ===\n");
    printf("1. Jouer une combinaison\n");
    printf("2. Manipuler la table\n");
    printf("3. Piocher une tuile\n");
    printf("4. Afficher mon chevalet\n");
    printf("5. Passer mon tour\n");  // Déplacé de 6 à 5
    printf("0. Quitter la partie\n");
    printf("---------------------------------\n");
}

/* ------------------------------------------------------------------------- */
int choisir_option_menu(void) {
    int choix;
    printf("Votre choix (0-6) : ");
    
    while(1) {
        if (scanf("%d", &choix) != 1) {
            printf("Entrée invalide. Réessayez : ");
            while(getchar() != '\n'); // Vider le buffer
            continue;
        }
        
        if (choix >= 0 && choix <= 6) {
            getchar(); // Consommer le newline
            return choix;
        }
        
        printf("Choix invalide. Entrez un nombre entre 0 et 6 : ");
    }
}

/* ------------------------------------------------------------------------- */
void executer_option(int choix, Joueur* j) {
    // TOUJOURS afficher la table AVANT de demander une action
    printf("\n=== TABLE ACTUELLE ===\n");
    afficher_combinaisons_table();
    printf("=======================\n");
    
    switch(choix) {
        case 0: // Quitter
            printf("Merci d'avoir joué ! À bientôt.\n");
            exit(0);
            break;
            
        case 1: // Jouer une combinaison
            printf("\n>>> JOUER UNE COMBINAISON\n");
            jouer_combinaison(j);
            break;
            
        case 2: // Manipuler la table
            printf("\n>>> MANIPULATION DE LA TABLE\n");
            if (!manipuler_table(j)) {
                printf("Manipulation annulée ou échouée.\n");
            }
            break;
            
        case 3: // Piocher une tuile
            printf("\n>>> PIOCHE D'UNE TUILE\n");
            piocher_tuile(j);
            {
                Tuile tuiles[MAX_TUILES];
                int nb_tuiles = 0;
                charger_chevalet(j->chevalet, tuiles, &nb_tuiles);
                printf("Vous avez pioché. Vous avez maintenant %d tuiles.\n", nb_tuiles);
            }
            break;
            
        case 4: // Afficher mon chevalet
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
            
        case 5: // Passer mon tour (anciennement 6)
            printf("\n>>> TOUR PASSÉ\n");
            printf("Vous passez votre tour.\n");
            break;
            
        default:
            printf("Option non reconnue.\n");
            break;
    }
}

/* ------------------------------------------------------------------------- */
// Fonction pour exécuter la boucle principale du jeu
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
            // MODIF: Déclarer les variables dans un bloc
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
    // Ici tu pourras ajouter le calcul des scores finaux
}