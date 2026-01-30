#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <cjson/cJSON.h>
#include "table.h"
#include "struct.h"
#include "Tuile.h"
#include "joueur.h"  // Pour combinaison_valide

// Variables globales pour la table en mémoire
static cJSON* table_json = NULL;
static bool table_chargee = false;

/* ------------------------------------------------------------------------- */
static void charger_table_en_memoire(void) {
    if (table_chargee) return;
    
    FILE* f = fopen("table.json", "r");
    if (!f) {
        table_json = cJSON_CreateArray();
        table_chargee = true;
        return;
    }
    
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *data = malloc(fsize + 1);
    fread(data, 1, fsize, f);
    data[fsize] = 0;
    fclose(f);
    
    table_json = cJSON_Parse(data);
    free(data);
    
    if (!table_json) {
        table_json = cJSON_CreateArray();
    }
    
    table_chargee = true;
}

/* ------------------------------------------------------------------------- */
static void sauvegarder_table_depuis_memoire(void) {
    if (!table_chargee) return;
    
    char *str = cJSON_Print(table_json);
    FILE* f = fopen("table.json", "w");
    if (f) {
        fprintf(f, "%s", str);
        fclose(f);
    }
    free(str);
}

/* ------------------------------------------------------------------------- */
static void liberer_table_memoire(void) {
    if (table_json) {
        cJSON_Delete(table_json);
        table_json = NULL;
    }
    table_chargee = false;
}

/* ------------------------------------------------------------------------- */
void creer_table(void) {
    liberer_table_memoire();
    table_json = cJSON_CreateArray();
    table_chargee = true;
    sauvegarder_table_depuis_memoire();
}

/* ------------------------------------------------------------------------- */
// Afficher toutes les combinaisons sur la table
void afficher_combinaisons_table(void) {
    charger_table_en_memoire();
    
    int nb_combinaisons = cJSON_GetArraySize(table_json);
    
    printf("\n=== TABLE DE JEU ===\n");
    
    if (nb_combinaisons == 0) {
        printf("Aucune combinaison sur la table.\n");
        return;
    }
    
    for (int i = 0; i < nb_combinaisons; i++) {
        cJSON* combinaison = cJSON_GetArrayItem(table_json, i);
        int nb_tuiles = cJSON_GetArraySize(combinaison);
        
        printf("[%d] ", i);
        for (int j = 0; j < nb_tuiles; j++) {
            cJSON* tuile = cJSON_GetArrayItem(combinaison, j);
            int valeur = cJSON_GetObjectItem(tuile, "valeur")->valueint;
            char couleur = cJSON_GetObjectItem(tuile, "couleur")->valuestring[0];
            bool joker = cJSON_IsTrue(cJSON_GetObjectItem(tuile, "joker"));
            
            if (joker) {
                printf("[J] ");
            } else {
                printf("%d%c ", valeur, couleur);
            }
        }
        printf("\n");
    }
    printf("===================\n");
}

/* ------------------------------------------------------------------------- */
// Récupérer un joker
bool peut_recuperer_joker(Tuile* tuile_joueur, int* comb_index, int* tuile_index) {
    charger_table_en_memoire();
    
    int nb_combinaisons = cJSON_GetArraySize(table_json);
    
    for (int i = 0; i < nb_combinaisons; i++) {
        cJSON* combinaison = cJSON_GetArrayItem(table_json, i);
        int nb_tuiles = cJSON_GetArraySize(combinaison);
        
        for (int j = 0; j < nb_tuiles; j++) {
            cJSON* tuile_json = cJSON_GetArrayItem(combinaison, j);
            if (!tuile_json) continue;
            
            cJSON* joker_field = cJSON_GetObjectItem(tuile_json, "joker");
            if (!joker_field) continue;
            
            bool est_joker = cJSON_IsTrue(joker_field);
            
            if (est_joker) {
                // Vérifier si la tuile du joueur peut remplacer ce joker
                if (!tuile_joueur->joker) {
                    // Créer un tableau de tuiles temporaire
                    Tuile tuiles_temp[MAX_TUILES];
                    
                    // Copier toutes les tuiles de la combinaison
                    for (int k = 0; k < nb_tuiles; k++) {
                        cJSON* t = cJSON_GetArrayItem(combinaison, k);
                        if (!t) continue;
                        
                        tuiles_temp[k].id = cJSON_GetObjectItem(t, "id")->valueint;
                        tuiles_temp[k].valeur = cJSON_GetObjectItem(t, "valeur")->valueint;
                        tuiles_temp[k].couleur = cJSON_GetObjectItem(t, "couleur")->valuestring[0];
                        tuiles_temp[k].joker = cJSON_IsTrue(cJSON_GetObjectItem(t, "joker"));
                    }
                    
                    // Remplacer le joker par la tuile du joueur
                    tuiles_temp[j] = *tuile_joueur;
                    
                    // Vérifier si la combinaison est toujours valide
                    if (combinaison_valide(tuiles_temp, nb_tuiles)) {
                        *comb_index = i;
                        *tuile_index = j;
                        return true;
                    }
                }
            }
        }
    }
    
    return false;
}


