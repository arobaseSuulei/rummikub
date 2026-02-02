// raylib/ui.h
#ifndef UI_H
#define UI_H

#include "raylib.h"

// Bouton interactif
typedef struct {
    Rectangle bounds;     // Position et taille
    Texture2D texture;    // Image du bouton
    Texture2D hoverTex;   // Image au survol (optionnel)
    char label[50];       // Texte affiché
    bool isHovered;
    bool isPressed;
} Button;

// Fonctions UI
void InitUI(void);
void CleanupUI(void);

// Boutons
Button CreateButton(int x, int y, Texture2D tex, const char* label);
bool IsButtonPressed(Button btn);
void UpdateButton(Button *btn);
void DrawButton(Button btn);

#endif
