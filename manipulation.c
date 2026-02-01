#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <cjson/cJSON.h>
#include "struct.h"
#include "Tuile.h"
#include "joueur.h"
#include "manipulation.h"

#define TABLE_VIRTUELLE "table_virtuelle.json"
#define CHEVALET_VIRTUEL "chevalet_virtuel.json"

/*--------------------------------------------------------*/
void debut_manipulation(Joueur* j) {
    // 1. Copier table.json → table_virtuelle.json
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
        printf("Table virtuelle créée\n");
    } else {
        if (src) fclose(src);
        if (dst) fclose(dst);
        // Créer table_virtuelle.json vide si table.json n'existe pas
        dst = fopen("table_virtuelle.json", "w");
        if (dst) {
            fprintf(dst, "[]");
            fclose(dst);
        }
    }
    
    // 2. Copier chevalet du joueur → chevalet_virtuel.json
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
        printf("Chevalet virtuel créé\n");
    } else {
        if (src) fclose(src);
        if (dst) fclose(dst);
        printf("Erreur: impossible de copier le chevalet\n");
    }
}

/*--------------------------------------------------------*/
void isoler_tuile(int tuile_id) {
    FILE* f_test = fopen("table_virtuelle.json", "r");
    if (!f_test) {
        printf("Erreur: table_virtuelle.json non trouvé\n");
        return;
    }
    fclose(f_test);
    
    FILE* f = fopen("table_virtuelle.json", "r");
    if (!f) {
        printf("Erreur: impossible d'ouvrir table_virtuelle.json\n");
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
        printf("Erreur: JSON invalide\n");
        return;
    }
    
    int nb_comb = cJSON_GetArraySize(root);
    int comb_index = -1, tuile_index = -1;
    cJSON* combinaison_trouvee = NULL;
    bool tuile_est_joker = false;
    
    for (int i = 0; i < nb_comb; i++) {
        cJSON* comb = cJSON_GetArrayItem(root, i);
        int nb_tuiles = cJSON_GetArraySize(comb);
        
        for (int j = 0; j < nb_tuiles; j++) {
            cJSON* tuile_json = cJSON_GetArrayItem(comb, j);
            int id = cJSON_GetObjectItem(tuile_json, "id")->valueint;
            
            if (id == tuile_id) {
                comb_index = i;
                tuile_index = j;
                combinaison_trouvee = comb;
                tuile_est_joker = cJSON_IsTrue(cJSON_GetObjectItem(tuile_json, "joker"));
                break;
            }
        }
        if (comb_index != -1) break;
    }
    
    if (comb_index == -1) {
        printf("Tuile ID %d non trouvée sur la table\n", tuile_id);
        cJSON_Delete(root);
        return;
    }
    
    if (cJSON_GetArraySize(combinaison_trouvee) == 1) {
        printf("Tuile ID %d est déjà isolée\n", tuile_id);
        cJSON_Delete(root);
        return;
    }
    
    // VÉRIFICATION RÈGLE JOKER
    bool contient_autre_joker = false;
    int nb_tuiles_comb = cJSON_GetArraySize(combinaison_trouvee);
    
    for (int k = 0; k < nb_tuiles_comb; k++) {
        cJSON* tuile_test = cJSON_GetArrayItem(combinaison_trouvee, k);
        bool est_joker = cJSON_IsTrue(cJSON_GetObjectItem(tuile_test, "joker"));
        
        if (est_joker) {
            int id_test = cJSON_GetObjectItem(tuile_test, "id")->valueint;
            if (id_test != tuile_id) {
                contient_autre_joker = true;
                break;
            }
        }
    }
    
    if (contient_autre_joker && !tuile_est_joker) {
        printf("Interdit: la combinaison contient un joker. Vous ne pouvez isoler que le joker lui-même.\n");
        cJSON_Delete(root);
        return;
    }
    
    cJSON* tuile_a_isoler = cJSON_DetachItemFromArray(combinaison_trouvee, tuile_index);
    cJSON* nouvelle_comb = cJSON_CreateArray();
    cJSON_AddItemToArray(nouvelle_comb, tuile_a_isoler);
    cJSON_AddItemToArray(root, nouvelle_comb);
    
    char* str = cJSON_Print(root);
    f = fopen("table_virtuelle.json", "w");
    if (f) {
        fprintf(f, "%s", str);
        fclose(f);
        printf("Tuile ID %d isolée avec succès\n", tuile_id);
    }
    
    free(str);
    cJSON_Delete(root);
}

