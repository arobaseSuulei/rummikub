#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "raylib.h"
#include <cjson/cJSON.h>
#include "game_state.h"

// --- INCLUSIONS DU BACKEND ---
#include "../src/struct.h"      // Structures Tuile, Joueur
#include "../src/joueur.h"      // Fonctions joueur
#include "../src/Tuile.h"       // Fonctions tuile
#include "../src/table.h"       // Fonctions table
#include "../src/partie.h"      // Fonctions partie
#include "../src/manipulation.h" // Fonctions manipulation

extern void creer_pioche(void);
extern void creer_table(void);
extern void piocher_tuile(Joueur* j);

// --- PROTOTYPES MANQUANTS ---
bool partie_en_cours(void);
void charger_joueur(Joueur *j);
void charger_chevalet(const char *filename, Tuile tuiles[], int *nb);
void sauvegarder_chevalet(const char *filename, Joueur j, Tuile tuiles[], int nb, bool premier_tour_valide);
void passer_tour(Joueur *current, Joueur *next);

// --- 1. VARIABLES GLOBALES STATIQUES ---
static Texture2D fond_accueil, btn_jouer, btn_scores, btn_aide, btn_quitter;
static Font gameFont;
static Texture2D btn_hvh, btn_hvs, fond_modale, fond_config;
static Rectangle rect_hvh, rect_hvs, rect_2P, rect_3P, rect_4P;
static Rectangle rect_jouer, rect_scores, rect_aide, rect_quitter;
static Texture2D fond_plateau;
static Texture2D tex_chevalet;
static Texture2D tex_joueurs[4];
static float gridStartX, gridStartY, gridWidth, gridHeight, cellW, cellH;

// Variables pour la synchronisation avec le backend
static Joueur backend_players[4];
static int backend_num_players = 0;
static bool backend_partie_en_cours = false;
static bool backend_manipulation_mode = false;
static char backend_current_player_file[50] = "";

Vector2 GetGridPos(int r, int c);

// Tableaux de textures pour les tuiles (Visibles par tout le fichier)
Texture2D tex_tuiles[4][14]; 
Texture2D tex_jokers[2];

// --- 2. FONCTIONS BRIDGE BACKEND-RAYLIB ---

// Fonction pour convertir une Tile (interface) en Tuile (backend)
Tuile TileToBackendTile(Tile t) {
    Tuile backend_tile;
    backend_tile.id = 0; // L'ID sera assigné par le backend
    backend_tile.valeur = t.valeur;
    backend_tile.couleur = t.couleur;
    backend_tile.joker = t.joker;
    return backend_tile;
}

// Fonction pour convertir une Tuile (backend) en Tile (interface)
Tile BackendTileToTile(Tuile backend_tile) {
    Tile tile;
    tile.valeur = backend_tile.valeur;
    tile.couleur = backend_tile.couleur;
    tile.joker = backend_tile.joker;
    return tile;
}

// Fonction pour obtenir l'index du joueur dont c'est le tour
int BackendGetCurrentPlayerIndex(void) {
    for (int i = 0; i < backend_num_players; i++) {
        if (backend_players[i].tour) {
            return i;
        }
    }
    return 0; // Par défaut, premier joueur
}

// Charger les joueurs depuis le backend
bool BackendLoadPlayers(GameData *game) {
    // Vérifier si une partie est en cours
    backend_partie_en_cours = partie_en_cours();
    
    if (!backend_partie_en_cours) {
        printf("[BACKEND] Aucune partie en cours\n");
        return false;
    }
    
    // Lire le nombre de joueurs
    FILE* f = fopen("nombre_joueurs.txt", "r");
    if (!f) {
        printf("[BACKEND] Erreur: nombre_joueurs.txt non trouvé\n");
        return false;
    }
    fscanf(f, "%d", &backend_num_players);
    fclose(f);
    
    if (backend_num_players < 1 || backend_num_players > 4) {
        printf("[BACKEND] Nombre de joueurs invalide: %d\n", backend_num_players);
        return false;
    }
    
    // Charger chaque joueur
    for (int i = 0; i < backend_num_players; i++) {
        sprintf(backend_players[i].chevalet, "%d.json", i + 1);
        charger_joueur(&backend_players[i]);
        
        // Copier le pseudo dans game->players
        strncpy(game->players[i].pseudo, backend_players[i].pseudo, 22);
        game->players[i].pseudo[22] = '\0';
        
        // Marquer le joueur dont c'est le tour
        game->players[i].tour = backend_players[i].tour;
        
        printf("[BACKEND] Joueur %d chargé: %s (tour: %s)\n", 
               i + 1, backend_players[i].pseudo, 
               backend_players[i].tour ? "OUI" : "NON");
    }
    
    // Synchroniser le nombre de joueurs
    game->num_players = backend_num_players;
    
    return true;
}

// Charger les tuiles d'un joueur depuis le backend
void BackendLoadPlayerTiles(GameData *game, int player_index) {
    if (player_index < 0 || player_index >= backend_num_players) {
        printf("[BACKEND] Index joueur invalide: %d\n", player_index);
        return;
    }
    
    Tuile backend_tuiles[MAX_TUILES];
    int nb_tuiles = 0;
    
    charger_chevalet(backend_players[player_index].chevalet, backend_tuiles, &nb_tuiles);
    
    // Convertir et stocker dans game->players
    game->players[player_index].nb_tuiles = (nb_tuiles > 30) ? 30 : nb_tuiles;
    
    for (int t = 0; t < game->players[player_index].nb_tuiles; t++) {
        game->players[player_index].main[t] = BackendTileToTile(backend_tuiles[t]);
    }
    
    printf("[BACKEND] %d tuiles chargées pour %s\n", 
           game->players[player_index].nb_tuiles, 
           backend_players[player_index].pseudo);
}

