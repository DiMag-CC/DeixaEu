#ifndef MENU_H
#define MENU_H

#include <raylib.h>

typedef enum {
    MENU_MAIN = 0,
    MENU_CREDITS = 1
} MenuScreen;

typedef struct Menu {
    MenuScreen screen;
    int selectedOption; // 0 = Play, 1 = Credist, 2 = Exit
    int confirmPressed;
} Menu;

Menu createMenu();
void updateMenu(Menu *menu);
void drawMenu(Menu menu);

#endif