bool recuperer_joker(Tuile* tuile_joueur, int comb_index, int tuile_index, Tuile* joker_recupere) {
    charger_table_en_memoire();
    
    cJSON* combinaison = cJSON_GetArrayItem(table_json, comb_index);
    if (!combinaison) {
        printf("DEBUG: Combinaison %d non trouvée\n", comb_index);
        return false;
    }
    
    cJSON* tuile_json = cJSON_GetArrayItem(combinaison, tuile_index);
    if (!tuile_json) {
        printf("DEBUG: Tuile %d non trouvée dans combinaison %d\n", tuile_index, comb_index);
        return false;
    }
    
    // Vérifier que c'est bien un joker
    cJSON* joker_field = cJSON_GetObjectItem(tuile_json, "joker");
    if (!joker_field || !cJSON_IsTrue(joker_field)) {
        printf("DEBUG: La tuile n'est pas un joker\n");
        return false;
    }
    
    // Récupérer le joker (FAIRE UNE COPIE des données d'abord)
    joker_recupere->id = cJSON_GetObjectItem(tuile_json, "id")->valueint;
    joker_recupere->valeur = 0;
    joker_recupere->couleur = 'J';
    joker_recupere->joker = true;
    
    // SAUVEGARDER L'ANCIENNE TUILE (en créant une copie)
    cJSON* ancienne_tuile_copie = cJSON_Duplicate(tuile_json, 1);
    
    // Créer la nouvelle tuile (remplacement du joker)
    cJSON* nouvelle_tuile = cJSON_CreateObject();
    cJSON_AddNumberToObject(nouvelle_tuile, "id", tuile_joueur->id);
    cJSON_AddNumberToObject(nouvelle_tuile, "valeur", tuile_joueur->valeur);
    char couleur_str[2] = {tuile_joueur->couleur, 0};
    cJSON_AddStringToObject(nouvelle_tuile, "couleur", couleur_str);
    cJSON_AddBoolToObject(nouvelle_tuile, "joker", tuile_joueur->joker);
    
    // Remplacer le joker par la nouvelle tuile
    cJSON_ReplaceItemInArray(combinaison, tuile_index, nouvelle_tuile);
    
    // VÉRIFICATION : La combinaison est-elle toujours valide ?
    if (!est_combinaison_valide(comb_index)) {
        printf("DEBUG: Combinaison invalide après remplacement. Annulation.\n");
        // RESTAURER L'ANCIENNE TUILE dans la table en mémoire
        cJSON_ReplaceItemInArray(combinaison, tuile_index, ancienne_tuile_copie);
        // La nouvelle tuile n'est plus utilisée
        cJSON_Delete(nouvelle_tuile);
        return false;
    }
    
    // Si tout est bon, nettoyer l'ancienne copie
    cJSON_Delete(ancienne_tuile_copie);
    
    // Sauvegarder seulement si tout est bon
    sauvegarder_table_depuis_memoire();
    
    printf("DEBUG: Remplacement effectué et validé\n");
    return true;
}