// Sauvegarder les tuiles d'un joueur dans le backend
void BackendSavePlayerTiles(GameData *game, int player_index) {
    if (player_index < 0 || player_index >= backend_num_players) {
        printf("[BACKEND] Index joueur invalide: %d\n", player_index);
        return;
    }
    
    // Convertir les tuiles interface -> backend
    Tuile backend_tuiles[MAX_TUILES];
    for (int t = 0; t < game->players[player_index].nb_tuiles; t++) {
        backend_tuiles[t] = TileToBackendTile(game->players[player_index].main[t]);
        // Conserver l'ID si possible (pour la manipulation)
        // Dans une vraie implémentation, il faudrait garder trace des IDs
    }
    
    // Sauvegarder (simplifié - dans la réalité il faudrait gérer premier_tour_valide)
    sauvegarder_chevalet(backend_players[player_index].chevalet, 
                        backend_players[player_index], 
                        backend_tuiles, 
                        game->players[player_index].nb_tuiles, 
                        false); // premier_tour_valide à false par défaut
}

// Initialiser une nouvelle partie dans le backend
bool BackendInitNewGame(int num_players, const char* player_names[]) {
    printf("[BACKEND] Initialisation nouvelle partie avec %d joueurs\n", num_players);
    
    // 1. Nettoyer
    system("rm -f nombre_joueurs.txt pioche.json table.json *.json");
    
    // 2. Créer pioche et table
    creer_pioche();
    creer_table();
    
    // 3. Écrire nombre_joueurs.txt MANUELLEMENT
    FILE* f = fopen("nombre_joueurs.txt", "w");
    if (f) {
        fprintf(f, "%d", num_players);
        fclose(f);
    }
    
    // 4. Créer fichiers joueurs MANUELLEMENT (évite creer_joueur)
    for (int i = 0; i < num_players; i++) {
        char filename[20];
        sprintf(filename, "%d.json", i + 1);
        
        cJSON *root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "pseudo", player_names[i]);
        cJSON_AddBoolToObject(root, "tour", (i == 0));
        cJSON_AddBoolToObject(root, "premier_tour", true);
        cJSON_AddItemToObject(root, "tuiles", cJSON_CreateArray());
        
        char *str = cJSON_Print(root);
        FILE *fj = fopen(filename, "w");
        if (fj) {
            fprintf(fj, "%s", str);
            fclose(fj);
        }
        free(str);
        cJSON_Delete(root);
    }
    
    // 5. Distribuer 14 tuiles à chaque joueur MANUELLEMENT
    for (int i = 0; i < 14; i++) {
        for (int j = 0; j < num_players; j++) {
            char filename[20];
            sprintf(filename, "%d.json", j + 1);
            
            // Lire fichier joueur
            FILE* fj = fopen(filename, "r");
            if (!fj) continue;
            
            fseek(fj, 0, SEEK_END);
            long size = ftell(fj);
            fseek(fj, 0, SEEK_SET);
            char* data = malloc(size + 1);
            fread(data, 1, size, fj);
            data[size] = 0;
            fclose(fj);
            
            // Ajouter une tuile
            cJSON* root = cJSON_Parse(data);
            free(data);
            
            if (root) {
                // Simuler pioche (tuile simple)
                cJSON* tuiles = cJSON_GetObjectItem(root, "tuiles");
                cJSON* tile = cJSON_CreateObject();
                cJSON_AddNumberToObject(tile, "id", i*10 + j);
                cJSON_AddNumberToObject(tile, "valeur", (i % 13) + 1);
                char couleur = "RBOV"[j % 4];
                char colStr[2] = {couleur, 0};
                cJSON_AddStringToObject(tile, "couleur", colStr);
                cJSON_AddBoolToObject(tile, "joker", false);
                cJSON_AddItemToArray(tuiles, tile);
                
                // Sauvegarder
                char* new_data = cJSON_Print(root);
                fj = fopen(filename, "w");
                if (fj) {
                    fprintf(fj, "%s", new_data);
                    fclose(fj);
                }
                free(new_data);
                cJSON_Delete(root);
            }
        }
    }
    
    return true;
}

// Passer au joueur suivant
void BackendPassTurn(int current_player_index) {
    int next_player_index = (current_player_index + 1) % backend_num_players;
    passer_tour(&backend_players[current_player_index], &backend_players[next_player_index]);
}

// --- 3. FONCTIONS DE CHARGEMENT ET UTILITAIRES ---

Vector2 GetGridPos(int r, int c) {
    float x = gridStartX + (c * cellW) + (cellW / 2.0f);
    float y = gridStartY + (r * cellH) + (cellH / 2.0f);
    return (Vector2){ x, y };
}

void DrawCustomTile(float x, float y, float w, float h, Tile t, bool selected) {
    Rectangle rect = { x, y, w, h };
    
    // 1. Corps de la tuile (Couleur Ivoire / Beige clair)
    Color tileColor = (Color){ 252, 245, 220, 255 }; 
    if (selected) tileColor = (Color){ 255, 230, 100, 255 }; // Jaune si sélectionnée

    // Ombre portée légère
    DrawRectangleRounded((Rectangle){ x + 2, y + 2, w, h }, 0.15f, 10, (Color){ 0, 0, 0, 60 });
    
    // Dessin du rectangle principal
    DrawRectangleRounded(rect, 0.15f, 10, tileColor);
    
    // Bordure pour l'effet de relief
    DrawRectangleRoundedLinesEx(rect, 0.15f, 10, 2, (Color){ 180, 160, 130, 255 });

    // 2. Le cercle central blanc (très léger)
    DrawCircle(x + w/2.0f, y + h/2.0f - 5, w/2.8f, (Color){ 255, 255, 255, 180 });

    // 3. Détermination de la couleur selon la lettre
    Color txtCol;
    switch(t.couleur) {
        case 'R': case 'r': txtCol = (Color){ 210, 30, 30, 255 }; break;
        case 'B': case 'b': txtCol = (Color){ 30, 80, 180, 255 }; break;
        case 'J': case 'j': case 'O': txtCol = (Color){ 230, 180, 0, 255 }; break;
        case 'N': case 'n': txtCol = (Color){ 40, 40, 40, 255 }; break;
        default: txtCol = DARKGRAY;
    }

    // 4. Contenu : Chiffre OU Joker
    if (t.joker) {
        Vector2 center = { x + w / 2.0f, y + h / 2.0f - 5 };
        float eyeOffset = w * 0.18f;
        float eyeRadius = w * 0.07f;

        // Yeux
        DrawCircleV((Vector2){ center.x - eyeOffset, center.y - eyeOffset }, eyeRadius, txtCol);
        DrawCircleV((Vector2){ center.x + eyeOffset, center.y - eyeOffset }, eyeRadius, txtCol);
        // Nez
        DrawCircleV(center, eyeRadius * 0.7f, txtCol);
        // Sourire (Arc de cercle)
        DrawRing(center, w * 0.20f, w * 0.25f, 40, 140, 20, txtCol);
        // Chapeau de bouffon
        Vector2 p1 = { center.x, center.y - w * 0.45f };
        Vector2 p2 = { center.x - w * 0.30f, center.y - w * 0.20f };
        Vector2 p3 = { center.x + w * 0.30f, center.y - w * 0.20f };
        DrawTriangle(p1, p2, p3, txtCol);
    } 
    else {
        const char* valStr = TextFormat("%d", t.valeur);
        int fontSize = (int)(h * 0.45f);
        int textWidth = MeasureText(valStr, fontSize);
        DrawText(valStr, x + (w - textWidth)/2.0f, y + (h - fontSize)/2.0f - 5, fontSize, txtCol);
    }

    // 5. Signature
    DrawText("Rummikub", x + w/2.0f - 18, y + h - 18, 7, (Color){ 160, 140, 110, 150 });
}