void isoler_tuile_chevalet(int tuile_id) {
    // 1. Chercher la tuile dans chevalet_virtuel.json
    FILE* f = fopen("chevalet_virtuel.json", "r");
    if (!f) {
        printf("Erreur: chevalet_virtuel.json non trouvé\n");
        return;
    }
    
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* data = malloc(fsize + 1);
    fread(data, 1, fsize, f);
    data[fsize] = 0;
    fclose(f);
    
    cJSON* chevalet_root = cJSON_Parse(data);
    free(data);
    if (!chevalet_root) {
        printf("Erreur: JSON invalide\n");
        return;
    }
    
    // 2. Chercher la tuile
    cJSON* tuiles_array = cJSON_GetObjectItem(chevalet_root, "tuiles");
    if (!tuiles_array) {
        printf("Erreur: pas de tableau 'tuiles'\n");
        cJSON_Delete(chevalet_root);
        return;
    }
    
    int nb_tuiles = cJSON_GetArraySize(tuiles_array);
    int tuile_index = -1;
    cJSON* tuile_trouvee = NULL;
    
    for (int i = 0; i < nb_tuiles; i++) {
        cJSON* tuile_json = cJSON_GetArrayItem(tuiles_array, i);
        int id = cJSON_GetObjectItem(tuile_json, "id")->valueint;
        
        if (id == tuile_id) {
            tuile_index = i;
            tuile_trouvee = cJSON_Duplicate(tuile_json, 1);
            break;
        }
    }
    
    if (tuile_index == -1) {
        printf("Tuile ID %d non trouvée dans votre chevalet\n", tuile_id);
        cJSON_Delete(chevalet_root);
        return;
    }
    
    // 3. Retirer du chevalet
    cJSON_DeleteItemFromArray(tuiles_array, tuile_index);
    
    // Sauvegarder chevalet modifié
    char* str = cJSON_Print(chevalet_root);
    f = fopen("chevalet_virtuel.json", "w");
    if (f) {
        fprintf(f, "%s", str);
        fclose(f);
    }
    free(str);
    cJSON_Delete(chevalet_root);
    
    // 4. Ajouter comme combinaison d'1 tuile à table_virtuelle.json
    f = fopen("table_virtuelle.json", "r");
    if (!f) {
        printf("Erreur: table_virtuelle.json non trouvé\n");
        cJSON_Delete(tuile_trouvee);
        return;
    }
    
    fseek(f, 0, SEEK_END);
    fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    data = malloc(fsize + 1);
    fread(data, 1, fsize, f);
    data[fsize] = 0;
    fclose(f);
    
    cJSON* table_root = cJSON_Parse(data);
    free(data);
    if (!table_root) {
        printf("Erreur: table_virtuelle.json invalide\n");
        cJSON_Delete(tuile_trouvee);
        return;
    }
    
    // Créer nouvelle combinaison
    cJSON* nouvelle_comb = cJSON_CreateArray();
    cJSON_AddItemToArray(nouvelle_comb, tuile_trouvee);
    cJSON_AddItemToArray(table_root, nouvelle_comb);
    
    // Sauvegarder table
    str = cJSON_Print(table_root);
    f = fopen("table_virtuelle.json", "w");
    if (f) {
        fprintf(f, "%s", str);
        fclose(f);
        printf("Tuile ID %d isolée du chevalet et placée sur la table\n", tuile_id);
    }
    
    free(str);
    cJSON_Delete(table_root);
}