/* ------------------------------------------------------------------------- */
// Étendre une suite
bool peut_etendre_suite(Tuile* tuile_joueur, int comb_index, bool gauche, Tuile* tuile_a_recuperer) {
    charger_table_en_memoire();
    
    if (!est_combinaison_suite(comb_index)) {
        return false;
    }
    
    cJSON* combinaison = cJSON_GetArrayItem(table_json, comb_index);
    int nb_tuiles = cJSON_GetArraySize(combinaison);
    
    if (gauche) {
        // Vérifier si on peut ajouter à gauche
        cJSON* premiere_tuile = cJSON_GetArrayItem(combinaison, 0);
        int valeur = cJSON_GetObjectItem(premiere_tuile, "valeur")->valueint;
        char couleur = cJSON_GetObjectItem(premiere_tuile, "couleur")->valuestring[0];
        
        if (tuile_joueur->couleur == couleur && 
            tuile_joueur->valeur == valeur - 1) {
            // Récupérer la dernière tuile
            cJSON* derniere_tuile = cJSON_GetArrayItem(combinaison, nb_tuiles - 1);
            tuile_a_recuperer->id = cJSON_GetObjectItem(derniere_tuile, "id")->valueint;
            tuile_a_recuperer->valeur = cJSON_GetObjectItem(derniere_tuile, "valeur")->valueint;
            tuile_a_recuperer->couleur = cJSON_GetObjectItem(derniere_tuile, "couleur")->valuestring[0];
            tuile_a_recuperer->joker = cJSON_IsTrue(cJSON_GetObjectItem(derniere_tuile, "joker"));
            return true;
        }
    } else {
        // Vérifier si on peut ajouter à droite
        cJSON* derniere_tuile = cJSON_GetArrayItem(combinaison, nb_tuiles - 1);
        int valeur = cJSON_GetObjectItem(derniere_tuile, "valeur")->valueint;
        char couleur = cJSON_GetObjectItem(derniere_tuile, "couleur")->valuestring[0];
        
        if (tuile_joueur->couleur == couleur && 
            tuile_joueur->valeur == valeur + 1) {
            // Récupérer la première tuile
            cJSON* premiere_tuile = cJSON_GetArrayItem(combinaison, 0);
            tuile_a_recuperer->id = cJSON_GetObjectItem(premiere_tuile, "id")->valueint;
            tuile_a_recuperer->valeur = cJSON_GetObjectItem(premiere_tuile, "valeur")->valueint;
            tuile_a_recuperer->couleur = cJSON_GetObjectItem(premiere_tuile, "couleur")->valuestring[0];
            tuile_a_recuperer->joker = cJSON_IsTrue(cJSON_GetObjectItem(premiere_tuile, "joker"));
            return true;
        }
    }
    
    return false;
}

bool etendre_suite(Tuile* tuile_joueur, int comb_index, bool gauche, Tuile* tuile_recuperee) {
    // D'abord, vérifier si l'extension est possible et savoir quelle tuile on récupérera
    Tuile tuile_a_recuperer_temp;
    if (!peut_etendre_suite(tuile_joueur, comb_index, gauche, &tuile_a_recuperer_temp)) {
        return false;
    }
    
    // Sauvegarder la tuile qu'on va récupérer
    *tuile_recuperee = tuile_a_recuperer_temp;
    
    charger_table_en_memoire();
    cJSON* combinaison = cJSON_GetArrayItem(table_json, comb_index);
    
    // Ajouter la nouvelle tuile
    cJSON* nouvelle_tuile = cJSON_CreateObject();
    cJSON_AddNumberToObject(nouvelle_tuile, "id", tuile_joueur->id);
    cJSON_AddNumberToObject(nouvelle_tuile, "valeur", tuile_joueur->valeur);
    char couleur_str[2] = {tuile_joueur->couleur, 0};
    cJSON_AddStringToObject(nouvelle_tuile, "couleur", couleur_str);
    cJSON_AddBoolToObject(nouvelle_tuile, "joker", tuile_joueur->joker);
    
    if (gauche) {
        // Ajouter au début
        cJSON_InsertItemInArray(combinaison, 0, nouvelle_tuile);
        // Supprimer la dernière (celle qu'on récupère)
        cJSON_DeleteItemFromArray(combinaison, cJSON_GetArraySize(combinaison) - 1);
    } else {
        // Ajouter à la fin
        cJSON_AddItemToArray(combinaison, nouvelle_tuile);
        // Supprimer la première (celle qu'on récupère)
        cJSON_DeleteItemFromArray(combinaison, 0);
    }
    
    // Vérifier que la combinaison est toujours valide
    if (!est_combinaison_valide(comb_index)) {
        // Annuler l'opération
        // (Devrait restaurer l'état précédent, mais complexe)
        return false;
    }
    
    sauvegarder_table_depuis_memoire();
    return true;
}