void DrawBtnInternal(Texture2D t, Rectangle r, Vector2 m) {
    if (t.id != 0) {
        bool h = CheckCollisionPointRec(m, r);
        DrawTexturePro(t, (Rectangle){0,0,(float)t.width,(float)t.height}, r, (Vector2){0,0}, 0.0f, h ? (Color){255,255,255,200} : WHITE);
    }
}

void LoadAndClean(Texture2D *tex, const char *fileName, char *basePath) {
    char p[512];
    sprintf(p, "%s/ui/%s", basePath, fileName);
    Image img = LoadImage(p);
    if (img.data != NULL) {
        ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
        Color *pixels = (Color *)img.data;
        for (int i = 0; i < img.width * img.height; i++) {
            int diffRG = abs(pixels[i].r - pixels[i].g);
            int diffRB = abs(pixels[i].r - pixels[i].b);
            if (diffRG < 20 && diffRB < 20 && pixels[i].r > 80) pixels[i] = BLANK;
        }
        *tex = LoadTextureFromImage(img);
        UnloadImage(img);
    }
}

// --- 4. LOGIQUE ET ETATS DU JEU ---

void InitGame(GameData *game) {
    char base_path[256] = "/home/maelle/Documents/rummikub/assets/images/";
    char full_path[512];
    
    game->selected_tile_idx = -1; // -1 signifie "aucune tuile sélectionnée"

    gameFont = LoadFontEx("/home/maelle/Downloads/Inter_28pt-Italic.ttf", 100, 0, 250); 
    SetTextureFilter(gameFont.texture, TEXTURE_FILTER_BILINEAR);

    sprintf(full_path, "%s/fonds/accueil.png", base_path);
    fond_accueil = LoadTexture(full_path);
    sprintf(full_path, "%s/fonds/fond.png", base_path); 
    fond_modale = LoadTexture(full_path);
    sprintf(full_path, "%s/fonds/fond_1.png", base_path);
    fond_config = LoadTexture(full_path);
    sprintf(full_path, "%s/fonds/plateau.png", base_path);
    fond_plateau = LoadTexture(full_path);

    LoadAndClean(&btn_aide, "bouton_aide.png", base_path);
    LoadAndClean(&btn_scores, "bouton_scores.png", base_path);
    LoadAndClean(&btn_quitter, "bouton_quitter.png", base_path);
    LoadAndClean(&btn_hvh, "bouton_hvh.png", base_path);
    LoadAndClean(&btn_hvs, "bouton_hvs.png", base_path);
    
    sprintf(full_path, "%s/ui/bouton_jouer.png", base_path);
    btn_jouer = LoadTexture(full_path);

    rect_aide    = (Rectangle){ (float)game->screen_width - 90, 50, 60, 60 };
    rect_scores  = (Rectangle){ (float)game->screen_width - 150, 130, 120, 120 };
    rect_quitter = (Rectangle){ (float)game->screen_width - 150, 270, 120, 65 };
    rect_jouer   = (Rectangle){ (game->screen_width - 500)/2.0f, (game->screen_height/2.0f) + 180, 500, 180};

    float boxX = (game->screen_width - 600) / 2.0f;
    float boxY = (game->screen_height - 400) / 2.0f;
    rect_hvs = (Rectangle){ boxX + 80,  boxY + 140, 160, 160 };
    rect_hvh = (Rectangle){ boxX + 360, boxY + 140, 160, 160 };
    
    for (int i = 0; i < 4; i++) {
        sprintf(full_path, "%s/ui/player_%d.png", base_path, i + 1);
        tex_joueurs[i] = LoadTexture(full_path);
    }

    gridStartX = 315.6f;  // Ajusté pour le bord gauche
    gridStartY = 207.0f; // Ajusté pour le bord haut
    gridWidth  = 992.2f;  // Largeur utile
    gridHeight = 525.0f; // Hauteur utile (9 lignes)

    cellW = gridWidth / 17.0f;
    cellH = gridHeight / 9.0f;
    
    // Initialiser le backend si une partie existe
    backend_partie_en_cours = partie_en_cours();
    if (backend_partie_en_cours) {
        printf("[INIT] Partie backend détectée, chargement...\n");
        BackendLoadPlayers(game);
        
        // Charger les tuiles du joueur actuel
        int current_player = BackendGetCurrentPlayerIndex();
        BackendLoadPlayerTiles(game, current_player);
    }
}

void ResetPlayersData(GameData *game) {
    for (int i = 0; i < 4; i++) {
        game->player_names[i][0] = '\0';      // Vide la chaîne de caractères
        game->name_char_counts[i] = 0;        // Remet le compteur à zéro
    }
    game->editing_player = 0;
}

