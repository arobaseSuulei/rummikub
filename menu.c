// menu.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>  
#include "menu.h"
#include "joueur.h"
#include "Tuile.h"
#include "manipulation.h"

/* ------------------------------------------------------------------------- */
void afficher_menu_principal(void) {
    printf("\n=== MENU PRINCIPAL - RUMMIKUB ===\n");
    printf("1. Jouer une combinaison\n");
    printf("2. Piocher une tuile et passer mon tour\n");
    printf("3. Afficher mon chevalet\n");
    printf("4. Afficher la table\n");
    printf("5. Manipuler la table\n");
    printf("0. Quitter la partie\n");
    printf("---------------------------------\n");
}

/* ------------------------------------------------------------------------- */
int choisir_option_menu(void) {
    int choix;
    printf("Votre choix (0-5) : ");
    
    while(1) {
        if (scanf("%d", &choix) != 1) {
            printf("Entrée invalide. Réessayez : ");
            while(getchar() != '\n');
            continue;
        }
        
        if (choix >= 0 && choix <= 5) {
            getchar();
            return choix;
        }
        
        printf("Choix invalide. Entrez un nombre entre 0 et 5 : ");
    }
}

/* ------------------------------------------------------------------------- */
void executer_option(int choix, Joueur* j) {
    switch(choix) {
        case 0:
            printf("Merci d'avoir joué ! À bientôt.\n");
            exit(0);
            
        case 1:
            printf("\n>>> JOUER UNE COMBINAISON\n");
            jouer_combinaison(j);
            break;
            
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
            break;
            
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
            break;
            
        case 4:
            printf("\n>>> TABLE ACTUELLE\n");
            afficher_table("table.json");
            break;
            
        case 5:
            menu_manipulation(j);
            break;
    }
}

void menu_manipulation(Joueur* j) {
    printf("\n>>> MANIPULATION DE LA TABLE\n");
    
    // Fonction pour initialiser les fichiers virtuels
    void initialiser_virtuel(void) {
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
    
    // Initialiser
    initialiser_virtuel();
    
    // Boucle de manipulation
    while (1) {
        printf("\n=== TABLE VIRTUELLE ===\n");
        afficher_table("table_virtuelle.json");
        
        printf("\n=== VOTRE CHEVALET ===\n");
        FILE* f_chevalet = fopen("chevalet_virtuel.json", "r");
        if (f_chevalet) {
            fseek(f_chevalet, 0, SEEK_END);
            long fsize = ftell(f_chevalet);
            fseek(f_chevalet, 0, SEEK_SET);
            char* data = malloc(fsize + 1);
            fread(data, 1, fsize, f_chevalet);
            data[fsize] = 0;
            fclose(f_chevalet);
            
            cJSON* root = cJSON_Parse(data);
            free(data);
            if (root) {
                cJSON* tuiles_array = cJSON_GetObjectItem(root, "tuiles");
                if (tuiles_array) {
                    int nb_tuiles = cJSON_GetArraySize(tuiles_array);
                    for (int i = 0; i < nb_tuiles; i++) {
                        cJSON* tuile_json = cJSON_GetArrayItem(tuiles_array, i);
                        int id = cJSON_GetObjectItem(tuile_json, "id")->valueint;
                        int valeur = cJSON_GetObjectItem(tuile_json, "valeur")->valueint;
                        char couleur = cJSON_GetObjectItem(tuile_json, "couleur")->valuestring[0];
                        bool joker = cJSON_IsTrue(cJSON_GetObjectItem(tuile_json, "joker"));
                        
                        printf("%d: %d%c%s  ", id, valeur, couleur, joker ? " (J)" : "");
                    }
                    printf("\n");
                }
                cJSON_Delete(root);
            }
        }
        
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
                printf("\n🎉 Tour validé avec succès ! Retour au menu principal.\n");
                break;
            } else {
                printf("\n❌ Validation échouée. Voulez-vous :\n");
                printf("   1. Recommencer (recréer les fichiers virtuels)\n");
                printf("   2. Continuer à corriger\n");
                printf("Choix (1-2) : ");
                
                int choix_reprise;
                scanf("%d", &choix_reprise);
                getchar();
                
                if (choix_reprise == 1) {
                    // Supprimer et recréer
                    remove("table_virtuelle.json");
                    remove("chevalet_virtuel.json");
                    initialiser_virtuel();
                    printf("Fichiers virtuels recréés depuis l'état actuel.\n");
                }
                // Si choix == 2, on continue simplement
            }
            
        } else if (choix_manip == 5) {
            remove("table_virtuelle.json");
            remove("chevalet_virtuel.json");
            initialiser_virtuel();
            printf("Manipulation recommencée. Fichiers virtuels recréés.\n");
            
        } else if (choix_manip == 6) {
            remove("table_virtuelle.json");
            remove("chevalet_virtuel.json");
            printf("Manipulation abandonnée. Retour au menu principal.\n");
            break;
            
        } else {
            printf("Choix invalide\n");
        }
    }
}