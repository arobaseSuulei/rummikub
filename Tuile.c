#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <cjson/cJSON.h>
#include "struct.h"
#include "Tuile.h"
#include "joueur.h"  // Pour Joueur*

/*-------------------------------------------------------------------*/
Tuile* initialiser_tuile() {
    Tuile* t = malloc(MAX_TUILES * sizeof(Tuile));
    if (!t) return NULL;
    
    int index = 0;
    int id = 1;
    char couleurs[] = {'B','R','O','V'};
    
    for (int exemplaire = 0; exemplaire < 2; exemplaire++) {
        for (int valeur = 1; valeur <= 13; valeur++) {
            for (int c = 0; c < 4; c++) {
                t[index++] = (Tuile){id++, valeur, couleurs[c], false};
            }
        }
    }
    
    t[index++] = (Tuile){id++, 0, 'J', true};
    t[index++] = (Tuile){id++, 0, 'J', true};
    
    return t;
}

/*-------------------------------------------------------------------*/
Tuile* melanger_tuiles() {
    Tuile* t = initialiser_tuile();
    if (!t) return NULL;
    
    srand(time(NULL));
    for (int i = MAX_TUILES - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Tuile tmp = t[i];
        t[i] = t[j];
        t[j] = tmp;
    }
    return t;
}

/*-------------------------------------------------------------------*/
void creer_pioche() {
    Tuile* t = melanger_tuiles();
    if (!t) return;
    
    cJSON *root = cJSON_CreateArray();
    for (int i = 0; i < MAX_TUILES; i++) {
        cJSON *item = cJSON_CreateObject();
        cJSON_AddNumberToObject(item, "id", t[i].id);
        cJSON_AddNumberToObject(item, "valeur", t[i].valeur);
        char str_color[2] = {t[i].couleur, 0};
        cJSON_AddStringToObject(item, "couleur", str_color);
        cJSON_AddBoolToObject(item, "joker", t[i].joker);
        cJSON_AddItemToArray(root, item);
    }
    
    char *str = cJSON_Print(root);
    FILE *f = fopen("pioche.json", "w");
    if (f) {
        fprintf(f, "%s", str);
        fclose(f);
        printf("Pioche créée avec succès.\n");
    }
    
    cJSON_Delete(root);
    free(str);
    free(t);
}

/*-------------------------------------------------------------------*/
bool combinaison_valide(Tuile* tuiles, int nb) {
    if (nb < 3) return false;
    
    int jokers = 0;
    for (int i = 0; i < nb; i++)
        if (tuiles[i].joker) jokers++;
    
    // === VÉRIFICATION BRELAN ===
    int valeur_set = -1;
    bool possible_set = true;
    char couleurs[4];
    int couleur_count = 0;
    
    for (int i = 0; i < nb; i++) {
        if (tuiles[i].joker) continue;
        
        if (valeur_set == -1) {
            valeur_set = tuiles[i].valeur;
        } else if (tuiles[i].valeur != valeur_set) { 
            possible_set = false; 
            break; 
        }
        
        // Vérifier doublon de couleur
        bool couleur_deja_presente = false;
        for (int j = 0; j < couleur_count; j++) {
            if (couleurs[j] == tuiles[i].couleur) {
                couleur_deja_presente = true;
                break;
            }
        }
        
        if (couleur_deja_presente) {
            possible_set = false;
            break;
        }
        
        if (couleur_count < 4) {
            couleurs[couleur_count++] = tuiles[i].couleur;
        } else {
            possible_set = false;
            break;
        }
    }
    
    if (possible_set && nb <= 4) {
        return true;
    }
    
    // === VÉRIFICATION SUITE ===
    if (nb - jokers < 2) return false;
    
    char couleur_suite = '\0';
    int valeurs[nb - jokers];
    int v_idx = 0;
    
    for (int i = 0; i < nb; i++) {
        if (tuiles[i].joker) continue;
        
        if (couleur_suite == '\0') {
            couleur_suite = tuiles[i].couleur;
        } else if (tuiles[i].couleur != couleur_suite) {
            return false;
        }
        
        valeurs[v_idx++] = tuiles[i].valeur;
    }
    
    // Trier les valeurs
    for (int i = 0; i < v_idx - 1; i++) {
        for (int j = i + 1; j < v_idx; j++) {
            if (valeurs[i] > valeurs[j]) {
                int tmp = valeurs[i];
                valeurs[i] = valeurs[j];
                valeurs[j] = tmp;
            }
        }
    }
    
    // Vérifier les doublons
    for (int i = 0; i < v_idx - 1; i++) {
        if (valeurs[i] == valeurs[i + 1]) return false;
    }
    
    // Calculer les "trous"
    int gaps = 0;
    for (int i = 0; i < v_idx - 1; i++) {
        gaps += valeurs[i + 1] - valeurs[i] - 1;
    }
    
    return (gaps <= jokers);
}