void StartGameLogic(GameData *game) {
    printf("[START] Création nouvelle partie avec %d joueurs\n", game->num_players);
    
    const char* player_names[4];
    for (int i = 0; i < game->num_players; i++) {
        player_names[i] = game->player_names[i];
    }
    
    if (game->num_players == 1) {
        player_names[1] = "ORDINATEUR";
        BackendInitNewGame(2, player_names);
        game->num_players = 2;
    } else {
        BackendInitNewGame(game->num_players, player_names);
    }
    
    // Charger depuis backend
    BackendLoadPlayers(game);
    int current = BackendGetCurrentPlayerIndex();
    BackendLoadPlayerTiles(game, current);
    
    game->players[0].tour = true;
    game->current_state = STATE_PLAYING;
}

void UpdateGame(GameData *game) {
    Vector2 mouse = GetMousePosition();
    
    // Quitter avec Echap
    if (IsKeyPressed(KEY_ESCAPE)) {
        game->should_close = true;
        return;
    }

    // 1. GESTION DU MENU PRINCIPAL
    if (game->current_state == STATE_MENU) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(mouse, rect_jouer)) {
                ResetPlayersData(game);
                game->current_state = STATE_CHOOSE_MODE;
            }
            else if (CheckCollisionPointRec(mouse, rect_aide)) {
                game->current_state = STATE_HELP;
            }
            else if (CheckCollisionPointRec(mouse, rect_quitter)) {
                game->should_close = true;
            }
        }
        
        // Vérifier si une partie backend existe
        if (!backend_partie_en_cours && partie_en_cours()) {
            backend_partie_en_cours = true;
            printf("[UPDATE] Partie backend détectée\n");
        }
    } 
    
    // 2. SAISIE DES NOMS
    else if (game->current_state == STATE_INPUT_NAMES) {
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 32) && (key <= 125) && (game->name_char_counts[game->editing_player] < 15)) {
                game->player_names[game->editing_player][game->name_char_counts[game->editing_player]] = (char)key;
                game->player_names[game->editing_player][game->name_char_counts[game->editing_player] + 1] = '\0';
                game->name_char_counts[game->editing_player]++;
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE)) {
            if (game->name_char_counts[game->editing_player] > 0) {
                game->name_char_counts[game->editing_player]--;
                game->player_names[game->editing_player][game->name_char_counts[game->editing_player]] = '\0';
            }
        }
    }
    
    // 3. GESTION DE L'AIDE
    else if (game->current_state == STATE_HELP) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            int bw = 800, bh = 550;
            float boxY = (game->screen_height - bh) / 2.0f;
            Rectangle btnClose = { (game->screen_width - 250)/2.0f, boxY + 460, 250, 50 };
            if (CheckCollisionPointRec(mouse, btnClose)) game->current_state = STATE_MENU; 
        }
    }
    
    // 4. GESTION DU CHOIX DU MODE
    else if (game->current_state == STATE_CHOOSE_MODE) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(mouse, rect_hvs)) {
                game->num_players = 1;
                game->editing_player = 0;
                TextCopy(game->player_names[0], "Joueur 1"); 
                game->name_char_counts[0] = 8;
                TextCopy(game->player_names[1], "ORDINATEUR");
                game->name_char_counts[1] = 10;
                game->current_state = STATE_INPUT_NAMES;
            } 
            else if (CheckCollisionPointRec(mouse, rect_hvh)) {
                game->current_state = STATE_CONFIG;
            }
            else {
                Rectangle box = {(game->screen_width - 600)/2.0f, (game->screen_height - 400)/2.0f, 600, 400};
                if (!CheckCollisionPointRec(mouse, box)) game->current_state = STATE_MENU;
            }
        }
    }
    
    // 5. GESTION DE LA CONFIGURATION
    else if (game->current_state == STATE_CONFIG) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            bool selection_faite = false;
            if (CheckCollisionPointRec(mouse, rect_2P)) { game->num_players = 2; selection_faite = true; }
            else if (CheckCollisionPointRec(mouse, rect_3P)) { game->num_players = 3; selection_faite = true; }
            else if (CheckCollisionPointRec(mouse, rect_4P)) { game->num_players = 4; selection_faite = true; }

            if (selection_faite) {
                game->editing_player = 0;
                game->current_state = STATE_INPUT_NAMES;
            }
        }
    }
    
    // 6. GESTION DU JEU EN COURS
    else if (game->current_state == STATE_PLAYING) {
        int p = 0;
        for (int i = 0; i < 4; i++) {
            if (game->players[i].tour) { 
                p = i; 
                break; 
            }
        }

        Vector2 mouse = GetMousePosition();

        // --- CLIC GAUCHE : ON ATTRAPE LA TUILE ---
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            float startX = 80 + 25; 
            float startY = (float)game->screen_height - 185 + 15;
            float spacingX = 62.0f;
            float spacingY = 82.0f;
            float tileW = 55.0f;
            float tileH = 75.0f;

            for (int t = 0; t < game->players[p].nb_tuiles; t++) {
                float posX = startX + (t % 15) * spacingX;
                float posY = startY + (t / 15) * spacingY;
                Rectangle tileRect = { posX, posY, tileW, tileH };

                if (CheckCollisionPointRec(mouse, tileRect)) {
                    game->selected_tile_idx = t;
                    game->dragging_tile_idx = t;
                    game->is_dragging = true;
                    game->drag_offset.x = mouse.x - posX;
                    game->drag_offset.y = mouse.y - posY;
                    break;
                }
            }
        }

        // --- RELÂCHEMENT : ON POSE LA TUILE ---
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            if (game->is_dragging && game->dragging_tile_idx != -1) {
                
                // On calcule la position RELATIVE (on retire la marge de l'image)
                float relativeX = mouse.x - gridStartX;
                float relativeY = mouse.y - gridStartY;

                int col = (int)(relativeX / cellW);
                int row = (int)(relativeY / cellH);

                // On vérifie si on est bien à l'intérieur de la grille
                if (col >= 0 && col < 17 && row >= 0 && row < 9 && relativeX >= 0 && relativeY >= 0) {
                    if (!game->board.occupee[row][col]) {
                        // Poser la tuile sur le plateau visuel
                        game->board.grille[row][col] = game->players[p].main[game->dragging_tile_idx];
                        game->board.occupee[row][col] = true;

                        // Pour une vraie intégration, il faudrait:
                        // 1. Créer une combinaison avec le backend
                        // 2. Valider la combinaison
                        // 3. Sauvegarder dans table.json
                        
                        // Pour l'instant, on retire juste de la main visuelle
                        for (int i = game->dragging_tile_idx; i < game->players[p].nb_tuiles - 1; i++) {
                            game->players[p].main[i] = game->players[p].main[i + 1];
                        }
                        game->players[p].nb_tuiles--;
                        
                        printf("[GAME] Tuile posée à (%d, %d)\n", row, col);
                    }
                }
            }
            game->is_dragging = false;
            game->dragging_tile_idx = -1;
            game->selected_tile_idx = -1;
        }
        
        // --- BOUTONS D'ACTION BACKEND ---
        Rectangle btnPiocher = { game->screen_width - 200, 70, 180, 40 };
        Rectangle btnValider = { game->screen_width - 200, 120, 180, 40 };
        Rectangle btnManipuler = { game->screen_width - 200, 170, 180, 40 };
        
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            // Bouton Piocher
            if (CheckCollisionPointRec(mouse, btnPiocher)) {
                printf("[ACTION] Pioche demandée pour joueur %d\n", p);
                // Ici on appellerait piocher_tuile() du backend
                // piocher_tuile(&backend_players[p]);
                // BackendLoadPlayerTiles(game, p);
            }
            
            // Bouton Valider tour
            if (CheckCollisionPointRec(mouse, btnValider)) {
                printf("[ACTION] Validation tour demandée\n");
                // Sauvegarder l'état actuel
                BackendSavePlayerTiles(game, p);
                // Passer au joueur suivant
                BackendPassTurn(p);
                // Recharger le nouveau joueur
                int new_player = BackendGetCurrentPlayerIndex();
                BackendLoadPlayerTiles(game, new_player);
                
                // Mettre à jour l'affichage
                for (int i = 0; i < backend_num_players; i++) {
                    game->players[i].tour = (i == new_player);
                }
            }
            
            // Bouton Manipuler
            if (CheckCollisionPointRec(mouse, btnManipuler)) {
                printf("[ACTION] Mode manipulation\n");
                backend_manipulation_mode = true;
                // Ici on pourrait ouvrir un menu de manipulation
            }
        }
    }
    
    // --- MODE CALIBRAGE TEMPORAIRE ---
    if (IsKeyDown(KEY_RIGHT)) gridStartX += 1.0f;
    if (IsKeyDown(KEY_LEFT))  gridStartX -= 1.0f;
    if (IsKeyDown(KEY_UP))    gridStartY -= 1.0f;
    if (IsKeyDown(KEY_DOWN))  gridStartY += 1.0f;
    if (IsKeyDown(KEY_W))     gridWidth += 1.0f;
    if (IsKeyDown(KEY_S))     gridWidth -= 1.0f;

    cellW = gridWidth / 17.0f;
    cellH = gridHeight / 9.0f;
}

