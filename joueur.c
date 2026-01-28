#include <stdio.h>
#include <stdbool.h>
#include "struct.h"
#include "Tuile.h"
#include "joueur.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*-----------------------------------------------------------------------------------------------------------------------------*/
int nbr_joueur(){
    int n;
    printf("Entrer le nombre de joueur : ");
    scanf("%d",&n);
    FILE* f = fopen("nombre_joueurs.txt", "w");
    fprintf(f, "%d", n);
    fclose(f);
    return n;
}

/*------------------------------------------------------------------------------------------------------------------------------*/
Joueur* ordre_joueur(Joueur* players, int nb_joueurs) {
    srand(time(NULL));
    for (int i = nb_joueurs - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Joueur tmp = players[i];
        players[i] = players[j];
        players[j] = tmp;
    }
    return players;
}

/*-------------------------------------------------------------------------------------------------------------------------------*/
Joueur* creer_joueur(int* nb_joueurs){
    int nb = nbr_joueur();
    *nb_joueurs = nb;

    Joueur *players = malloc(nb * sizeof(Joueur));
    if (!players){
        printf("pas assez de memoire\n");
        exit(1);
    }

    for (int i = 0; i < nb; i++){
        printf("Joueur %d: ", i+1);
        scanf("%49s", players[i].pseudo);
        getchar();
    }

    ordre_joueur(players, nb);

    for (int i = 0; i < nb; i++){
        char filename[50];
        sprintf(filename,"%d.json", i+1);
        strcpy(players[i].chevalet, filename);
    }

    return players;
}

/*---------------------------------------------------------------------------------------------------------*/
void distribuer_tuile() {
    int nb_joueurs;
    Joueur* players = creer_joueur(&nb_joueurs);
    if (!players) return;

    for (int j = 0; j < nb_joueurs; j++) {
        FILE* fj = fopen(players[j].chevalet, "w");
        if (!fj) continue;

        fprintf(fj,
            "{\n  \"pseudo\": \"%s\",\n  \"tour\": %s,\n  \"premier_tour\": true,\n  \"tuiles\": []\n}\n",
            players[j].pseudo, (j == 0) ? "true" : "false");
        fclose(fj);
    }

    for (int i = 0; i < 14; i++)
        for (int j = 0; j < nb_joueurs; j++)
            piocher_tuile(players[j]);

    free(players);
}

/*------------------------------------------------------------------------------------------------------------*/
void piocher_tuile(Joueur j) {
    Tuile pioche[MAX_TUILES], tuiles_joueur[MAX_TUILES];
    int np = 0, nj = 0;
    char ligne[256];
    char pseudo[50] = "";
    int tour = 0;
    bool premier_tour = true;

    FILE* f = fopen("pioche.json", "r");
    if (!f) return;

    while (fgets(ligne, sizeof(ligne), f)) {
        if (strchr(ligne, '{')) {
            sscanf(ligne,
                " {\"id\":%d,\"valeur\":%d,\"couleur\":\"%c\",\"joker\":%*[^t]true}",
                &pioche[np].id,
                &pioche[np].valeur,
                &pioche[np].couleur);
            pioche[np].joker = strstr(ligne, "true") != NULL;
            np++;
        }
    }
    fclose(f);
    if (np == 0) return;

    Tuile t = pioche[0];

    f = fopen("pioche.json", "w");
    fprintf(f, "[\n");
    for (int i = 1; i < np; i++)
        fprintf(f,
            "  {\"id\":%d,\"valeur\":%d,\"couleur\":\"%c\",\"joker\":%s}%s\n",
            pioche[i].id,
            pioche[i].valeur,
            pioche[i].couleur,
            pioche[i].joker ? "true" : "false",
            (i < np - 1) ? "," : "");
    fprintf(f, "]\n");
    fclose(f);

    FILE* fj = fopen(j.chevalet, "r");
    if (!fj) return;

    while (fgets(ligne, sizeof(ligne), fj)) {
        if (strstr(ligne, "\"pseudo\"")) {
            sscanf(ligne, " \"pseudo\": \"%49[^\"]\"", pseudo);
            continue;
        }
        if (strstr(ligne, "\"tour\": true")) tour = 1;
        else if (strstr(ligne, "\"tour\": false")) tour = 0;

        if (strstr(ligne, "\"premier_tour\": false")) premier_tour = false;

        if (strchr(ligne, '{') && strstr(ligne, "id")) {
            sscanf(ligne,
                " {\"id\":%d,\"valeur\":%d,\"couleur\":\"%c\",\"joker\":%*[^t]true}",
                &tuiles_joueur[nj].id,
                &tuiles_joueur[nj].valeur,
                &tuiles_joueur[nj].couleur);
            tuiles_joueur[nj].joker = strstr(ligne, "true") != NULL;
            nj++;
        }
    }
    fclose(fj);

    tuiles_joueur[nj++] = t;

    fj = fopen(j.chevalet, "w");
    fprintf(fj,
        "{\n  \"pseudo\": \"%s\",\n  \"tour\": %s,\n  \"premier_tour\": %s,\n  \"tuiles\": [\n",
        pseudo,
        tour ? "true" : "false",
        premier_tour ? "true" : "false");

    for (int i = 0; i < nj; i++)
        fprintf(fj,
            "    {\"id\":%d,\"valeur\":%d,\"couleur\":\"%c\",\"joker\":%s}%s\n",
            tuiles_joueur[i].id,
            tuiles_joueur[i].valeur,
            tuiles_joueur[i].couleur,
            tuiles_joueur[i].joker ? "true" : "false",
            (i < nj - 1) ? "," : "");

    fprintf(fj, "  ]\n}\n");
    fclose(fj);
}