/*-------------------------------------------------------------------*/
void afficher_tuiles(Tuile tuiles[], int nb_tuiles) {
    printf("Vos tuiles :\n");
    for (int i = 0; i < nb_tuiles; i++) {
        printf("%d: %d%c%s\n",
               tuiles[i].id,
               tuiles[i].valeur,
               tuiles[i].couleur,
               tuiles[i].joker ? " (J)" : "");
    }
}

/*-------------------------------------------------------------------*/
void ajouter_a_table(Tuile* comb, int n) {
    if (n < 3) return;
    
    cJSON *root = NULL;
    FILE* f = fopen("table.json", "r");
    if(f){
        fseek(f,0,SEEK_END);
        long fsize = ftell(f);
        fseek(f,0,SEEK_SET);
        char *data = malloc(fsize+1);
        fread(data,1,fsize,f);
        data[fsize]=0;
        fclose(f);
        
        root = cJSON_Parse(data);
        free(data);
    }
    
    if(!root) root = cJSON_CreateArray();
    
    cJSON *new_comb = cJSON_CreateArray();
    for(int i=0;i<n;i++){
        cJSON *item = cJSON_CreateObject();
        cJSON_AddNumberToObject(item,"id",comb[i].id);
        cJSON_AddNumberToObject(item,"valeur",comb[i].valeur);
        char str_color[2]={comb[i].couleur,0};
        cJSON_AddStringToObject(item,"couleur",str_color);
        cJSON_AddBoolToObject(item,"joker",comb[i].joker);
        cJSON_AddItemToArray(new_comb,item);
    }
    cJSON_AddItemToArray(root,new_comb);
    
    char *str = cJSON_Print(root);
    FILE* fw = fopen("table.json","w");
    if(fw){ fprintf(fw,"%s",str); fclose(fw); }
    free(str);
    cJSON_Delete(root);
}

/*-------------------------------------------------------------------*/
void distribuer_tuile() {
    int nb_joueurs;
    Joueur* players = malloc(sizeof(Joueur) * 4); // temporaire
    // TODO: Implémenter proprement
    printf("distribuer_tuile() - À IMPLÉMENTER\n");
    free(players);
}

/*-------------------------------------------------------------------*/
void piocher_tuile(Joueur* j) {
    // Copie de la fonction de joueur.c
    FILE* f = fopen("pioche.json", "r");
    if (!f) return;
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *data = malloc(fsize+1);
    fread(data, 1, fsize, f);
    data[fsize] = 0;
    fclose(f);
    
    cJSON *root = cJSON_Parse(data);
    if (!root) { free(data); return; }
    
    cJSON *first = cJSON_GetArrayItem(root, 0);
    if(!first){ cJSON_Delete(root); free(data); return; }
    
    Tuile t;
    t.id = cJSON_GetObjectItem(first,"id")->valueint;
    t.valeur = cJSON_GetObjectItem(first,"valeur")->valueint;
    t.couleur = cJSON_GetObjectItem(first,"couleur")->valuestring[0];
    t.joker = cJSON_IsTrue(cJSON_GetObjectItem(first,"joker"));
    
    cJSON_DeleteItemFromArray(root,0);
    char *new_data = cJSON_Print(root);
    FILE* fw = fopen("pioche.json","w");
    if(fw){ fprintf(fw,"%s",new_data); fclose(fw); }
    free(new_data);
    free(data);
    cJSON_Delete(root);
    
    f = fopen(j->chevalet,"r");
    if(!f) return;
    fseek(f,0,SEEK_END);
    fsize = ftell(f);
    fseek(f,0,SEEK_SET);
    data = malloc(fsize+1);
    fread(data,1,fsize,f);
    data[fsize]=0;
    fclose(f);
    
    root = cJSON_Parse(data);
    if(!root){ free(data); return; }
    
    cJSON *tuiles_obj = cJSON_GetObjectItem(root,"tuiles");
    cJSON *new_tuile = cJSON_CreateObject();
    cJSON_AddNumberToObject(new_tuile,"id",t.id);
    cJSON_AddNumberToObject(new_tuile,"valeur",t.valeur);
    char str_color[2]={t.couleur,0};
    cJSON_AddStringToObject(new_tuile,"couleur",str_color);
    cJSON_AddBoolToObject(new_tuile,"joker",t.joker);
    cJSON_AddItemToArray(tuiles_obj,new_tuile);
    
    new_data = cJSON_Print(root);
    fw = fopen(j->chevalet,"w");
    if(fw){ fprintf(fw,"%s",new_data); fclose(fw); }
    free(new_data);
    free(data);
    cJSON_Delete(root);
}
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