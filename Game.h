#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

#define FALSE 0
#define TRUE 1

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define GRID_SIZE 6
#define CELL_SIZE 70

#define BUTTON_WIDTH 200
#define BUTTON_HEIGHT 50

#define WINDOW_CENTER_X 400
#define WINDOW_CENTER_Y 300
#define MAX_IMAGES 10



typedef enum {
    GAME_START,
    GAME_NOT_STARTED,
    GAME_STOPPED,
    GAME_SCORES
} Game;

typedef struct {
    int x;
    int y;
} Position;

typedef struct{
    int filled;
    int selected;
    SDL_Color soldier_color;
    Position pos;
    SDL_Texture* image;
} Cellule;

typedef struct{
    int num_player;
    int score;
    int win_number;
} Scores;

/** Calculate the positions of the buttons to center them*/
SDL_Rect newGameButtonRect = {
    WINDOW_CENTER_X - BUTTON_WIDTH / 2,
    WINDOW_CENTER_Y - 2 * BUTTON_HEIGHT,
    BUTTON_WIDTH,
    BUTTON_HEIGHT
};

SDL_Rect resumeGameButtonRect = {
    WINDOW_CENTER_X - BUTTON_WIDTH / 2,
    WINDOW_CENTER_Y - BUTTON_HEIGHT,
    BUTTON_WIDTH,
    BUTTON_HEIGHT
};

SDL_Rect scoresButtonRect = {
    WINDOW_CENTER_X - BUTTON_WIDTH / 2,
    WINDOW_CENTER_Y,
    BUTTON_WIDTH,
    BUTTON_HEIGHT
};

SDL_Rect exitButtonRect = {
    WINDOW_CENTER_X - BUTTON_WIDTH / 2,
    WINDOW_CENTER_Y + BUTTON_HEIGHT,
    BUTTON_WIDTH,
    BUTTON_HEIGHT
};

SDL_Rect continueButtonRect = {
    580 ,
    20 ,
    BUTTON_WIDTH,
    BUTTON_HEIGHT
};

SDL_Rect saveExitButtonRect = {
    580 ,
    70 ,
    BUTTON_WIDTH,
    BUTTON_HEIGHT
};

SDL_Rect exitGameButtonRect = {
    580 ,
    120 ,
    BUTTON_WIDTH,
    BUTTON_HEIGHT
};

int initialize_window();
void destroy_window();
void save_score(const char* filename);
void load_score(const char* filename);
void save_grid(const char* filename);
void load_grid(const char* filename);
void delete_file_content(const char* filename);
int is_file_empty(const char* filename);
void game_over();
void show_all_grid_info();
int is_around(Cellule first, Cellule second);
int is_jump_possible(Cellule first, Cellule second);
void reset_selections();
int compare_colors(SDL_Color color1, SDL_Color color2);
void handle_selection();
int is_mouse_over_button(SDL_Rect buttonRect, int mouseX, int mouseY);
void process_input();
void draw_text(const char* text, int x, int y, int fontSize);
SDL_Texture* load_texture(const char* path);
void draw_image (SDL_Texture* image,int size,int x,int y);
void draw_menu_button(int isMouseOver,SDL_Rect ButtonRect);
void render();
void draw_menu();
void update_soldiers();
void draw_filled_circle(int centerX, int centerY, int radius, SDL_Color color);
void draw_combat_zone();


#endif // GAME_H_INCLUDED
