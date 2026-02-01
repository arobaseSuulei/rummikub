// menu.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>  
#include "menu.h"
#include "joueur.h"
#include "Tuile.h"
#include "table.h"
#include "manipulation.h"
#include "partie.h"

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
void afficher_menu_principal(void) {
    printf("\n=== MENU PRINCIPAL - RUMMIKUB ===\n");
    printf("1. Jouer une combinaison\n");
    printf("2. Piocher une tuile et passer mon tour\n");
    printf("3. Afficher mon chevalet\n");
    printf("4. Afficher la table\n");
    printf("5. Manipuler la table\n");
    printf("6. Passer mon tour (sans piocher)\n");  // <-- NOUVELLE
    printf("0. Quitter la partie\n");
    printf("---------------------------------\n");
}

/* ------------------------------------------------------------------------- */
int choisir_option_menu(void) {
    int choix;
    printf("Votre choix (0-6) : ");  // <-- Changé à 6
    
    while(1) {
        if (scanf("%d", &choix) != 1) {
            printf("Entrée invalide. Réessayez : ");
            while(getchar() != '\n');
            continue;
        }
        
        if (choix >= 0 && choix <= 6) {  // <-- Changé à 6
            getchar();
            return choix;
        }
        
        printf("Choix invalide. Entrez un nombre entre 0 et 6 : ");  // <-- 6
    }
}

/* ------------------------------------------------------------------------- */

bool executer_option(int choix, Joueur* j) {
    switch(choix) {
        case 0:
            printf("Merci d'avoir joué ! À bientôt.\n");
            exit(0);
            
        case 1:
            printf("\n>>> JOUER UNE COMBINAISON\n");
            return jouer_combinaison(j);
            
        case 2:
            printf("\n>>> PIOCHE ET FIN DE TOUR\n");
            piocher_tuile(j);
            {
                Tuile tuiles[MAX_TUILES];
                int nb_tuiles = 0;
                charger_chevalet(j->chevalet, tuiles, &nb_tuiles);
                printf("Vous avez pioché. Vous avez maintenant %d tuiles.\n", nb_tuiles);
                printf("Votre tour est terminé.\n");
            }
            return true;
            
        case 3:
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
            return false;
            
        case 4:
            printf("\n>>> TABLE ACTUELLE\n");
            afficher_table("table.json");
            return false;
            
        case 5:
            printf("\n>>> MANIPULATION DE LA TABLE\n");
            FILE* f = fopen("table.json", "r");
            if (!f) {
                printf("La table est vide. Vous ne pouvez pas la manipuler.\n");
                return false;
            }
            
            fseek(f, 0, SEEK_END);
            long fsize = ftell(f);
            fclose(f);
            
            if (fsize <= 2) {
                printf("La table est vide. Vous ne pouvez pas la manipuler.\n");
                return false;
            }
            
            return menu_manipulation(j);
            
        case 6:
            printf("\n>>> PASSER SON TOUR\n");
            printf("Vous passez votre tour sans piocher.\n");
            printf("Votre tour est terminé.\n");
            return true;
            
        default:
            return false;
    }
}

static void initialiser_virtuel(Joueur* j) {
    // Copier table.json → table_virtuelle.json
    FILE *src = fopen("table.json", "r");
    FILE *dst = fopen("table_virtuelle.json", "w");
    if (src && dst) {
        char buffer[1024];
        size_t n;
        while ((n = fread(buffer, 1, sizeof(buffer), src)) > 0) {
            fwrite(buffer, 1, n, dst);
        }
        fclose(src);
        fclose(dst);
    }
    
    // Copier chevalet → chevalet_virtuel.json
    src = fopen(j->chevalet, "r");
    dst = fopen("chevalet_virtuel.json", "w");
    if (src && dst) {
        char buffer[1024];
        size_t n;
        while ((n = fread(buffer, 1, sizeof(buffer), src)) > 0) {
            fwrite(buffer, 1, n, dst);
        }
        fclose(src);
        fclose(dst);
    }
}