/* ------------------------------------------------------------------------- */
// Remplacer une tuile
bool peut_remplacer_tuile(Tuile* tuile_joueur, int comb_index, int tuile_index) {
    charger_table_en_memoire();
    
    cJSON* combinaison = cJSON_GetArrayItem(table_json, comb_index);
    if (!combinaison) return false;
    
    int nb_tuiles = cJSON_GetArraySize(combinaison);
    if (tuile_index < 0 || tuile_index >= nb_tuiles) {
        return false;
    }
    
    // Vérifier si le remplacement garde la combinaison valide
    // Pour l'instant, on accepte tout
    return true;
}

bool remplacer_tuile(Tuile* tuile_joueur, int comb_index, int tuile_index, Tuile* ancienne_tuile) {
    if (!peut_remplacer_tuile(tuile_joueur, comb_index, tuile_index)) {
        return false;
    }
    
    charger_table_en_memoire();
    cJSON* combinaison = cJSON_GetArrayItem(table_json, comb_index);
    
    // Récupérer l'ancienne tuile
    cJSON* tuile_json = cJSON_GetArrayItem(combinaison, tuile_index);
    ancienne_tuile->id = cJSON_GetObjectItem(tuile_json, "id")->valueint;
    ancienne_tuile->valeur = cJSON_GetObjectItem(tuile_json, "valeur")->valueint;
    ancienne_tuile->couleur = cJSON_GetObjectItem(tuile_json, "couleur")->valuestring[0];
    ancienne_tuile->joker = cJSON_IsTrue(cJSON_GetObjectItem(tuile_json, "joker"));
    
    // Créer la nouvelle tuile
    cJSON* nouvelle_tuile = cJSON_CreateObject();
    cJSON_AddNumberToObject(nouvelle_tuile, "id", tuile_joueur->id);
    cJSON_AddNumberToObject(nouvelle_tuile, "valeur", tuile_joueur->valeur);
    char couleur_str[2] = {tuile_joueur->couleur, 0};
    cJSON_AddStringToObject(nouvelle_tuile, "couleur", couleur_str);
    cJSON_AddBoolToObject(nouvelle_tuile, "joker", tuile_joueur->joker);
    
    // Remplacer
    cJSON_ReplaceItemInArray(combinaison, tuile_index, nouvelle_tuile);
    
    // Vérifier la validité
    if (!est_combinaison_valide(comb_index)) {
        // Annuler
        cJSON_ReplaceItemInArray(combinaison, tuile_index, tuile_json);
        return false;
    }
    
    sauvegarder_table_depuis_memoire();
    return true;
}

/* ------------------------------------------------------------------------- */
// Diviser une suite
bool peut_diviser_suite(int comb_index, int position) {
    charger_table_en_memoire();
    
    if (!est_combinaison_suite(comb_index)) {
        return false;
    }
    
    int nb_tuiles = compter_tuiles_combinaison(comb_index);
    
    // position est l'index où couper (0 < position < nb_tuiles-1)
    // Chaque partie doit avoir au moins 3 tuiles
    if (position < 2 || position > nb_tuiles - 3) {
        return false;
    }
    
    return true;
}