void ajouter_tuile_combinaison(int tuile_id, int combinaison_index) {
    cJSON* tuile_trouvee = NULL;
    int source_type = 0;
    int comb_index_table = -1, tuile_index_table = -1;
    bool tuile_est_joker = false;
    
    FILE* f = fopen("table_virtuelle.json", "r");
    if (f) {
        fseek(f, 0, SEEK_END);
        long fsize = ftell(f);
        fseek(f, 0, SEEK_SET);
        char* data = malloc(fsize + 1);
        fread(data, 1, fsize, f);
        data[fsize] = 0;
        fclose(f);
        
        cJSON* table_root = cJSON_Parse(data);
        free(data);
        
        if (table_root) {
            int nb_comb = cJSON_GetArraySize(table_root);
            
            for (int i = 0; i < nb_comb; i++) {
                cJSON* comb = cJSON_GetArrayItem(table_root, i);
                int nb_tuiles = cJSON_GetArraySize(comb);
                
                for (int j = 0; j < nb_tuiles; j++) {
                    cJSON* tuile_json = cJSON_GetArrayItem(comb, j);
                    int id = cJSON_GetObjectItem(tuile_json, "id")->valueint;
                    
                    if (id == tuile_id) {
                        tuile_trouvee = cJSON_Duplicate(tuile_json, 1);
                        source_type = 1;
                        comb_index_table = i;
                        tuile_index_table = j;
                        tuile_est_joker = cJSON_IsTrue(cJSON_GetObjectItem(tuile_json, "joker"));
                        break;
                    }
                }
                if (tuile_trouvee) break;
            }
            
            if (tuile_trouvee) {
                cJSON* comb_source = cJSON_GetArrayItem(table_root, comb_index_table);
                
                // VÉRIFICATION RÈGLE JOKER
                bool source_contient_autre_joker = false;
                int nb_tuiles_source = cJSON_GetArraySize(comb_source);
                
                for (int k = 0; k < nb_tuiles_source; k++) {
                    cJSON* tuile_test = cJSON_GetArrayItem(comb_source, k);
                    bool est_joker = cJSON_IsTrue(cJSON_GetObjectItem(tuile_test, "joker"));
                    
                    if (est_joker) {
                        int id_test = cJSON_GetObjectItem(tuile_test, "id")->valueint;
                        if (id_test != tuile_id) {
                            source_contient_autre_joker = true;
                            break;
                        }
                    }
                }
                
                if (source_contient_autre_joker && !tuile_est_joker) {
                    printf("Interdit: la combinaison source contient un joker. Vous ne pouvez prendre que le joker.\n");
                    cJSON_Delete(table_root);
                    return;
                }
                
                cJSON_DeleteItemFromArray(comb_source, tuile_index_table);
                if (cJSON_GetArraySize(comb_source) == 0) {
                    cJSON_DeleteItemFromArray(table_root, comb_index_table);
                }
                
                char* str = cJSON_Print(table_root);
                f = fopen("table_virtuelle.json", "w");
                if (f) {
                    fprintf(f, "%s", str);
                    fclose(f);
                }
                free(str);
            }
            cJSON_Delete(table_root);
        }
    }
    
    if (!tuile_trouvee) {
        f = fopen("chevalet_virtuel.json", "r");
        if (f) {
            fseek(f, 0, SEEK_END);
            long fsize = ftell(f);
            fseek(f, 0, SEEK_SET);
            char* data = malloc(fsize + 1);
            fread(data, 1, fsize, f);
            data[fsize] = 0;
            fclose(f);
            
            cJSON* chevalet_root = cJSON_Parse(data);
            free(data);
            
            if (chevalet_root) {
                cJSON* tuiles_array = cJSON_GetObjectItem(chevalet_root, "tuiles");
                if (tuiles_array) {
                    int nb_tuiles = cJSON_GetArraySize(tuiles_array);
                    
                    for (int i = 0; i < nb_tuiles; i++) {
                        cJSON* tuile_json = cJSON_GetArrayItem(tuiles_array, i);
                        int id = cJSON_GetObjectItem(tuile_json, "id")->valueint;
                        
                        if (id == tuile_id) {
                            tuile_trouvee = cJSON_Duplicate(tuile_json, 1);
                            source_type = 2;
                            tuile_est_joker = cJSON_IsTrue(cJSON_GetObjectItem(tuile_json, "joker"));
                            
                            cJSON_DeleteItemFromArray(tuiles_array, i);
                            char* str = cJSON_Print(chevalet_root);
                            f = fopen("chevalet_virtuel.json", "w");
                            if (f) {
                                fprintf(f, "%s", str);
                                fclose(f);
                            }
                            free(str);
                            break;
                        }
                    }
                }
                cJSON_Delete(chevalet_root);
            }
        }
    }
    
    if (!tuile_trouvee) {
        printf("Erreur: tuile ID %d non trouvée\n", tuile_id);
        return;
    }
    
    f = fopen("table_virtuelle.json", "r");
    if (!f) {
        printf("Erreur: table_virtuelle.json non trouvé\n");
        cJSON_Delete(tuile_trouvee);
        return;
    }
    
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* data = malloc(fsize + 1);
    fread(data, 1, fsize, f);
    data[fsize] = 0;
    fclose(f);
    
    cJSON* table_root = cJSON_Parse(data);
    free(data);
    if (!table_root) {
        printf("Erreur: table_virtuelle.json invalide\n");
        cJSON_Delete(tuile_trouvee);
        return;
    }
    
    int nb_comb = cJSON_GetArraySize(table_root);
    if (combinaison_index < 0 || combinaison_index >= nb_comb) {
        printf("Erreur: index de combinaison %d invalide (0-%d)\n", combinaison_index, nb_comb-1);
        cJSON_Delete(table_root);
        cJSON_Delete(tuile_trouvee);
        return;
    }
    
    cJSON* comb_cible = cJSON_GetArrayItem(table_root, combinaison_index);
    cJSON_AddItemToArray(comb_cible, tuile_trouvee);
    
    char* str = cJSON_Print(table_root);
    f = fopen("table_virtuelle.json", "w");
    if (f) {
        fprintf(f, "%s", str);
        fclose(f);
        printf("Tuile ID %d (%s) ajoutée à la combinaison [%d]\n", 
               tuile_id, 
               source_type == 1 ? "table" : "chevalet",
               combinaison_index);
    }
    
    free(str);
    cJSON_Delete(table_root);
}

