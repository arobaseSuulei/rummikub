#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <cjson/cJSON.h>
#include "_table.h"
#include "_struct.h"
#include "_Tuile.h"
#include "_joueur.h"  

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
void ordonner_table(const char* fichier) {
    FILE* f = fopen(fichier, "r");
    if (!f) return;
    
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* data = malloc(fsize + 1);
    fread(data, 1, fsize, f);
    data[fsize] = 0;
    fclose(f);
    
    cJSON* root = cJSON_Parse(data);
    free(data);
    if (!root) return;
    
    for (int i = 0; i < cJSON_GetArraySize(root); i++) {
        cJSON* comb = cJSON_GetArrayItem(root, i);
        int n = cJSON_GetArraySize(comb);
        if (n <= 1) continue;
        
        // Vérifier couleur unique
        char couleur = '\0';
        bool meme_couleur = true;
        for (int j = 0; j < n; j++) {
            cJSON* t = cJSON_GetArrayItem(comb, j);
            if (cJSON_IsTrue(cJSON_GetObjectItem(t, "joker"))) continue;
            
            char c = cJSON_GetObjectItem(t, "couleur")->valuestring[0];
            if (couleur == '\0') couleur = c;
            else if (c != couleur) { meme_couleur = false; break; }
        }
        
        if (meme_couleur) {
            // Créer un nouveau tableau trié
            cJSON* new_comb = cJSON_CreateArray();
            
            // Extraire les tuiles
            int valeurs[n];
            cJSON* tuiles[n];
            
            for (int j = 0; j < n; j++) {
                cJSON* t = cJSON_GetArrayItem(comb, j);
                valeurs[j] = cJSON_GetObjectItem(t, "valeur")->valueint;
                tuiles[j] = t;
            }
            
            // Tri à bulles
            for (int j = 0; j < n - 1; j++) {
                for (int k = 0; k < n - j - 1; k++) {
                    if (valeurs[k] > valeurs[k + 1]) {
                        int temp_val = valeurs[k];
                        valeurs[k] = valeurs[k + 1];
                        valeurs[k + 1] = temp_val;
                        
                        cJSON* temp_t = tuiles[k];
                        tuiles[k] = tuiles[k + 1];
                        tuiles[k + 1] = temp_t;
                    }
                }
            }
            
            // Ajouter dans le nouvel ordre
            for (int j = 0; j < n; j++) {
                cJSON_AddItemToArray(new_comb, cJSON_Duplicate(tuiles[j], 1));
            }
            
            // CORRECTION POSITION JOKER
            int joker_pos = -1;
            for (int j = 0; j < n; j++) {
                cJSON* t = cJSON_GetArrayItem(new_comb, j);
                if (cJSON_IsTrue(cJSON_GetObjectItem(t, "joker"))) {
                    joker_pos = j;
                    break;
                }
            }
            
            if (joker_pos != -1) {
                if (joker_pos == 0 && n >= 3) {
                    int position_trou = -1;
                    
                    for (int j = 1; j < n - 1; j++) {
                        cJSON* t1 = cJSON_GetArrayItem(new_comb, j);
                        cJSON* t2 = cJSON_GetArrayItem(new_comb, j + 1);
                        
                        if (cJSON_IsTrue(cJSON_GetObjectItem(t1, "joker")) || 
                            cJSON_IsTrue(cJSON_GetObjectItem(t2, "joker"))) {
                            continue;
                        }
                        
                        int v1 = cJSON_GetObjectItem(t1, "valeur")->valueint;
                        int v2 = cJSON_GetObjectItem(t2, "valeur")->valueint;
                        
                        if (v2 - v1 > 1) {
                            position_trou = j;
                            break;
                        }
                    }
                    
                    if (position_trou != -1) {
                        cJSON* joker = cJSON_DetachItemFromArray(new_comb, 0);
                        cJSON_InsertItemInArray(new_comb, position_trou, joker);
                    } else {
                        cJSON* joker = cJSON_DetachItemFromArray(new_comb, 0);
                        cJSON_AddItemToArray(new_comb, joker);
                    }
                }
            }
            
            cJSON_ReplaceItemInArray(root, i, new_comb);
        }
    }
    
    char* str = cJSON_Print(root);
    FILE* fw = fopen(fichier, "w");
    if (fw) { fprintf(fw, "%s", str); fclose(fw); }
    free(str);
    cJSON_Delete(root);
}
/* ------------------------------------------------------------------------- */