bool diviser_suite(int comb_index, int position) {
    if (!peut_diviser_suite(comb_index, position)) {
        return false;
    }
    
    charger_table_en_memoire();
    cJSON* combinaison = cJSON_GetArrayItem(table_json, comb_index);
    int nb_tuiles = cJSON_GetArraySize(combinaison);
    
    // Créer une nouvelle combinaison avec la seconde partie
    cJSON* nouvelle_combinaison = cJSON_CreateArray();
    
    // Déplacer les tuiles de position à nb_tuiles-1 vers la nouvelle combinaison
    for (int i = position; i < nb_tuiles; i++) {
        cJSON* tuile = cJSON_GetArrayItem(combinaison, position);
        cJSON_AddItemToArray(nouvelle_combinaison, tuile);
        cJSON_DeleteItemFromArray(combinaison, position);
    }
    
    // Ajouter la nouvelle combinaison à la table
    cJSON_AddItemToArray(table_json, nouvelle_combinaison);
    
    // Vérifier que les deux combinaisons sont valides
    if (!est_combinaison_valide(comb_index) || 
        !est_combinaison_valide(cJSON_GetArraySize(table_json) - 1)) {
        // Annuler (plus complexe)
        return false;
    }
    
    sauvegarder_table_depuis_memoire();
    return true;
}

/* ------------------------------------------------------------------------- */
// Retirer une tuile
bool peut_retirer_tuile(int comb_index, int tuile_index) {
    charger_table_en_memoire();
    
    int nb_tuiles = compter_tuiles_combinaison(comb_index);
    
    // Peut retirer seulement si la combinaison a plus de 3 tuiles
    if (nb_tuiles <= 3) {
        return false;
    }
    
    // Vérifier que la combinaison reste valide après retrait
    // Pour l'instant, on accepte tout sauf pour les brelans
    if (est_combinaison_brelan(comb_index)) {
        // Pour un brelan, retirer une tuile le rend invalide (< 3 tuiles de même valeur)
        return false;
    }
    
    return true;
}

bool retirer_tuile(int comb_index, int tuile_index, Tuile* tuile_retiree) {
    if (!peut_retirer_tuile(comb_index, tuile_index)) {
        return false;
    }
    
    charger_table_en_memoire();
    cJSON* combinaison = cJSON_GetArrayItem(table_json, comb_index);
    
    // Récupérer la tuile
    cJSON* tuile_json = cJSON_GetArrayItem(combinaison, tuile_index);
    tuile_retiree->id = cJSON_GetObjectItem(tuile_json, "id")->valueint;
    tuile_retiree->valeur = cJSON_GetObjectItem(tuile_json, "valeur")->valueint;
    tuile_retiree->couleur = cJSON_GetObjectItem(tuile_json, "couleur")->valuestring[0];
    tuile_retiree->joker = cJSON_IsTrue(cJSON_GetObjectItem(tuile_json, "joker"));
    
    // Retirer la tuile
    cJSON_DeleteItemFromArray(combinaison, tuile_index);
    
    // Vérifier que la combinaison est toujours valide
    if (!est_combinaison_valide(comb_index)) {
        // Annuler (devrait rajouter la tuile)
        return false;
    }
    
    sauvegarder_table_depuis_memoire();
    return true;
}

/* ------------------------------------------------------------------------- */
// Fonctions utilitaires
int compter_combinaisons_table(void) {
    charger_table_en_memoire();
    return cJSON_GetArraySize(table_json);
}

bool est_combinaison_valide(int comb_index) {
    charger_table_en_memoire();
    
    cJSON* combinaison = cJSON_GetArrayItem(table_json, comb_index);
    if (!combinaison) return false;
    
    int nb_tuiles = cJSON_GetArraySize(combinaison);
    if (nb_tuiles < 3) return false;
    
    // Extraire les tuiles
    Tuile tuiles[MAX_TUILES];
    for (int i = 0; i < nb_tuiles; i++) {
        cJSON* tuile_json = cJSON_GetArrayItem(combinaison, i);
        tuiles[i].id = cJSON_GetObjectItem(tuile_json, "id")->valueint;
        tuiles[i].valeur = cJSON_GetObjectItem(tuile_json, "valeur")->valueint;
        tuiles[i].couleur = cJSON_GetObjectItem(tuile_json, "couleur")->valuestring[0];
        tuiles[i].joker = cJSON_IsTrue(cJSON_GetObjectItem(tuile_json, "joker"));
    }
    
    // Utiliser ta fonction combinaison_valide
    return combinaison_valide(tuiles, nb_tuiles);
}