/*------------------------------------------------------------------------------------------------------------------------------*/
bool combinaison_valide(Tuile* tuiles, int nb) {
    if (nb < 3) return false;

    int jokers = 0;
    for (int i = 0; i < nb; i++)
        if (tuiles[i].joker) jokers++;

    // Vérification si c'est un set (mêmes valeurs, couleurs différentes)
    int valeur_set = -1;
    bool possible_set = true;
    char couleurs[nb];
    int couleur_count = 0;

    for (int i = 0; i < nb; i++) {
        if (tuiles[i].joker) continue;
        if (valeur_set == -1) valeur_set = tuiles[i].valeur;
        else if (tuiles[i].valeur != valeur_set) {
            possible_set = false;
            break;
        }

        for (int j = 0; j < couleur_count; j++)
            if (couleurs[j] == tuiles[i].couleur) {
                possible_set = false;
                break;
            }
        if (!possible_set) break;
        couleurs[couleur_count++] = tuiles[i].couleur;
    }

    if (possible_set) return true;

    // Vérification si c'est une suite (valeurs consécutives, même couleur)
    if (nb - jokers < 2) return false; // minimum 3 tuiles avec jokers inclus

    char couleur_suite = '\0';
    int valeurs[nb - jokers];
    int v_idx = 0;
    for (int i = 0; i < nb; i++) {
        if (tuiles[i].joker) continue;
        if (couleur_suite == '\0') couleur_suite = tuiles[i].couleur;
        else if (tuiles[i].couleur != couleur_suite) return false;
        valeurs[v_idx++] = tuiles[i].valeur;
    }

    // Tri des valeurs pour vérifier les écarts
    for (int i = 0; i < v_idx - 1; i++)
        for (int j = i + 1; j < v_idx; j++)
            if (valeurs[i] > valeurs[j]) {
                int tmp = valeurs[i]; valeurs[i] = valeurs[j]; valeurs[j] = tmp;
            }

    // Vérifier qu'il n'y a pas de doublons dans les valeurs
    for (int i = 0; i < v_idx - 1; i++)
        if (valeurs[i] == valeurs[i + 1])
            return false;

    // Compter les "trous" que les jokers peuvent remplir
    int gaps = 0;
    for (int i = 0; i < v_idx - 1; i++)
        gaps += valeurs[i + 1] - valeurs[i] - 1;

    return (gaps <= jokers);
}


/*-------------------------------------------------------------------------------------------------------------------------------------*/
void afficher_tuiles(Tuile tuiles[], int nb_tuiles) {
    printf("Vos tuiles :\n");
    for (int i = 0; i < nb_tuiles; i++) {
        printf("%d: %d%c%s\n",
               tuiles[i].id,                  // afficher l'id
               tuiles[i].valeur,
               tuiles[i].couleur,
               tuiles[i].joker ? " (J)" : "");
    }
}


/*--------------------------------------------------------------------------------------------------------------------------------------*/
void charger_chevalet(const char* fichier, Tuile tuiles[], int* nb_tuiles) {
    char ligne[256];
    *nb_tuiles = 0;

    FILE* f = fopen(fichier, "r");
    if (!f) return;

    while (fgets(ligne, sizeof(ligne), f)) {
        if (strchr(ligne, '{') && strstr(ligne, "id")) {
            sscanf(ligne,
                " {\"id\":%d,\"valeur\":%d,\"couleur\":\"%c\",\"joker\":%*[^t]true}",
                &tuiles[*nb_tuiles].id,
                &tuiles[*nb_tuiles].valeur,
                &tuiles[*nb_tuiles].couleur);
            tuiles[*nb_tuiles].joker = strstr(ligne, "true") != NULL;
            (*nb_tuiles)++;
        }
    }
    fclose(f);
}

