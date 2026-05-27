#ifndef MENU_H
#define MENU_H

#include <raylib.h>

typedef enum {
    MENU_MAIN = 0,
    MENU_RANKING = 1,
    MENU_CREDITS = 2
} MenuScreen;

typedef struct Menu {
    MenuScreen screen;
    int selectedOption; // 0 = Play, 1 = Ranking, 2 = Credits, 3 = Exit
    int confirmPressed;
} Menu;

Menu createMenu();
void updateMenu(Menu *menu);
void drawMenu(Menu menu);
void unloadMenuResources(void);

#endif