bool est_combinaison_brelan(int comb_index) {
    charger_table_en_memoire();
    
    cJSON* combinaison = cJSON_GetArrayItem(table_json, comb_index);
    if (!combinaison) return false;
    
    int nb_tuiles = cJSON_GetArraySize(combinaison);
    if (nb_tuiles < 3) return false;
    
    int valeur = -1;
    for (int i = 0; i < nb_tuiles; i++) {
        cJSON* tuile_json = cJSON_GetArrayItem(combinaison, i);
        bool est_joker = cJSON_IsTrue(cJSON_GetObjectItem(tuile_json, "joker"));
        
        if (!est_joker) {
            int val = cJSON_GetObjectItem(tuile_json, "valeur")->valueint;
            if (valeur == -1) {
                valeur = val;
            } else if (val != valeur) {
                return false;
            }
        }
    }
    
    return true;
}

bool est_combinaison_suite(int comb_index) {
    charger_table_en_memoire();
    
    cJSON* combinaison = cJSON_GetArrayItem(table_json, comb_index);
    if (!combinaison) return false;
    
    int nb_tuiles = cJSON_GetArraySize(combinaison);
    if (nb_tuiles < 3) return false;
    
    char couleur = '\0';
    int valeurs[nb_tuiles];
    int jokers = 0;
    
    for (int i = 0; i < nb_tuiles; i++) {
        cJSON* tuile_json = cJSON_GetArrayItem(combinaison, i);
        bool est_joker = cJSON_IsTrue(cJSON_GetObjectItem(tuile_json, "joker"));
        
        if (est_joker) {
            jokers++;
            valeurs[i] = -1; // Marqueur pour joker
        } else {
            if (couleur == '\0') {
                couleur = cJSON_GetObjectItem(tuile_json, "couleur")->valuestring[0];
            } else if (cJSON_GetObjectItem(tuile_json, "couleur")->valuestring[0] != couleur) {
                return false;
            }
            valeurs[i] = cJSON_GetObjectItem(tuile_json, "valeur")->valueint;
        }
    }
    
    // Trier les valeurs (sans les jokers)
    int valeurs_triees[nb_tuiles - jokers];
    int idx = 0;
    for (int i = 0; i < nb_tuiles; i++) {
        if (valeurs[i] != -1) {
            valeurs_triees[idx++] = valeurs[i];
        }
    }
    
    // Trier le tableau
    for (int i = 0; i < idx - 1; i++) {
        for (int j = i + 1; j < idx; j++) {
            if (valeurs_triees[i] > valeurs_triees[j]) {
                int tmp = valeurs_triees[i];
                valeurs_triees[i] = valeurs_triees[j];
                valeurs_triees[j] = tmp;
            }
        }
    }
    
    // Vérifier les doublons
    for (int i = 0; i < idx - 1; i++) {
        if (valeurs_triees[i] == valeurs_triees[i + 1]) {
            return false;
        }
    }
    
    // Vérifier les gaps
    int gaps = 0;
    for (int i = 0; i < idx - 1; i++) {
        gaps += valeurs_triees[i + 1] - valeurs_triees[i] - 1;
    }
    
    return (gaps <= jokers);
}

int compter_tuiles_combinaison(int comb_index) {
    charger_table_en_memoire();
    
    cJSON* combinaison = cJSON_GetArrayItem(table_json, comb_index);
    if (!combinaison) return 0;
    
    return cJSON_GetArraySize(combinaison);
}

void obtenir_tuile_table(int comb_index, int tuile_index, Tuile* resultat) {
    charger_table_en_memoire();
    
    cJSON* combinaison = cJSON_GetArrayItem(table_json, comb_index);
    if (!combinaison) return;
    
    cJSON* tuile_json = cJSON_GetArrayItem(combinaison, tuile_index);
    if (!tuile_json) return;
    
    resultat->id = cJSON_GetObjectItem(tuile_json, "id")->valueint;
    resultat->valeur = cJSON_GetObjectItem(tuile_json, "valeur")->valueint;
    resultat->couleur = cJSON_GetObjectItem(tuile_json, "couleur")->valuestring[0];
    resultat->joker = cJSON_IsTrue(cJSON_GetObjectItem(tuile_json, "joker"));
}