/*--------------------------------------------------------------------------------------------------------------------------------------------*/
void sauvegarder_chevalet(const char* fichier, Joueur j, Tuile tuiles[], int nb_tuiles)
{
    char pseudo[64] = "";
    bool tour = false;
    bool premier_tour = false;

    FILE* fr = fopen(fichier, "r");
    if (fr) {
        char buf[256];
        while (fgets(buf, sizeof(buf), fr)) {
            if (sscanf(buf, " \"pseudo\":\"%63[^\"]\"", pseudo) == 1) {}
            else if (strstr(buf, "\"tour\":true")) tour = true;
            else if (strstr(buf, "\"tour\":false")) tour = false;
            else if (strstr(buf, "\"premier_tour\":true")) premier_tour = true;
            else if (strstr(buf, "\"premier_tour\":false")) premier_tour = false;
        }
        fclose(fr);
    }

    if (pseudo[0] == '\0')
        strcpy(pseudo, j.pseudo);

    FILE* f = fopen(fichier, "w");
    if (!f) return;

    fprintf(f,
        "{\n"
        "  \"pseudo\":\"%s\",\n"
        "  \"tour\":%s,\n"
        "  \"premier_tour\":%s,\n"
        "  \"tuiles\":[\n",
        pseudo,
        tour ? "true" : "false",
        premier_tour ? "true" : "false"
    );

    for (int i = 0; i < nb_tuiles; i++) {
        if (i) fprintf(f, ",\n");
        fprintf(f,
            "    {\"id\":%d,\"valeur\":%d,\"couleur\":\"%c\",\"joker\":%s}",
            tuiles[i].id,
            tuiles[i].valeur,
            tuiles[i].couleur,
            tuiles[i].joker ? "true" : "false");
    }

    fprintf(f, "\n  ]\n}\n");
    fclose(f);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------*/
void ajouter_a_table(Tuile* comb, int n)
{
    if (n < 3) return;

    Tuile table[100][MAX_TUILES];
    int tailles[100];
    int nb_comb = 0;

    /* -------- lecture existante -------- */
    FILE* f = fopen("table.json", "r");
    if (f) {
        char ligne[256];
        int idx = -1;

        while (fgets(ligne, sizeof(ligne), f)) {
            if (strchr(ligne, '[') && idx == -1) continue;

            if (strchr(ligne, '[')) {
                idx++;
                tailles[idx] = 0;
                continue;
            }

            if (strchr(ligne, '{')) {
                sscanf(ligne,
                    " {\"id\":%d,\"valeur\":%d,\"couleur\":\"%c\",\"joker\":%*[^t]true}",
                    &table[idx][tailles[idx]].id,
                    &table[idx][tailles[idx]].valeur,
                    &table[idx][tailles[idx]].couleur);
                table[idx][tailles[idx]].joker = strstr(ligne, "true") != NULL;
                tailles[idx]++;
            }
        }
        nb_comb = idx + 1;
        fclose(f);
    }

    /* -------- ajouter nouvelle combinaison -------- */
    tailles[nb_comb] = n;
    for (int i = 0; i < n; i++)
        table[nb_comb][i] = comb[i];
    nb_comb++;

    /* -------- réécriture propre -------- */
    FILE* fw = fopen("table.json", "w");
    if (!fw) return;

    fprintf(fw, "[\n");
    for (int i = 0; i < nb_comb; i++) {
        fprintf(fw, "  [\n");
        for (int j = 0; j < tailles[i]; j++) {
            fprintf(fw,
                "    {\"id\":%d,\"valeur\":%d,\"couleur\":\"%c\",\"joker\":%s}%s\n",
                table[i][j].id,
                table[i][j].valeur,
                table[i][j].couleur,
                table[i][j].joker ? "true" : "false",
                (j < tailles[i] - 1) ? "," : "");
        }
        fprintf(fw, "  ]%s\n", (i < nb_comb - 1) ? "," : "");
    }
    fprintf(fw, "]\n");
    fclose(fw);
}



/*---------------------------------------------------------------------------------------------------------------------------------------*/
bool est_premier_tour(Joueur j) {
    FILE* f = fopen(j.chevalet, "r");
    if (!f) return true;

    char ligne[256];

    while (fgets(ligne, sizeof(ligne), f)) {
        if (strstr(ligne, "\"premier_tour\":true")) {
            fclose(f);
            return true;
        }
        if (strstr(ligne, "\"premier_tour\":false")) {
            fclose(f);
            return false;
        }
    }

    fclose(f);
    return true;
}

/*---------------------------------------------------------------------------------------------------------------------------------------*/
void jouer_combinaison(Joueur j)
{
    Tuile tuiles_joueur[MAX_TUILES];
    int nb_tuiles = 0;
    char ligne[256];
    bool premier_tour = est_premier_tour(j);
    int somme_cumulee = 0;

    if (premier_tour) {
        FILE* ft = fopen("tampon.json", "w");
        if (ft) {
            fprintf(ft, "[\n]\n");
            fclose(ft);
        }
        printf(">>> Premier tour : au moins 30 points.\n");
    }

    while (1) {
        charger_chevalet(j.chevalet, tuiles_joueur, &nb_tuiles);
        afficher_tuiles(tuiles_joueur, nb_tuiles);

        int indices[MAX_TUILES], nb_comb = 0;

        printf("Entrez les IDs des tuiles à poser (min 3) :\n");
        if (!fgets(ligne, sizeof(ligne), stdin)) return;

        char* tok = strtok(ligne, " \n");
        while (tok) {
            int id = atoi(tok);
            for (int i = 0; i < nb_tuiles; i++)
                if (tuiles_joueur[i].id == id)
                    indices[nb_comb++] = i;
            tok = strtok(NULL, " \n");
        }

        if (nb_comb < 3) {
            printf("Au moins 3 tuiles.\n");
            continue;
        }

        Tuile comb[MAX_TUILES];
        for (int i = 0; i < nb_comb; i++)
            comb[i] = tuiles_joueur[indices[i]];

        if (!combinaison_valide(comb, nb_comb)) {
            printf("Combinaison invalide.\n");
            continue;
        }

        /* ---------- PREMIER TOUR ---------- */
        if (premier_tour) {
            int somme = 0, max_val = 0;
            for (int i = 0; i < nb_comb; i++)
                if (!comb[i].joker && comb[i].valeur > max_val)
                    max_val = comb[i].valeur;

            for (int i = 0; i < nb_comb; i++)
                somme += comb[i].joker ? max_val : comb[i].valeur;

            somme_cumulee += somme;

            FILE* ft = fopen("tampon.json", "r+");
            fseek(ft, -2, SEEK_END);
            if (ftell(ft) > 2) fprintf(ft, ",\n");

            fprintf(ft, "  [\n");
            for (int i = 0; i < nb_comb; i++)
                fprintf(ft,
                    "    {\"id\":%d,\"valeur\":%d,\"couleur\":\"%c\",\"joker\":%s}%s\n",
                    comb[i].id, comb[i].valeur, comb[i].couleur,
                    comb[i].joker ? "true" : "false",
                    (i < nb_comb - 1) ? "," : "");
            fprintf(ft, "  ]\n]\n");
            fclose(ft);

            printf("Somme cumulée = %d\n", somme_cumulee);

            printf("Encore une combinaison ? (o/n) ");
            fgets(ligne, sizeof(ligne), stdin);

            if (ligne[0] == 'o' || ligne[0] == 'O')
                continue;

            if (somme_cumulee < 30) {
                printf("Premier tour raté.\n");
                piocher_tuile(j);
                return;
            }

            /* 🔥 CORRECTION MAJEURE ICI 🔥 */
            FILE* ftam = fopen("tampon.json", "r");
            FILE* ftab = fopen("table.json", "w");

            char buf[512];
            while (fgets(buf, sizeof(buf), ftam))
                fputs(buf, ftab);

            fclose(ftam);
            fclose(ftab);

            /* retirer les tuiles jouées */
            Tuile restantes[MAX_TUILES];
            int nb_restantes = 0;

            for (int i = 0; i < nb_tuiles; i++) {
                bool garder = true;
                FILE* ft2 = fopen("tampon.json", "r");
                char b[256];
                while (fgets(b, sizeof(b), ft2)) {
                    int id;
                    if (sscanf(b, " {\"id\":%d", &id) == 1 &&
                        id == tuiles_joueur[i].id) {
                        garder = false;
                        break;
                    }
                }
                fclose(ft2);
                if (garder) restantes[nb_restantes++] = tuiles_joueur[i];
            }

            sauvegarder_chevalet(j.chevalet, j, restantes, nb_restantes);
            printf("Premier tour validé.\n");
            return;
        }

        /* ---------- TOURS SUIVANTS ---------- */
        ajouter_a_table(comb, nb_comb);

        Tuile restantes[MAX_TUILES];
        int nb_restantes = 0;
        for (int i = 0; i < nb_tuiles; i++) {
            bool garder = true;
            for (int k = 0; k < nb_comb; k++)
                if (i == indices[k]) garder = false;
            if (garder) restantes[nb_restantes++] = tuiles_joueur[i];
        }

        sauvegarder_chevalet(j.chevalet, j, restantes, nb_restantes);
        printf("Combinaison posée.\n");

        printf("Encore une ? (o/n) ");
        fgets(ligne, sizeof(ligne), stdin);
        if (ligne[0] != 'o' && ligne[0] != 'O')
            return;
    }
}