void DrawGame(GameData *game) {
    Vector2 mouse = GetMousePosition();
    
    // Déclaration du bouton retour (scope supérieur)
    Rectangle rect_retour = { 20, 20, 100, 30 };
    
    // 1. Fond principal
    DrawTexturePro(fond_accueil, (Rectangle){0, 0, 2208, 1242},
                  (Rectangle){0, 0, (float)game->screen_width, (float)game->screen_height},
                  (Vector2){0, 0}, 0.0f, WHITE);

    // 2. Boutons du Menu (On les dessine si on n'est pas en plein jeu)
    if (game->current_state != STATE_PLAYING) {
        bool h_j = CheckCollisionPointRec(mouse, rect_jouer);
        DrawTexturePro(btn_jouer, (Rectangle){0,0,(float)btn_jouer.width,(float)btn_jouer.height}, 
                       rect_jouer, (Vector2){0,0}, 0.0f, h_j ? (Color){255,255,255,200} : WHITE);

        DrawBtnInternal(btn_aide, rect_aide, mouse);
        DrawBtnInternal(btn_scores, rect_scores, mouse);
        DrawBtnInternal(btn_quitter, rect_quitter, mouse);
        
        // Afficher un message si une partie backend existe
        if (backend_partie_en_cours) {
            DrawText("Partie en cours détectée", 50, 100, 20, GREEN);
            DrawText("Cliquez sur 'JOUER' pour reprendre", 50, 130, 18, YELLOW);
        }
    }
    
    // 3. ÉCRAN SAISIE DES NOMS
    if (game->current_state == STATE_INPUT_NAMES) {
        DrawRectangle(0, 0, game->screen_width, game->screen_height, (Color){0, 0, 0, 200});
        
        bool noms_valides = true;
        for (int i = 0; i < game->num_players; i++) {
            if (game->name_char_counts[i] == 0) noms_valides = false;
        }
        
        // On définit le bouton LANCER ici pour qu'il soit accessible partout dans ce bloc
        Rectangle btnLancer = { (float)game->screen_width/2 - 100, 550, 200, 60 };

        for (int i = 0; i < game->num_players; i++) {
            Rectangle inputRect = { (float)game->screen_width/2 - 150, 200.0f + (i * 80), 300, 50 };
            
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, inputRect)) {
                game->editing_player = i;
            }

            DrawRectangleRec(inputRect, (game->editing_player == i) ? RAYWHITE : LIGHTGRAY);
            DrawRectangleLinesEx(inputRect, 2, (game->editing_player == i) ? ORANGE : DARKGRAY);
            DrawText(game->player_names[i], inputRect.x + 10, inputRect.y + 15, 20, BLACK);
            DrawText(TextFormat("JOUEUR %d :", i + 1), inputRect.x - 120, inputRect.y + 15, 20, WHITE);
        }

        // Dessin et clic du bouton Lancer
        Color couleurBouton = noms_valides ? GREEN : DARKGRAY;
        DrawRectangleRounded(btnLancer, 0.2f, 10, couleurBouton);
        DrawText("LANCER !", btnLancer.x + 50, btnLancer.y + 20, 25, WHITE);

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, btnLancer)) {
            if (noms_valides) {
                StartGameLogic(game); 
                game->current_state = STATE_PLAYING;
            }
        }
        if (!noms_valides) DrawText("Entrez tous les noms pour continuer", btnLancer.x - 50, btnLancer.y + 75, 18, MAROON);
    }

    // 4. Modale AIDE
    if (game->current_state == STATE_HELP) {
        DrawRectangle(0, 0, game->screen_width, game->screen_height, (Color){10, 10, 10, 230});
        int bw = 800, bh = 550;
        Rectangle box = {(game->screen_width - bw)/2.0f, (game->screen_height - bh)/2.0f, (float)bw, (float)bh};
        
        DrawRectangleRec(box, WHITE);
        DrawRectangleLinesEx(box, 3, DARKBLUE);
        DrawTextEx(gameFont, "REGLES DU RUMMIKUB", (Vector2){box.x + 230, box.y + 30}, 32, 1, DARKBLUE);

        float tx = box.x + 40;
        DrawTextEx(gameFont, "BUT DU JEU", (Vector2){tx, box.y + 90}, 24, 1, MAROON);
        DrawTextEx(gameFont, "Poser toutes vos tuiles sur la table avant les autres.", (Vector2){tx + 10, box.y + 120}, 20, 1, BLACK);
        DrawTextEx(gameFont, "LES COMBINAISONS (3 tuiles minimum)", (Vector2){tx, box.y + 160}, 24, 1, MAROON);
        DrawTextEx(gameFont, "- SUITE : Chiffres qui se suivent (ex: 1, 2, 3 bleus).", (Vector2){tx + 10, box.y + 195}, 19, 1, BLACK);
        DrawTextEx(gameFont, "- GROUPE : Meme chiffre, couleurs differentes", (Vector2){tx + 10, box.y + 225}, 19, 1, BLACK);
        DrawTextEx(gameFont, "(ex: 8 rouge, 8 noir, 8 jaune, 8 bleu).", (Vector2){tx + 100, box.y + 250}, 17, 1, DARKGRAY);
        DrawTextEx(gameFont, "LA REGLE DES 30 POINTS", (Vector2){tx, box.y + 300}, 24, 1, MAROON);
        DrawTextEx(gameFont, "Votre premiere pose doit totaliser au moins 30 points.", (Vector2){tx + 10, box.y + 330}, 19, 1, BLACK);

        Rectangle btn = {(game->screen_width - 250)/2.0f, box.y + 460, 250, 50};
        DrawRectangleRec(btn, CheckCollisionPointRec(mouse, btn) ? ORANGE : DARKBLUE);
        DrawTextEx(gameFont, "J'AI COMPRIS", (Vector2){btn.x + 45, btn.y + 12}, 22, 1, WHITE);
    }
    
    // 5. Modale CHOIX MODE
    if (game->current_state == STATE_CHOOSE_MODE) {
        DrawRectangle(0, 0, game->screen_width, game->screen_height, (Color){0, 0, 0, 180});
        Rectangle box = {(game->screen_width - 600)/2.0f, (game->screen_height - 400)/2.0f, 600, 400};
        DrawTexturePro(fond_modale, (Rectangle){0, 0, 450, 300}, box, (Vector2){0,0}, 0.0f, WHITE);
        DrawTexturePro(btn_hvs, (Rectangle){0, 0, 512, 512}, rect_hvs, (Vector2){0,0}, 0.0f, CheckCollisionPointRec(mouse, rect_hvs) ? WHITE : (Color){255, 255, 255, 200});
        DrawTexturePro(btn_hvh, (Rectangle){0, 0, 512, 512}, rect_hvh, (Vector2){0,0}, 0.0f, CheckCollisionPointRec(mouse, rect_hvh) ? WHITE : (Color){255, 255, 255, 200});
    }

    // 6. Modale CONFIG (Avec ton centrage optimisé)
    if (game->current_state == STATE_CONFIG) {
        DrawRectangle(0, 0, game->screen_width, game->screen_height, (Color){0, 0, 0, 180});
        float imgW = 463.0f, imgH = 260.0f;
        Rectangle box = {(game->screen_width - imgW)/2.0f, (game->screen_height - imgH)/2.0f, imgW, imgH};
        DrawTexturePro(fond_config, (Rectangle){0, 0, imgW, imgH}, box, (Vector2){0,0}, 0.0f, WHITE);
        
        rect_2P = (Rectangle){ box.x + 50,  box.y + 100, 80, 80 };
        rect_3P = (Rectangle){ box.x + 190, box.y + 100, 80, 80 };
        rect_4P = (Rectangle){ box.x + 330, box.y + 100, 80, 80 };

        Rectangle rs[] = {rect_2P, rect_3P, rect_4P};
        const char* ls[] = {"2", "3", "4"};
        Color colors[] = {RED, DARKBLUE, ORANGE}; 

        for(int i = 0; i < 3; i++) {
            bool h = CheckCollisionPointRec(mouse, rs[i]);
            Color couleurIvoire = (Color){ 225, 210, 180, 255 };
            DrawRectangleRounded(rs[i], 0.15f, 10, h ? (Color){200, 185, 155, 255} : couleurIvoire);
            
            // Ton calcul de centrage
            float fontSize = 50.0f;
            Vector2 textSize = MeasureTextEx(gameFont, ls[i], fontSize, 1);
            Vector2 positionTexte = { 
                (rs[i].x + rs[i].width/2.0f) - (textSize.x/2.0f) - 8, 
                (rs[i].y + rs[i].height/2.0f) - (textSize.y/2.0f) - 12 
            };
            DrawTextEx(gameFont, ls[i], positionTexte, fontSize, 1, colors[i]);
        }
    }

    // --- 7. STATE PLAYING : PLACES FIXES AUTOUR DE LA TABLE ---
    if (game->current_state == STATE_PLAYING) {
        DrawTexturePro(fond_plateau, (Rectangle){ 0, 0, 1536, 1024 }, 
                       (Rectangle){ 0, 0, (float)game->screen_width, (float)game->screen_height }, (Vector2){ 0, 0 }, 0.0f, WHITE);

        int p = 0; 
        for (int i = 0; i < 4; i++) if (game->players[i].tour) { p = i; break; }
        
        // --- DESSIN DE LA GRILLE VISUELLE ---
        Color gridColor = (Color){ 180, 180, 220, 100 }; // Bleu clair transparent
        
        // Dessiner les lignes verticales
        for (int c = 0; c <= 17; c++) {
            float x = gridStartX + (c * cellW);
            DrawLineEx((Vector2){ x, gridStartY }, 
                      (Vector2){ x, gridStartY + gridHeight }, 
                      1.5f, gridColor);
        }
        
        // Dessiner les lignes horizontales
        for (int r = 0; r <= 9; r++) {
            float y = gridStartY + (r * cellH);
            DrawLineEx((Vector2){ gridStartX, y }, 
                      (Vector2){ gridStartX + gridWidth, y }, 
                      1.5f, gridColor);
        }
        
        // Option : Dessiner un fond légèrement coloré pour chaque cellule
        for (int r = 0; r < 9; r++) {
            for (int c = 0; c < 17; c++) {
                Rectangle cellRect = {
                    gridStartX + (c * cellW),
                    gridStartY + (r * cellH),
                    cellW,
                    cellH
                };
                // Fond très léger pour mieux voir les cellules
                DrawRectangleRec(cellRect, (Color){ 240, 240, 255, 30 });
            }
        }
        
        // --- DESSIN DES TUILES SUR LE PLATEAU ---
        for (int r = 0; r < 9; r++) {
            for (int c = 0; c < 17; c++) {
                if (game->board.occupee[r][c]) {
                    
                    // Taille de la tuile (85% de la cellule pour laisser un petit bord)
                    float tW = cellW * 0.95f;
                    float tH = cellH * 0.95f;

                    // CENTRAGE PARFAIT : 
                    // On part du coin haut-gauche de la case (gridStartX + c*cellW)
                    // Et on ajoute la moitié de l'espace vide pour centrer
                    float px = gridStartX + (c * cellW) + (cellW - tW) / 2.0f;
                    float py = gridStartY + (r * cellH) + (cellH - tH) / 2.0f;
                    
                    DrawCustomTile(px, py, tW, tH, game->board.grille[r][c], false);
                }
            }
        }
        
        // --- AFFICHAGE DES 4 PLACES D'AVATARS ---
        for (int i = 0; i < game->num_players; i++) {
            Rectangle rAdv;
            if (i == 0)      rAdv = (Rectangle){ 30, (float)game->screen_height/2 - 90, 110, 180 };
            else if (i == 1) rAdv = (Rectangle){ (float)game->screen_width/2 - 260, 30, 250, 90 };
            else if (i == 2) rAdv = (Rectangle){ (float)game->screen_width/2 + 10, 30, 250, 90 };
            else             rAdv = (Rectangle){ (float)game->screen_width - 140, (float)game->screen_height/2 - 90, 110, 180 };

            Color borderColor = (i == p) ? GOLD : RAYWHITE;
            DrawRectangleRounded(rAdv, 0.2f, 10, (Color){ 30, 30, 30, 180 });
            DrawRectangleRoundedLines(rAdv, 0.2f, 10, borderColor); 

            float iconSize = 55.0f;
            float iconX = rAdv.x + (rAdv.width - iconSize) / 2;
            float iconY = rAdv.y + 10;

            if (game->num_players == 1 && i == 1) {
                // Si mode HvS et index 1 -> C'est l'ordi
                DrawRectangleRounded((Rectangle){iconX, iconY, iconSize, iconSize}, 0.2f, 5, (Color){ 45, 45, 50, 255 });
                DrawRectangleLinesEx((Rectangle){iconX + 5, iconY + 5, iconSize - 10, iconSize - 10}, 2, SKYBLUE);
                DrawText("CPU", iconX + 13, iconY + 22, 16, SKYBLUE);
            } 
            else if (tex_joueurs[i].id != 0) {
                DrawTexturePro(tex_joueurs[i], (Rectangle){0, 0, (float)tex_joueurs[i].width, (float)tex_joueurs[i].height},
                               (Rectangle){iconX, iconY, iconSize, iconSize}, (Vector2){0,0}, 0.0f, WHITE);
            }
            // --- TEXTE AJUSTÉ DANS LE CADRE ---
            float textX = rAdv.x + 10;
            // On calcule le Y en partant du BAS du cadre (-40 pixels) pour être sûr d'être dedans
            float textBaseY = rAdv.y + rAdv.height - 42; 
            
            // Le Pseudo
            DrawText(game->players[i].pseudo, textX, textBaseY, 15, (i == p) ? ORANGE : WHITE);
            
            // Le badge des tuiles (remonté d'un poil)
            Rectangle badgeTuiles = { textX, textBaseY + 18, 85, 16 };
            DrawRectangleRounded(badgeTuiles, 0.3f, 5, (Color){0, 0, 0, 120}); 
            
            DrawText(TextFormat("%d tuiles", game->players[i].nb_tuiles), textX + 5, textBaseY + 20, 12, LIGHTGRAY);
        }
        
        // --- ZONE BAS : INFOS TOUR ET CHEVALET ---
        const char* label = TextFormat("TOUR DE : %s", game->players[p].pseudo);
        float labelW = (float)MeasureText(label, 22);
        float labelX = (game->screen_width - labelW) / 2.0f;
        float actIconX = labelX - 60;
        float actIconY = (float)game->screen_height - 225;

        // Icône du joueur actif
        if (tex_joueurs[p].id != 0) {
            DrawTexturePro(tex_joueurs[p], (Rectangle){0, 0, (float)tex_joueurs[p].width, (float)tex_joueurs[p].height},
                           (Rectangle){actIconX, actIconY, 50, 50}, (Vector2){0,0}, 0.0f, WHITE);
        } else {
            DrawRectangleRounded((Rectangle){actIconX, actIconY, 50, 50}, 0.2f, 5, DARKGRAY);
            DrawText("CPU", actIconX + 10, actIconY + 18, 14, SKYBLUE);
        }
        
        DrawText(label, labelX, game->screen_height - 210, 22, GOLD);
        
        // --- BOUTONS D'ACTION BACKEND ---
        Rectangle btnPiocher = { game->screen_width - 200, 70, 180, 40 };
        Rectangle btnValider = { game->screen_width - 200, 120, 180, 40 };
        Rectangle btnManipuler = { game->screen_width - 200, 170, 180, 40 };
        
        // Bouton Piocher
        DrawRectangleRec(btnPiocher, CheckCollisionPointRec(mouse, btnPiocher) ? DARKGREEN : GREEN);
        DrawText("PIOCHER", btnPiocher.x + 50, btnPiocher.y + 12, 20, WHITE);
        
        // Bouton Valider tour
        DrawRectangleRec(btnValider, CheckCollisionPointRec(mouse, btnValider) ? DARKBLUE : BLUE);
        DrawText("VALIDER TOUR", btnValider.x + 30, btnValider.y + 12, 18, WHITE);
        
        // Bouton Manipuler
        DrawRectangleRec(btnManipuler, CheckCollisionPointRec(mouse, btnManipuler) ? DARKPURPLE : PURPLE);
        DrawText("MANIPULER", btnManipuler.x + 40, btnManipuler.y + 12, 20, WHITE);
        
        // Info backend
        if (backend_partie_en_cours) {
            DrawText(TextFormat("Backend: %d joueurs", backend_num_players), 
                     game->screen_width - 250, 220, 16, LIGHTGRAY);
        }

        // 1. Dessin du chevalet
        Rectangle rectChevalet = { 80, (float)game->screen_height - 185, (float)game->screen_width - 160, 150 };
        DrawRectangleRounded(rectChevalet, 0.05f, 10, (Color){ 80, 50, 30, 255 }); // Marron plus foncé
        DrawRectangleRoundedLinesEx(rectChevalet, 0.05f, 10, 3, (Color){ 50, 30, 20, 255 });
          
        // 2. Variables de positionnement pour les tuiles dessinées
        float startX = rectChevalet.x + 25;
        float startY = rectChevalet.y + 15;
        float tileW = 55.0f;   // Taille équilibrée
        float tileH = 75.0f; 
        float spacingX = 62.0f;   
        float spacingY = 82.0f;   

        // 3. Boucle de dessin (on n'utilise plus GetTextureFromTile !)
        for (int t = 0; t < game->players[p].nb_tuiles; t++) {
            if (game->is_dragging && t == game->dragging_tile_idx) continue; 
            
            float posX = startX + (t % 15) * spacingX;
            float posY = startY + (t / 15) * spacingY;
            DrawCustomTile(posX, posY, tileW, tileH, game->players[p].main[t], (t == game->selected_tile_idx));
        }

        // --- F. LA TUILE QUI SUIT LA SOURIS (UNE SEULE FOIS) ---
        if (game->is_dragging && game->dragging_tile_idx != -1) {
            Vector2 m = GetMousePosition();
            DrawCustomTile(m.x - game->drag_offset.x, m.y - game->drag_offset.y, 
                           tileW, tileH, game->players[p].main[game->dragging_tile_idx], true);
        }

        // 4. Bouton Retour Menu
        if (CheckCollisionPointRec(mouse, rect_retour) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            game->current_state = STATE_MENU;
        }
        DrawText(" < MENU", 30, 25, 20, WHITE);
        
    } // Fin du bloc STATE_PLAYING

    // Optionnel : Dessiner le curseur (si tu veux)
    DrawCircleV(mouse, 5, RED);
}

