#include <stdio.h>
#include <stdlib.h>
#include <MLV/MLV_all.h>
#define F_HEIGHT 600
#define F_WIDTH 600

typedef struct{
    int x;
    int y;
} Mouse;

typedef struct{
    char *text;
    int x;
    int y;
    int width;
    int height;
    MLV_Color color;
} Button;

int mouse_in(Button *b, Mouse *mouse){
    // Detecte si les coordonnée x, y de la structure mouse sont ou pas dans les coordonnées de la structure boutton
    // la fonction renvoie 1 si les coordonnées de la souris sont dedans et 0 sinon
    if (mouse->x > b->x && mouse->x < (b->x + b->width)){
        if (mouse->y > b->y && mouse->y < (b->y + b->height))
            return 1;
    }
    return 0;
}

void draw_button(Button *b){
    // Dessine un boutton avec la structure boutton
    MLV_draw_rectangle(b->x, b->y, b->width, b->height, b->color);
    MLV_draw_text_box(
                    b->x , b->y, b->width, b->height,
                    b->text, 1,
                    MLV_COLOR_BLACK, MLV_COLOR_BLACK, b->color,
                    MLV_TEXT_CENTER, MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER
                );
}

void init_window(char* name){
    // Initialisation d'une fenetre blanche avec un nom placer en paramètre
    MLV_create_window( name, name, F_HEIGHT, F_WIDTH );
    MLV_draw_filled_rectangle(0, 0, F_HEIGHT, F_WIDTH, MLV_COLOR_WHITE);
    MLV_actualise_window();
}

int main(void) {
    init_window("Menu");
    Mouse mouse;
    Button button1, button2, button3;

    button1.width = 200; button2.width = 200; button3.width = 200;
    button1.height = 80; button2.height = 80; button3.height = 80;

    button1.text = "Jouer"; button1.x = (F_WIDTH/2)-(button1.width/2); button1.y = (F_HEIGHT*1)/5;
    button2.text = "Options"; button2.x = (F_WIDTH/2)-(button1.width/2); button2.y = (F_HEIGHT*2)/5;
    button3.text = "Quitter"; button3.x = (F_WIDTH/2)-(button1.width/2); button3.y = (F_HEIGHT*3)/5;

    button1.color = MLV_COLOR_GREY; button2.color = MLV_COLOR_GREY; button3.color = MLV_COLOR_GREY;

    draw_button(&button1); draw_button(&button2); draw_button(&button3);

    MLV_actualise_window();

    while (!mouse_in(&button3, &mouse)){ // Pour l'instant
        MLV_wait_mouse(&mouse.x, &mouse.y);
        if (mouse_in(&button1, &mouse))
            printf("jouer");
            // JOUER
        else if (mouse_in(&button2, &mouse))
            printf("options");
            // OPTIONS
        MLV_actualise_window();
    }
    MLV_free_window();
    return 0;
}