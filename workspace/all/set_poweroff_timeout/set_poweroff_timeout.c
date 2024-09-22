#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <msettings.h>

#include "defines.h"
#include "api.h"
#include "utils.h"

enum {
	CURSOR_HOUR,
	CURSOR_MINUTE,
	CURSOR_SECOND,
};

int main(int argc , char* argv[]) {
	PWR_setCPUSpeed(CPU_SPEED_MENU);
	
	SDL_Surface* screen = GFX_init(MODE_MAIN);
	PAD_init();
	PWR_init();
	InitSettings();
	
	SDL_Surface* digits = SDL_CreateRGBSurface(SDL_SWSURFACE, SCALE2(120,16), FIXED_DEPTH,RGBA_MASK_AUTO);
	SDL_FillRect(digits, NULL, RGB_BLACK);
	
	SDL_Surface* digit;
	char* chars[] = { "0","1","2","3","4","5","6","7","8","9","/",":", NULL };
	char* c;
	int i = 0;
    #define DIGIT_WIDTH 10
    #define DIGIT_HEIGHT 16

    #define CHAR_COLON 11

	while (c = chars[i]) {
		digit = TTF_RenderUTF8_Blended(font.large, c, COLOR_WHITE);
		int y = i==CHAR_COLON ? SCALE1(-1.5) : 0; // : sits too low naturally

		SDL_BlitSurface(digit, NULL, digits, &(SDL_Rect){ (i * SCALE1(DIGIT_WIDTH)) + (SCALE1(DIGIT_WIDTH) - digit->w)/2, y + (SCALE1(DIGIT_HEIGHT) - digit->h)/2 });
		SDL_FreeSurface(digit);
		i += 1;
	}
	
	SDL_Event event;
	int quit = 0;
	int save_changes = 0;
	int select_cursor = 0;

    int timeout = getInt(POWEROFF_TIMEOUT_PATH);
	
	int hour_selected = 0;
	int minute_selected = 2;
	int seconds_selected = 0;

    if(timeout > 0){
        convertMillisecondsToTime(timeout, &hour_selected, &minute_selected, &seconds_selected);
    }
	
	// x,y,w are pre-scaled
	int blit(int i, int x, int y) {
		SDL_BlitSurface(digits, &(SDL_Rect){i*SCALE1(10),0,SCALE2(10,16)}, screen, &(SDL_Rect){x,y});
		return x + SCALE1(10);
	}
	void blitBar(int x, int y, int w) {
		GFX_blitPill(ASSET_UNDERLINE, screen, &(SDL_Rect){x,y,w});
	}
	int blitNumber(int num, int x, int y) {
		int n;
		if (num > 999) {
			n = num / 1000;
			num -= n * 1000;
			x = blit(n, x,y);
			
			n = num / 100;
			num -= n * 100;
			x = blit(n, x,y);
		}
		n = num / 10;
		num -= n * 10;
		x = blit(n, x,y);
		
		n = num;
		x = blit(n, x,y);
		
		return x;
	}
	void validate(void) {
		// time
		if (hour_selected > 23) hour_selected -= 24;
		else if (hour_selected < 0) hour_selected += 24;
		if (minute_selected > 59) minute_selected -= 60;
		else if (minute_selected < 0) minute_selected += 60;
		if (seconds_selected > 59) seconds_selected -= 60;
		else if (seconds_selected < 0) seconds_selected += 60;
	}
	
	int option_count = 3;

	int dirty = 1;
	int show_setting = 0;
	int was_online = PLAT_isOnline();

	while(!quit) {
		uint32_t frame_start = SDL_GetTicks();
		
		PAD_poll();
		
		if (PAD_justRepeated(BTN_UP)) {
			dirty = 1;
			switch(select_cursor) {
				case CURSOR_HOUR:
					hour_selected++;
				break;
				case CURSOR_MINUTE:
					minute_selected++;
				break;
				case CURSOR_SECOND:
					seconds_selected++;
				break;
				default:
				break;
			}
		}
		else if (PAD_justRepeated(BTN_DOWN)) {
			dirty = 1;
			switch(select_cursor) {
				case CURSOR_HOUR:
					hour_selected--;
				break;
				case CURSOR_MINUTE:
					minute_selected--;
				break;
				case CURSOR_SECOND:
					seconds_selected--;
				break;
				default:
				break;
			}
		}
		else if (PAD_justRepeated(BTN_LEFT)) {
			dirty = 1;
			select_cursor--;
			if (select_cursor < 0) select_cursor += option_count;
		}
		else if (PAD_justRepeated(BTN_RIGHT)) {
			dirty = 1;
			select_cursor++;
			if (select_cursor >= option_count) select_cursor -= option_count;
		}
		else if (PAD_justPressed(BTN_A)) {
			save_changes = 1;
			quit = 1;
		}
		else if (PAD_justPressed(BTN_B)) {
			quit = 1;
		}
		
		PWR_update(&dirty, NULL, NULL,NULL);
		
		int is_online = PLAT_isOnline();
		if (was_online!=is_online) dirty = 1;
		was_online = is_online;
		
		if (dirty) {
			validate();

			GFX_clear(screen);
			
			GFX_blitHardwareGroup(screen, show_setting);
			
			if (show_setting) GFX_blitHardwareHints(screen, show_setting);

			GFX_blitButtonGroup((char*[]){ "B","CANCEL", "A","SET", NULL }, 1, screen, 1);
		
			int ox = (screen->w - (SCALE1(78))) / 2;
			
			// time
			int x = ox;
			int y = SCALE1((((FIXED_HEIGHT / FIXED_SCALE)-PILL_SIZE-DIGIT_HEIGHT)/2));

            x = blitNumber(hour_selected, x,y);
			x = blit(CHAR_COLON, x,y);
			x = blitNumber(minute_selected, x,y);
			x = blit(CHAR_COLON, x,y);
			x = blitNumber(seconds_selected, x,y);
			
            // cursor
			x = ox;
			y += SCALE1(19);
			if (select_cursor!=CURSOR_HOUR) {
				x += SCALE1(30);
				x += (select_cursor - 1) * SCALE1(30);
			}
			blitBar(x,y, (SCALE1(20)));
		
			GFX_flip(screen);
			dirty = 0;
		}
		else GFX_sync();
	}
	
	SDL_FreeSurface(digits);
	
	QuitSettings();
	PWR_quit();
	PAD_quit();
	GFX_quit();	

	if (save_changes){
        int convertedTimeToMilliseconds = convertTimeToMilliseconds(hour_selected, minute_selected, seconds_selected);
        char buffer[20];
        sprintf(buffer, "%d", convertedTimeToMilliseconds);  // Convert int to string
        putFile(POWEROFF_TIMEOUT_PATH, buffer);
    }
	
	return EXIT_SUCCESS;
}