bool validation_table_complete(void) {
    charger_table_en_memoire();
    
    int nb_combinaisons = cJSON_GetArraySize(table_json);
    
    for (int i = 0; i < nb_combinaisons; i++) {
        if (!est_combinaison_valide(i)) {
            return false;
        }
    }
    
    return true;
}

bool peut_ajouter_tuile_combinaison(Tuile* tuile, int comb_index) {
    charger_table_en_memoire();
    
    cJSON* combinaison = cJSON_GetArrayItem(table_json, comb_index);
    if (!combinaison) return false;
    
    int nb_tuiles = cJSON_GetArraySize(combinaison);
    if (nb_tuiles < 3) return false;
    
    // Créer un tableau temporaire avec toutes les tuiles + la nouvelle
    Tuile tuiles_temp[MAX_TUILES];
    
    // Copier les tuiles existantes
    for (int i = 0; i < nb_tuiles; i++) {
        cJSON* tuile_json = cJSON_GetArrayItem(combinaison, i);
        tuiles_temp[i].id = cJSON_GetObjectItem(tuile_json, "id")->valueint;
        tuiles_temp[i].valeur = cJSON_GetObjectItem(tuile_json, "valeur")->valueint;
        tuiles_temp[i].couleur = cJSON_GetObjectItem(tuile_json, "couleur")->valuestring[0];
        tuiles_temp[i].joker = cJSON_IsTrue(cJSON_GetObjectItem(tuile_json, "joker"));
    }
    
    // Ajouter la nouvelle tuile
    tuiles_temp[nb_tuiles] = *tuile;
    
    // Vérifier si la combinaison est toujours valide
    return combinaison_valide(tuiles_temp, nb_tuiles + 1);
}

bool ajouter_tuile_combinaison(Tuile* tuile, int comb_index) {
    if (!peut_ajouter_tuile_combinaison(tuile, comb_index)) {
        return false;
    }
    
    charger_table_en_memoire();
    cJSON* combinaison = cJSON_GetArrayItem(table_json, comb_index);
    int nb_tuiles = cJSON_GetArraySize(combinaison);
    
    // Créer l'objet JSON pour la nouvelle tuile
    cJSON* nouvelle_tuile = cJSON_CreateObject();
    cJSON_AddNumberToObject(nouvelle_tuile, "id", tuile->id);
    cJSON_AddNumberToObject(nouvelle_tuile, "valeur", tuile->valeur);
    char couleur_str[2] = {tuile->couleur, 0};
    cJSON_AddStringToObject(nouvelle_tuile, "couleur", couleur_str);
    cJSON_AddBoolToObject(nouvelle_tuile, "joker", tuile->joker);
    
    // DÉTERMINER LA POSITION D'INSERTION
    int position_insertion = nb_tuiles; // Par défaut à la fin
    
    if (est_combinaison_suite(comb_index) && !tuile->joker) {
        // Pour une suite : trouver où insérer pour garder l'ordre
        // 1. Si la tuile est plus petite que la première → insérer au début
        cJSON* premiere_json = cJSON_GetArrayItem(combinaison, 0);
        int premiere_valeur = cJSON_GetObjectItem(premiere_json, "valeur")->valueint;
        char premiere_couleur = cJSON_GetObjectItem(premiere_json, "couleur")->valuestring[0];
        
        if (tuile->couleur == premiere_couleur && tuile->valeur < premiere_valeur) {
            position_insertion = 0;
        }
        // 2. Sinon, trouver la bonne position
        else {
            for (int i = 0; i < nb_tuiles - 1; i++) {
                cJSON* tuile_i = cJSON_GetArrayItem(combinaison, i);
                cJSON* tuile_i1 = cJSON_GetArrayItem(combinaison, i + 1);
                
                int val_i = cJSON_GetObjectItem(tuile_i, "valeur")->valueint;
                int val_i1 = cJSON_GetObjectItem(tuile_i1, "valeur")->valueint;
                char couleur_i = cJSON_GetObjectItem(tuile_i, "couleur")->valuestring[0];
                
                if (tuile->couleur == couleur_i && 
                    tuile->valeur > val_i && tuile->valeur < val_i1) {
                    position_insertion = i + 1;
                    break;
                }
            }
        }
    }
    
    // Insérer à la bonne position
    if (position_insertion == nb_tuiles) {
        cJSON_AddItemToArray(combinaison, nouvelle_tuile);
    } else {
        cJSON_InsertItemInArray(combinaison, position_insertion, nouvelle_tuile);
    }
    
    // Vérifier que la combinaison est toujours valide
    if (!est_combinaison_valide(comb_index)) {
        // Annuler
        cJSON_DeleteItemFromArray(combinaison, position_insertion);
        return false;
    }
    
    sauvegarder_table_depuis_memoire();
    return true;
}


