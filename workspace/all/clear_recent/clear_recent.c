#include <stdio.h>
#include <msettings.h>

#include "defines.h"
#include "utils.h"
#include "api.h"

int main(int argc, char* argv[]) {
    PWR_setCPUSpeed(CPU_SPEED_MENU);

    SDL_Surface* screen = GFX_init(MODE_MAIN);
    PAD_init();
    PWR_init();
    InitSettings();

    int quit = 0;
    int save_changes = 0;

    // Show confirmation message
    // GFX_blitHardwareGroup(screen, show_setting);
    GFX_blitMessage(font.large, "Are you sure you want to clear\nRecently Played?", screen, &(SDL_Rect){0,0,screen->w,screen->h});
    GFX_blitButtonGroup((char*[]){ "B","CANCEL", "A","CLEAR", NULL },0, screen, 1);

    GFX_flip(screen);

    // Wait for user's input
    while (!quit) {
        uint32_t frame_start = SDL_GetTicks();

        PAD_poll();
        if (PAD_justPressed(BTN_A)) {
            save_changes = 1;
            quit = 1;
        } else if (PAD_justPressed(BTN_B)) {
            quit = 1;
        } else {
            GFX_sync();
        }

        PWR_update(0, NULL, NULL,NULL);
    }

    // Execute main program based on user's input
    if (save_changes) {
         fclose(fopen(RECENT_PATH, "w"));
    }

    QuitSettings();
    PWR_quit();
    GFX_quit();

    return EXIT_SUCCESS;
}
