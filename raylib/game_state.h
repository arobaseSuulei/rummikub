#ifndef GAME_STATE_H
#define GAME_STATE_H
#define GRID_ROWS 9
#define GRID_COLS 17
#define MAX_PLAYERS 4
#include "raylib.h"

// États du jeu
typedef enum {
    STATE_MENU,           
    STATE_HELP,   
    STATE_INPUT_NAMES,
    STATE_CHOOSE_MODE,    
    STATE_CONFIG,         
    STATE_PLAYING,        
    STATE_SCORES          
} GameState;

// Couleurs des tuiles
typedef enum {
    TILE_RED,
    TILE_BLUE,
    TILE_BLACK,
    TILE_YELLOW,
    TILE_JOKER
} TileColor;

// Structure d'une tuile
typedef struct {
    int id;            
    int valeur;        
    char couleur;      
    bool joker;
    Rectangle rect;    
    bool is_selected;
} Tile; // FIX 1 : Il manquait l'accolade fermante "}" et le nom "Tile;" ici

// Structure pour un joueur
typedef struct {
    char pseudo[23];
    int icon_id;
    bool tour;
    bool premier_tour;
    Tile main[30];     
    int nb_tuiles;
} Player;

typedef struct {
    Tile grille[9][17];
    bool occupee[9][17];
} Board;

// Structure pour gérer l'état
typedef struct {
    GameState current_state;
    int screen_width;
    int screen_height;
    bool should_close;
    int config_step;     
    int selected_tile_idx;
    int num_players;        
    char player_names[4][16]; 
    int name_char_counts[4];
    int editing_player;   
    int dragging_tile_idx;   // Index de la tuile qu'on déplace (-1 si rien)
    bool is_dragging;      // Est-ce qu'on est en train de déplacer ?
    Vector2 drag_offset;     // Pour éviter que la tuile "saute" au centre de la souris
    
    Board board;
    
    Player players[4];
    Tile pioche[106];  
    int nb_pioche; // FIX 2 : Il manquait un point-virgule ";" ici
} GameData; // FIX 3 : Il manquait l'accolade fermante "}" avant le nom GameData

// Fonctions
void InitGame(GameData *game);
void UpdateGame(GameData *game);
void DrawGame(GameData *game);
void CleanupGame(GameData *game);

#endif