void CleanupGame(GameData *game) {
    // Sauvegarder l'état des joueurs si une partie est en cours
    if (backend_partie_en_cours) {
        for (int i = 0; i < backend_num_players; i++) {
            if (game->players[i].nb_tuiles > 0) {
                BackendSavePlayerTiles(game, i);
            }
        }
    }
    
    // Vérifiez toujours si la texture est chargée avant de la décharger
    if (fond_accueil.id != 0) UnloadTexture(fond_accueil);
    if (btn_jouer.id != 0) UnloadTexture(btn_jouer);
    if (btn_scores.id != 0) UnloadTexture(btn_scores);
    if (btn_aide.id != 0) UnloadTexture(btn_aide);
    if (btn_quitter.id != 0) UnloadTexture(btn_quitter);
    
    if (gameFont.texture.id != 0) UnloadFont(gameFont);
    
    if (btn_hvh.id != 0) UnloadTexture(btn_hvh);
    if (btn_hvs.id != 0) UnloadTexture(btn_hvs);
    if (fond_modale.id != 0) UnloadTexture(fond_modale);
    if (fond_config.id != 0) UnloadTexture(fond_config);
    if (fond_plateau.id != 0) UnloadTexture(fond_plateau);
    
    // Déchargement des textures de tuiles
    for (int c = 0; c < 4; c++) {
        for (int v = 1; v <= 13; v++) {
            if (tex_tuiles[c][v].id != 0) UnloadTexture(tex_tuiles[c][v]);
        }
    }
    
    // Déchargement des jokers
    for (int i = 0; i < 2; i++) {
        if (tex_jokers[i].id != 0) UnloadTexture(tex_jokers[i]);
    }
    
    if (tex_chevalet.id != 0) UnloadTexture(tex_chevalet);
    
    // Déchargement des avatars joueurs
    for (int i = 0; i < 4; i++) {
        if (tex_joueurs[i].id != 0) UnloadTexture(tex_joueurs[i]);
    }
}
