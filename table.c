#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <cjson/cJSON.h>
#include "table.h"
#include "struct.h"
#include "Tuile.h"
#include "joueur.h"  

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
void afficher_table(const char* fichier_table) {
    FILE* f = fopen(fichier_table, "r");
    if (!f) {
        printf("Table vide ou fichier %s non trouvé\n", fichier_table);
        return;
    }
    
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* data = malloc(fsize + 1);
    fread(data, 1, fsize, f);
    data[fsize] = 0;
    fclose(f);
    
    cJSON* root = cJSON_Parse(data);
    free(data);
    if (!root) {
        printf("Erreur de lecture de la table\n");
        return;
    }
    
    int nb_comb = cJSON_GetArraySize(root);
    printf("\n=== TABLE (%d combinaison%s) ===\n", nb_comb, nb_comb > 1 ? "s" : "");
    
    if (nb_comb == 0) {
        printf("Aucune combinaison sur la table.\n");
    }
    
    for (int i = 0; i < nb_comb; i++) {
        cJSON* comb = cJSON_GetArrayItem(root, i);
        int nb_tuiles = cJSON_GetArraySize(comb);
        
        printf("[%d] ", i);
        for (int j = 0; j < nb_tuiles; j++) {
            cJSON* tuile_json = cJSON_GetArrayItem(comb, j);
            int valeur = cJSON_GetObjectItem(tuile_json, "valeur")->valueint;
            char couleur = cJSON_GetObjectItem(tuile_json, "couleur")->valuestring[0];
            bool joker = cJSON_IsTrue(cJSON_GetObjectItem(tuile_json, "joker"));
            
            if (joker) {
                printf("[J] ");
            } else {
                printf("%d%c ", valeur, couleur);
            }
        }
        printf("\n");
    }
    printf("=============================\n");
    
    cJSON_Delete(root);
}