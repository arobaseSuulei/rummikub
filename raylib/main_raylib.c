#include "raylib.h"
#include "game_state.h"

int main(void) {
    // Initialisation
    const int screenWidth = 1600;
    const int screenHeight = 900;
    InitWindow(screenWidth, screenHeight, "Rummikub Premium");
    SetTargetFPS(60);
    
    // Données du jeu
    GameData game = {
        .current_state = STATE_MENU,
        .screen_width = screenWidth,
        .screen_height = screenHeight,
        .should_close = false
    };
    
    InitGame(&game);
    
    // Boucle principale
    while (!game.should_close && !WindowShouldClose()) {
        // Mise à jour
        UpdateGame(&game);
        
        // Dessin
        BeginDrawing();
            ClearBackground(BLACK);
            DrawGame(&game);
        EndDrawing();
    }
    
    // Nettoyage
    CleanupGame(&game);
    CloseWindow();
    
    return 0;
}