bool diviser_suite_avec_ajout(int comb_index, int position, Tuile* tuile_ajout, bool ajouter_a_premiere) {
    charger_table_en_memoire();
    
    cJSON* combinaison = cJSON_GetArrayItem(table_json, comb_index);
    if (!combinaison) {
        printf("DEBUG: Combinaison %d non trouvée\n", comb_index);
        return false;
    }
    
    int nb_tuiles = cJSON_GetArraySize(combinaison);
    printf("DEBUG: Division de %d tuiles à position %d\n", nb_tuiles, position);
    
    // Vérifier que position est valide
    if (position < 0 || position >= nb_tuiles - 1) {
        printf("DEBUG: Position invalide\n");
        return false;
    }
    
    // Créer la nouvelle tuile JSON
    cJSON* nouvelle_tuile = cJSON_CreateObject();
    cJSON_AddNumberToObject(nouvelle_tuile, "id", tuile_ajout->id);
    cJSON_AddNumberToObject(nouvelle_tuile, "valeur", tuile_ajout->valeur);
    char couleur_str[2] = {tuile_ajout->couleur, 0};
    cJSON_AddStringToObject(nouvelle_tuile, "couleur", couleur_str);
    cJSON_AddBoolToObject(nouvelle_tuile, "joker", tuile_ajout->joker);
    
    // Créer la deuxième combinaison
    cJSON* nouvelle_combinaison = cJSON_CreateArray();
    
    // Déplacer les tuiles après 'position' vers la nouvelle combinaison
    // On déplace (nb_tuiles - position - 1) éléments
    int elements_a_deplacer = nb_tuiles - position - 1;
    printf("DEBUG: À déplacer : %d éléments\n", elements_a_deplacer);
    
    for (int i = 0; i < elements_a_deplacer; i++) {
        // Toujours prendre l'élément à position+1 (car on supprime au fur et à mesure)
        cJSON* tuile = cJSON_GetArrayItem(combinaison, position + 1);
        if (!tuile) {
            printf("DEBUG: Élément %d non trouvé\n", position + 1);
            return false;
        }
        
        // Dupliquer l'élément avant de l'ajouter
        cJSON* tuile_copie = cJSON_Duplicate(tuile, 1);
        cJSON_AddItemToArray(nouvelle_combinaison, tuile_copie);
        
        // Supprimer l'original
        cJSON_DeleteItemFromArray(combinaison, position + 1);
    }
    
    // Ajouter la nouvelle tuile à la bonne combinaison
    if (ajouter_a_premiere) {
        // Ajouter à la fin de la première partie
        cJSON_AddItemToArray(combinaison, nouvelle_tuile);
        printf("DEBUG: Tuile ajoutée à la première partie\n");
    } else {
        // Ajouter au début de la deuxième partie
        cJSON_InsertItemInArray(nouvelle_combinaison, 0, nouvelle_tuile);
        printf("DEBUG: Tuile ajoutée à la deuxième partie\n");
    }
    
    // Ajouter la nouvelle combinaison à la table
    cJSON_AddItemToArray(table_json, nouvelle_combinaison);
    
    printf("DEBUG: Vérification des combinaisons...\n");
    
    // Vérifier que les deux combinaisons sont valides
    if (!est_combinaison_valide(comb_index)) {
        printf("DEBUG: Première combinaison invalide\n");
        return false;
    }
    
    int nouvelle_index = cJSON_GetArraySize(table_json) - 1;
    if (!est_combinaison_valide(nouvelle_index)) {
        printf("DEBUG: Deuxième combinaison invalide\n");
        return false;
    }
    
    sauvegarder_table_depuis_memoire();
    printf("DEBUG: Division réussie\n");
    return true;
}