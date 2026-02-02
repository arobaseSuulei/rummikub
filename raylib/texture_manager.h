// raylib/texture_manager.h
#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include "raylib.h"

// Textures principales
typedef struct {
    Texture2D fond_accueil;
    Texture2D fond_plateau;
    
    // Boutons
    Texture2D btn_jouer;
    Texture2D btn_quitter;
    Texture2D btn_scores;
    Texture2D btn_aide;
    Texture2D btn_pioche;
    Texture2D btn_valider;
    Texture2D btn_annuler;
    Texture2D btn_retour;
    
    // Tuiles (tableau de 54 textures)
    Texture2D tuiles[54];
    
    // UI
    Texture2D chevalet_bg;
    Texture2D player_icons[4];
} GameTextures;

// Fonctions
void InitTextures(void);
void CleanupTextures(void);
Texture2D GetTileTexture(int id);  // Récupérer texture par ID tuile

extern GameTextures textures;  // Global accessible

#endif