static void afficher_chevalet_virtuel(void) {
    Tuile tuiles[MAX_TUILES];
    int nb_tuiles = 0;
    charger_chevalet("chevalet_virtuel.json", tuiles, &nb_tuiles);
    afficher_tuiles(tuiles, nb_tuiles);
}

bool menu_manipulation(Joueur* j) {
    printf("\n>>> MANIPULATION DE LA TABLE\n");
    
    // Initialiser
    initialiser_virtuel(j);
    
    // Boucle de manipulation
    while (1) {
        printf("\n=== TABLE VIRTUELLE ===\n");
        afficher_table("table_virtuelle.json");
        
        printf("\n=== VOTRE CHEVALET ===\n");
        afficher_chevalet_virtuel();
        
        printf("\n=== MENU MANIPULATION ===\n");
        printf("1. Isoler une tuile de la table\n");
        printf("2. Isoler une tuile de mon chevalet\n");
        printf("3. Ajouter une tuile à une combinaison\n");
        printf("4. Valider et appliquer\n");
        printf("5. Recommencer (annuler et repartir de zéro)\n");
        printf("6. Quitter (abandonner la manipulation)\n");
        printf("Choix (1-6) : ");
        
        int choix_manip;
        scanf("%d", &choix_manip);
        getchar();
        
        if (choix_manip == 1) {
            printf("\nEntrez l'ID de la tuile à isoler : ");
            int tuile_id;
            scanf("%d", &tuile_id);
            getchar();
            isoler_tuile(tuile_id);
            
        } else if (choix_manip == 2) {
            printf("\nEntrez l'ID de la tuile à isoler de votre chevalet : ");
            int tuile_id;
            scanf("%d", &tuile_id);
            getchar();
            isoler_tuile_chevalet(tuile_id);
            
        } else if (choix_manip == 3) {
            printf("\nEntrez l'ID de la tuile à ajouter : ");
            int tuile_id;
            scanf("%d", &tuile_id);
            getchar();
            
            printf("Entrez l'index de la combinaison cible : ");
            int comb_index;
            scanf("%d", &comb_index);
            getchar();
            
            ajouter_tuile_combinaison(tuile_id, comb_index);
            
        } else if (choix_manip == 4) {
            if (valider_tour(j)) {
                printf("\n🎉 Tour validé avec succès !\n");
                remove("table_virtuelle.json");
                remove("chevalet_virtuel.json");
                return true; // Tour terminé
            }
            // Si échec, reste dans le menu
            
        } else if (choix_manip == 5) {
            remove("table_virtuelle.json");
            remove("chevalet_virtuel.json");
            initialiser_virtuel(j);
            printf("Manipulation recommencée.\n");
            
        } else if (choix_manip == 6) {
            remove("table_virtuelle.json");
            remove("chevalet_virtuel.json");
            printf("Manipulation abandonnée.\n");
            return false; // Tour non terminé
            
        } else {
            printf("Choix invalide\n");
        }
    }
}

void menu_principal(void) {
    while (1) {
        printf("\n=== RUMMIKUB ===\n\n");
        
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
                    if (!lancer_partie()) {
                        // Partie déjà terminée
                        continue;
                    }
                    // Partie jouée et terminée (gagnant)
                    continue;  // ← Revenir au menu
                    
                case 2:
                    // Nouvelle partie - supprimer les anciens fichiers
                    remove("nombre_joueurs.txt");
                    remove("pioche.json");
                    remove("table.json");
                    remove("table_virtuelle.json");
                    remove("chevalet_virtuel.json");
                    remove("tampon.json");
                    remove("scores.json");
                    for (int i = 1; i <= 4; i++) {
                        char filename[20];
                        sprintf(filename, "%d.json", i);
                        remove(filename);
                    }
                    
                    printf("\n--- NOUVELLE PARTIE ---\n");
                    lancer_partie();
                    continue;  // ← Revenir au menu après
                    
                case 0:
                    printf("Au revoir !\n");
                    exit(0);
                    
                default:
                    printf("Choix invalide.\n");
                    continue;
            }
        }
        
        // Nouvelle partie (aucun fichier existant)
        printf("\n--- NOUVELLE PARTIE ---\n");
        lancer_partie();
        continue;  // ← Revenir au menu après
    }
}