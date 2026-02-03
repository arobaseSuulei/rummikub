#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <cjson/cJSON.h>
#include "_struct.h"
#include "_Tuile.h"
#include "_joueur.h"
#include "_manipulation.h"

#define TABLE_VIRTUELLE "table_virtuelle.json"
#define CHEVALET_VIRTUEL "chevalet_virtuel.json"

bool valider_tour(Joueur* j) {
    FILE* f = fopen("table_virtuelle.json", "r");
    if (!f) return false;
    
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* data = malloc(fsize + 1);
    fread(data, 1, fsize, f);
    data[fsize] = 0;
    fclose(f);
    
    cJSON* root = cJSON_Parse(data);
    free(data);
    if (!root) return false;
    
    int nb_comb = cJSON_GetArraySize(root);
    bool toutes_valides = true;
    
    for (int i = 0; i < nb_comb; i++) {
        cJSON* comb = cJSON_GetArrayItem(root, i);
        int nb_tuiles = cJSON_GetArraySize(comb);
        
        Tuile tuiles[MAX_TUILES];
        for (int j = 0; j < nb_tuiles; j++) {
            cJSON* tuile_json = cJSON_GetArrayItem(comb, j);
            tuiles[j].id = cJSON_GetObjectItem(tuile_json, "id")->valueint;
            tuiles[j].valeur = cJSON_GetObjectItem(tuile_json, "valeur")->valueint;
            tuiles[j].couleur = cJSON_GetObjectItem(tuile_json, "couleur")->valuestring[0];
            tuiles[j].joker = cJSON_IsTrue(cJSON_GetObjectItem(tuile_json, "joker"));
        }
        
        if (!combinaison_valide(tuiles, nb_tuiles)) {
            toutes_valides = false;
            break;
        }
    }
    
    if (!toutes_valides) {
        cJSON_Delete(root);
        return false;
    }
    
    char* str = cJSON_Print(root);
    FILE* f_table = fopen("table.json", "w");
    if (f_table) {
        fprintf(f_table, "%s", str);
        fclose(f_table);
    }
    free(str);
    
    FILE* f_chevalet = fopen("chevalet_virtuel.json", "r");
    if (f_chevalet) {
        fseek(f_chevalet, 0, SEEK_END);
        long fsize = ftell(f_chevalet);
        fseek(f_chevalet, 0, SEEK_SET);
        char* data = malloc(fsize + 1);
        fread(data, 1, fsize, f_chevalet);
        data[fsize] = 0;
        fclose(f_chevalet);
        
        FILE* f_dest = fopen(j->chevalet, "w");
        if (f_dest) {
            fprintf(f_dest, "%s", data);
            fclose(f_dest);
        }
        free(data);
    }
    
    remove("table_virtuelle.json");
    remove("chevalet_virtuel.json");
    
    cJSON_Delete(root);
    return true;
}