bool valider_tour(Joueur* j) {
    FILE* f = fopen("table_virtuelle.json", "r");
    if (!f) {
        printf("Erreur: table_virtuelle.json non trouvé\n");
        return false;
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
        printf("Erreur: table_virtuelle.json invalide\n");
        return false;
    }
    
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
            printf("❌ Combinaison [%d] invalide: ", i);
            for (int j = 0; j < nb_tuiles; j++) {
                printf("%d%c ", tuiles[j].valeur, tuiles[j].couleur);
            }
            printf("\n");
            toutes_valides = false;
        }
    }
    
    // SI INVALIDE, ARRÊTER ICI
    if (!toutes_valides) {
        printf("❌ Table invalide. Corrigez les combinaisons signalées.\n");
        cJSON_Delete(root);
        return false;  // ← RETOURNER FALSE
    }
    
    // SI VALIDE, CONTINUER
    char* str = cJSON_Print(root);
    FILE* f_table = fopen("table.json", "w");
    if (f_table) {
        fprintf(f_table, "%s", str);
        fclose(f_table);
    }
    free(str);
    
    // Mettre à jour chevalet
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
    
    printf("✅ Toutes les combinaisons sont valides !\n");
    printf("✅ Table et chevalet mis à jour.\n");
    
    cJSON_Delete(root);
    return true;
}