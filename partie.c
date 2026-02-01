// partie.c
#include <stdio.h>
#include <stdlib.h>
#include <cjson/cJSON.h>
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
    
    int joueur_actuel = 0;
    for (int i = 0; i < nb_joueurs; i++) {
        charger_joueur(&players[i]);
        if (players[i].tour) {
            joueur_actuel = i;
            break;
        }
    }
    
    printf("C'est à %s de jouer.\n", players[joueur_actuel].pseudo);
    
    while (1) {
        // VÉRIFIER SI PIOCHE VIDE
        FILE* f_pioche = fopen("pioche.json", "r");
        if (f_pioche) {
            fseek(f_pioche, 0, SEEK_END);
            long fsize = ftell(f_pioche);
            fclose(f_pioche);
            
            if (fsize <= 2) {  // Pioche vide = "[]"
                printf("\n⚠️  LA PIOCHE EST VIDE !\n");
                
                // Trouver le gagnant (score restant le plus bas)
                int gagnant_index = 0;
                int score_min = 9999;
                int scores_bruts[nb_joueurs];
                
                for (int i = 0; i < nb_joueurs; i++) {
                    Tuile tuiles[MAX_TUILES];
                    int nb_tuiles = 0;
                    charger_chevalet(players[i].chevalet, tuiles, &nb_tuiles);
                    
                    scores_bruts[i] = 0;
                    for (int k = 0; k < nb_tuiles; k++) {
                        scores_bruts[i] += tuiles[k].joker ? 30 : tuiles[k].valeur;
                    }
                    
                    if (scores_bruts[i] < score_min) {
                        score_min = scores_bruts[i];
                        gagnant_index = i;
                    }
                }
                
                printf("%s gagne avec le score restant le plus bas (%d points) !\n", 
                       players[gagnant_index].pseudo, score_min);
                
                // Sauvegarder scores dans fichier
                cJSON *root = cJSON_CreateObject();
                int score_gagnant = 0;
                
                for (int i = 0; i < nb_joueurs; i++) {
                    if (i == gagnant_index) {
                        // Score gagnant = somme des différences
                        int total = 0;
                        for (int j = 0; j < nb_joueurs; j++) {
                            if (j != gagnant_index) {
                                total += scores_bruts[j] - score_min;
                            }
                        }
                        cJSON_AddNumberToObject(root, players[i].pseudo, total);
                        score_gagnant = total;
                    } else {
                        // Score perdant = -(score_brut - score_min)
                        cJSON_AddNumberToObject(root, players[i].pseudo, -(scores_bruts[i] - score_min));
                    }
                }
                
                char *json_str = cJSON_Print(root);
                FILE *f = fopen("scores.json", "w");
                if (f) {
                    fprintf(f, "%s", json_str);
                    fclose(f);
                }
                free(json_str);
                cJSON_Delete(root);
                
                // Afficher scores
                printf("\n=== SCORES FINAUX (pioche vide) ===\n");
                FILE *f_scores = fopen("scores.json", "r");
                if (f_scores) {
                    fseek(f_scores, 0, SEEK_END);
                    long fsize = ftell(f_scores);
                    fseek(f_scores, 0, SEEK_SET);
                    char *data = malloc(fsize + 1);
                    fread(data, 1, fsize, f_scores);
                    data[fsize] = 0;
                    fclose(f_scores);
                    
                    cJSON *scores_json = cJSON_Parse(data);
                    free(data);
                    if (scores_json) {
                        cJSON *item = scores_json->child;
                        while (item) {
                            printf("%s : %d points\n", item->string, item->valueint);
                            item = item->next;
                        }
                        cJSON_Delete(scores_json);
                    }
                }
                
                free(players);
                return;
            }
        }
        
        // TOUR NORMAL
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
                // VÉRIFIER SI LE JOUEUR A GAGNÉ
                if (a_fini(*j)) {
                    printf("\n🎉 FÉLICITATIONS ! %s a gagné la partie !\n", j->pseudo);
                    printf("Il ne lui reste plus de tuiles !\n");
                    
                    // Sauvegarder scores dans fichier
                    cJSON *root = cJSON_CreateObject();
                    int score_total_gagnant = 0;
                    
                    for (int i = 0; i < nb_joueurs; i++) {
                        Tuile tuiles[MAX_TUILES];
                        int nb_tuiles = 0;
                        charger_chevalet(players[i].chevalet, tuiles, &nb_tuiles);
                        
                        int score = 0;
                        for (int k = 0; k < nb_tuiles; k++) {
                            score += tuiles[k].joker ? 30 : tuiles[k].valeur;
                        }
                        
                        if (i == joueur_actuel) {
                            // Le gagnant: somme des scores des autres
                            int somme_autres = 0;
                            for (int j = 0; j < nb_joueurs; j++) {
                                if (j != joueur_actuel) {
                                    Tuile tuiles_autre[MAX_TUILES];
                                    int nb_tuiles_autre = 0;
                                    charger_chevalet(players[j].chevalet, tuiles_autre, &nb_tuiles_autre);
                                    
                                    for (int k = 0; k < nb_tuiles_autre; k++) {
                                        somme_autres += tuiles_autre[k].joker ? 30 : tuiles_autre[k].valeur;
                                    }
                                }
                            }
                            cJSON_AddNumberToObject(root, players[i].pseudo, somme_autres);
                            score_total_gagnant = somme_autres;
                        } else {
                            // Perdants: leur score en négatif
                            cJSON_AddNumberToObject(root, players[i].pseudo, -score);
                        }
                    }
                    
                    char *json_str = cJSON_Print(root);
                    FILE *f = fopen("scores.json", "w");
                    if (f) {
                        fprintf(f, "%s", json_str);
                        fclose(f);
                    }
                    free(json_str);
                    cJSON_Delete(root);
                    
                    // Afficher scores
                    printf("\n=== SCORES FINAUX ===\n");
                    FILE *f_scores = fopen("scores.json", "r");
                    if (f_scores) {
                        fseek(f_scores, 0, SEEK_END);
                        long fsize = ftell(f_scores);
                        fseek(f_scores, 0, SEEK_SET);
                        char *data = malloc(fsize + 1);
                        fread(data, 1, fsize, f_scores);
                        data[fsize] = 0;
                        fclose(f_scores);
                        
                        cJSON *scores_json = cJSON_Parse(data);
                        free(data);
                        if (scores_json) {
                            cJSON *item = scores_json->child;
                            while (item) {
                                printf("%s : %d points\n", item->string, item->valueint);
                                item = item->next;
                            }
                            cJSON_Delete(scores_json);
                        }
                    }
                    
                    free(players);
                    return;
                }
                
                int next = (joueur_actuel + 1) % nb_joueurs;
                passer_tour(j, &players[next]);
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