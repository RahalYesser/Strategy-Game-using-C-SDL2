#include <SDL2/SDL.h>
#include <SDL_image/SDL_image.h>
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_mixer.h>
#include <SDL_ttf/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
//#include <iostream.h>
#include <string.h>
#include "Game.h"

int game_is_running = FALSE;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
TTF_Font* font = NULL;
Mix_Chunk *audio_chunk = NULL;

SDL_Event event;

SDL_Texture *image_texture_red,*image_texture_blue;
SDL_Texture *win_right_texture,*win_left_texture;
SDL_Texture *winner,*bg_image,*bg_scores;

Game game_status = GAME_NOT_STARTED;

const SDL_Color WHITE = {255,255,255,255};
const SDL_Color RED = {255,0,0,255};
const SDL_Color BLUE = {0,0,255,255};

Cellule grid[GRID_SIZE][GRID_SIZE];

Scores scores[2];

int isMouseOverNewGame,isMouseOverResumeGame,isMouseOverScores,isMouseOverExit,isMouseOverSaveExit,isMouseOverExitGame,isMouseOverContinue = FALSE;

Cellule first_selected;
Cellule second_selected;
Cellule selected_image;
Cellule target_cell;


/** Calculate the starting position for the grid to center it in the window */
int xOffset,yOffset;
int xOffset = (SCREEN_WIDTH - (GRID_SIZE * CELL_SIZE)) /2 ;
int yOffset = (SCREEN_HEIGHT - (GRID_SIZE * CELL_SIZE)) /2;

int player_number;
int blue_on_red,red_on_blue;
int current_player;
int game_is_over;
int exit_game = FALSE;


/** Intialize game window */
int initialize_window() {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
        return FALSE;
    }

    window = SDL_CreateWindow(
        "Combat Game",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_BORDERLESS);

    if (!window) {
        fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
        return FALSE;
    }


    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fprintf(stderr, "Renderer creation failed: %s\n", SDL_GetError());
        return FALSE;
    }

     //Initialize SDL_image
    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        SDL_Quit();
        return -1;
    }

    if (TTF_Init() != 0) {
        fprintf(stderr, "SDL_ttf initialization failed: %s\n", TTF_GetError());
        destroy_window();
        return FALSE;
    }

    TTF_Font *font = TTF_OpenFont("Poppins-BlackItalic.ttf", 24);
    if (!font) {
        fprintf(stderr, "Font loading failed: %s\n", TTF_GetError());
        destroy_window();
        return FALSE;
    }
    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        fprintf(stderr, "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        return FALSE;
    }

    // Load audio file
    audio_chunk = Mix_LoadWAV("Oggy.mp3");
    if (!audio_chunk) {
        fprintf(stderr, "Failed to load audio! SDL_mixer Error: %s\n", Mix_GetError());
        return FALSE;
    }
    if (game_status==GAME_START){
    // Initialize the grid with zeroes
    for (int i = 0; i <= GRID_SIZE; ++i) {
        for (int j = 0; j <= GRID_SIZE; ++j) {
            grid[i][j].filled = 0;
        }
    }

    }

    return TRUE;
}

/** Close game window */
void destroy_window() {
    TTF_CloseFont(font);
    SDL_DestroyTexture(image_texture_red);
    SDL_DestroyTexture(image_texture_blue);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    TTF_Quit();
    Mix_FreeChunk(audio_chunk);
    Mix_CloseAudio();
    SDL_Quit();
}

/** Reset game*/
void reset_game(){
    blue_on_red=0;
    red_on_blue=0;
    game_is_over=0;
}

/** Initilize grid */
void initialize_grid() {
    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE; ++j) {
            grid[i][j].filled = 0;
            grid[i][j].selected = 0;
            grid[i][j].image = NULL;
            grid[i][j].pos.x = i*CELL_SIZE + xOffset;
            grid[i][j].pos.y = j*CELL_SIZE + yOffset;
        }
    }
}

/** Initialize soldiers on combat zone (grid) */
void initialize_soldiers() {
    int col;
    int n = GRID_SIZE/2;
    int i=0;
    int j=0;
    //Draw from the start
    for (int row = 0; row < CELL_SIZE * (GRID_SIZE/2); row += CELL_SIZE) {
        for (col = 0; col < CELL_SIZE * n; col += CELL_SIZE) {
            grid[i][j].filled=1;
            grid[i][j].soldier_color=RED;
            grid[i][j].image=image_texture_red;
            j++;
            if (col == CELL_SIZE * (n-1)){
               n--;
            }
        }
        j=0;
        i++;
    }

    n=GRID_SIZE/2;
    i=GRID_SIZE-1;
    j=GRID_SIZE-1;
    // Draw from the end
    for (int row = CELL_SIZE * (GRID_SIZE-1); row >= CELL_SIZE * (GRID_SIZE-(GRID_SIZE/2)); row -= CELL_SIZE) {
        for (int col = CELL_SIZE * (GRID_SIZE-1); col >= CELL_SIZE * (GRID_SIZE-n); col -= CELL_SIZE) {
            grid[i][j].filled=1;
            grid[i][j].soldier_color=BLUE;
            grid[i][j].image=image_texture_blue;
            j--;
            if (col == CELL_SIZE * (GRID_SIZE-n) ){
               n--;
            }
        }
        j=GRID_SIZE-1;
        i--;
    }
}

/** Save score */
void save_score(const char* filename){
    FILE* file = fopen(filename, "w");
    if (file != NULL) {
        for (int i = 0; i < 2; ++i) {
            fprintf(file, "%d %d %d\n",
                    scores[i].num_player,
                    scores[i].score,
                    scores[i].win_number);
        }
        fclose(file);
    }
}

/** load score */
void load_score(const char* filename){
    FILE* file = fopen(filename, "r");
    if (file != NULL) {
        for (int i = 0; i < 2; ++i) {
            fscanf(file, "%d %d %d\n",
                    &scores[i].num_player,
                    &scores[i].score,
                    &scores[i].win_number);
        }
        fclose(file);
    }
}

/** Save game */
void save_grid(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file != NULL) {
        // Save the current player first
        fprintf(file, "%d\n", current_player);
        for (int i = 0; i < GRID_SIZE; ++i) {
            for (int j = 0; j < GRID_SIZE; ++j) {
                fprintf(file, "%d %d %d %d %d %d %d %d \n",
                        grid[i][j].filled,
                        grid[i][j].selected,
                        grid[i][j].soldier_color.r,
                        grid[i][j].soldier_color.g,
                        grid[i][j].soldier_color.b,
                        grid[i][j].soldier_color.a,
                        grid[i][j].pos.x,
                        grid[i][j].pos.y);
            }
        }
        fclose(file);
    }
}

/** Load game */
void load_grid(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file != NULL) {
        // Load the current player first
        fscanf(file, "%d", &current_player);
        for (int i = 0; i < GRID_SIZE; ++i) {
            for (int j = 0; j < GRID_SIZE; ++j) {
                fscanf(file, "%d %d %d %d %d %d %d %d",
                       &grid[i][j].filled,
                       &grid[i][j].selected,
                       &grid[i][j].soldier_color.r,
                       &grid[i][j].soldier_color.g,
                       &grid[i][j].soldier_color.b,
                       &grid[i][j].soldier_color.a,
                       &grid[i][j].pos.x,
                       &grid[i][j].pos.y);
            }
        }
        fclose(file);
    }
}

/** Initilize game file to save new game */
void delete_file_content(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file != NULL) {
        fclose(file);
    } else {
        printf("Error opening file for deletion: %s\n", filename);
    }
}

/** Check if the game file is empty */
int is_file_empty(const char* filename){
  FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file '%s'\n", filename);
        return -1;  // Return -1 to indicate an error
    }
    fseek(file, 0, SEEK_END);  // Move the file pointer to the end of the file
    long size = ftell(file);   // Get the current position, which is the file size

    fclose(file);

    if (size == 0) {
        return 1;  // File is empty
    } else {
        return 0;  // File is not empty
    }
}

/** Check if game is over */
void game_over(){
    int col;
    int n = GRID_SIZE/2;
    int i=0;
    int j=0;
    // Check soldiers from the start
    for (int row = 0; row < CELL_SIZE * (GRID_SIZE/2); row += CELL_SIZE) {
        for (col = 0; col < CELL_SIZE * n; col += CELL_SIZE) {

            if(compare_colors(grid[i][j].soldier_color,BLUE)){
                blue_on_red++;
            }
            j++;
            if (col == CELL_SIZE * (n-1)){
               n--;
            }
        }
        j=0;
        i++;
    }

    n = GRID_SIZE/2;
    i=GRID_SIZE-1;
    j=GRID_SIZE-1;
    // Check soldiers from the end
    for (int row = CELL_SIZE * (GRID_SIZE-1); row >= CELL_SIZE * (GRID_SIZE-(GRID_SIZE/2)); row -= CELL_SIZE) {
    for (int col = CELL_SIZE * (GRID_SIZE-1); col >= CELL_SIZE * (GRID_SIZE-n); col -= CELL_SIZE) {
        if(compare_colors(grid[i][j].soldier_color,RED)){
                red_on_blue++;
            }
        j--;
        if (col == CELL_SIZE * (GRID_SIZE-n) ){
               n--;
            }
        }
        j=GRID_SIZE-1;
        i--;
    }
    printf("Game over : %d\n",game_is_over);
    printf("red_on_blue : %d\n",red_on_blue);
    printf("blue_on_red : %d\n",blue_on_red);

    if(red_on_blue==player_number){
        game_is_over=1;

    }else if(blue_on_red==player_number){
        game_is_over=1;
    }else {
        blue_on_red=0;
        red_on_blue=0;
    }

}

/** Show grid info on console */
void show_all_grid_info (){
     //show grid info
                     for(int i=0;i<GRID_SIZE;i++){
                        for(int j=0;j<GRID_SIZE;j++){
                            printf("grid[%d][%d]=%d \n",i,j,grid[i][j].filled);
                            printf("SDL Color : R=%d, G=%d, B=%d, A=%d\n",
                                   grid[i][j].soldier_color.r,
                                   grid[i][j].soldier_color.g,
                                   grid[i][j].soldier_color.b,
                                   grid[i][j].soldier_color.a);
                            printf("Position : (%d, %d)\n",grid[i][j].pos.x,grid[i][j].pos.y);

                        }
                    }
}

/** Function to check if the second cell is around the first cell*/
int is_around(Cellule first, Cellule second) {
    int dx = abs(first.pos.x - second.pos.x);
    int dy = abs(first.pos.y - second.pos.y);
    // Check if the second cell is in one of the eight neighboring positions
    return (dx <= CELL_SIZE && dy <= CELL_SIZE) && (dx != 0 || dy != 0) ;
}

/** Function to check if jump of soldier is possible*/
int is_jump_possible(Cellule first, Cellule second){

    // Check if there is an image with a different color between the first and second selected cells
    int stepX = 0;
    int stepY = 0;
    int dx = abs(first.pos.x - second.pos.x);
    int dy = abs(first.pos.y - second.pos.y);

    if (second_selected.pos.x > first_selected.pos.x) {
        stepX = 1;
    } else if (second_selected.pos.x < first_selected.pos.x) {
        stepX = -1;
    }

    if (second_selected.pos.y > first_selected.pos.y) {
        stepY = 1;
    } else if (second_selected.pos.y < first_selected.pos.y) {
        stepY = -1;
    }
        printf("StepX : %d\n",stepX);
        printf("StepY : %d\n",stepY);
         for (int i = 0; i < GRID_SIZE; ++i) {
            for (int j = 0; j < GRID_SIZE; ++j) {
                    if(grid[i][j].pos.x == first.pos.x && grid[i][j].pos.y==first.pos.y
                       && grid[i+stepX][j+stepY].image!=NULL
                       && dx <= 2* CELL_SIZE && dy <= 2*CELL_SIZE
                       && (dx != 0 || dy != 0) ){
                           if (compare_colors(grid[i][j].soldier_color,BLUE) && compare_colors(grid[i+stepX][j+stepY].soldier_color,RED)){
                                    printf("there is different image between");
                                    return 1;

                           }
                           else if (compare_colors(grid[i][j].soldier_color,RED) && compare_colors(grid[i+stepX][j+stepY].soldier_color,BLUE) ){
                                    printf("there is different image between");
                                    return 1;
                           }
                    }
            }
          }
      return 0 ;

}

/** Reset selected cellule */
void reset_selections() {
    for (int row = 0; row < GRID_SIZE; ++row) {
        for (int col = 0; col < GRID_SIZE; ++col) {
            grid[row][col].selected = 0;
        }
    }
    // Reset all selection variables
    first_selected.selected = 0;
    second_selected.selected = 0;
    // Add more variables if needed
}

/** Function to compare two SDL_Color structures */
int compare_colors(SDL_Color color1, SDL_Color color2) {
    return color1.r == color2.r &&
           color1.g == color2.g &&
           color1.b == color2.b &&
           color1.a == color2.a;
}

/** Function to check selected cellule */
void handle_selection() {
    int mouseX = event.button.x;
    int mouseY = event.button.y;
    // Check which cell was clicked
    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE; ++j) {
            if (mouseX >= grid[i][j].pos.x && mouseX < grid[i][j].pos.x + CELL_SIZE &&
                mouseY >= grid[i][j].pos.y && mouseY < grid[i][j].pos.y + CELL_SIZE) {
                if(game_is_over != 1){
                if (!first_selected.selected) {
                    // First selection
                    first_selected = grid[i][j];
                    if(current_player==1&&compare_colors(first_selected.soldier_color,RED)) {
                        grid[i][j].selected=!grid[i][j].selected;
                        printf("Clicked on Red");
                    }
                    if(current_player==2&&compare_colors(first_selected.soldier_color,BLUE)) {
                        grid[i][j].selected=!grid[i][j].selected;
                        printf("Clicked on Blue");
                    }
                    first_selected = grid[i][j];
                    // If the first selected cell has no image, reset selections
                    if (first_selected.image == NULL) {
                        printf("First selected cell has no image. Resetting selections.\n");
                        reset_selections();
                    } else {
                        printf("First selected cell: (%d, %d)\n", i, j);
                        //show_all_grid_info();
                    }
                } else {
                    // Second selection
                    second_selected = grid[i][j];
                    is_jump_possible(first_selected,second_selected);
                    // If the second selected cell is at the same position, reset selections
                    if (first_selected.pos.x == second_selected.pos.x && first_selected.pos.y == second_selected.pos.y) {
                        printf("Second selected cell is at the same position. Resetting selections.\n");
                        reset_selections();
                    }
                    // If the second selected cell has no image and is around the first selected cell
                    else if (second_selected.image == NULL && (is_around(first_selected, second_selected) || is_jump_possible(first_selected,second_selected) ) /*||  isJump(first_selected,second_selected)*/  ) {
                        // Save the first selected cell in selected_image and the second in target_cell
                        selected_image = first_selected;
                        target_cell = second_selected;
                        printf("Selected image at: (%d, %d)\n", selected_image.pos.x , selected_image.pos.y );
                        printf("Target cell at: (%d, %d)\n", target_cell.pos.x , target_cell.pos.y );
                        // Move the image from the first selected cell to the second selected cell
                        target_cell.image = selected_image.image;
                        target_cell.soldier_color = selected_image.soldier_color;
                        target_cell.filled = 1;
                        grid[i][j] = target_cell;
                        // Clear the source cell
                        selected_image.image = NULL;
                        selected_image.soldier_color = WHITE;
                        selected_image.filled = 0;
                        // Clear seleted image
                        for (int row = 0; row <= GRID_SIZE; ++row) {
                            for (int col = 0; col <= GRID_SIZE; ++col) {
                                if( grid[row][col].image!=NULL && grid[row][col].pos.x == selected_image.pos.x && grid[row][col].pos.y == selected_image.pos.y){
                                    grid[row][col] = selected_image;
                                }
                            }
                        }
                        // Check if game is over
                        game_over();
                        printf("Game Over : %d",game_is_over);
                        if(!game_is_over) {
                                current_player = (current_player == 1) ? 2 : 1;
                        }
                        reset_selections();

                    }
                    // If the conditions for a valid move are not met, reset selections
                    else {
                        printf("Conditions for a valid move are not met. Resetting selections.\n");
                        reset_selections();
                    }
                }
                break;
            }
            }
        }
    }
}

/** Check selected button */
int is_mouse_over_button(SDL_Rect buttonRect, int mouseX, int mouseY) {
    return mouseX >= buttonRect.x && mouseX < buttonRect.x + buttonRect.w &&
           mouseY >= buttonRect.y && mouseY < buttonRect.y + buttonRect.h;
}

/** Check all game action */
void process_input(){
    SDL_PollEvent(&event);
    int mouseX, mouseY;  // Move the declarations outside the switch statement

    switch(event.type){
    case SDL_QUIT :
        game_is_running = FALSE;
        break;
    case SDL_KEYDOWN :
        if(event.key.keysym.sym == SDLK_ESCAPE)
            if (game_status == GAME_SCORES){
                game_status = GAME_NOT_STARTED;
            }
            else if(game_status == GAME_START && !game_is_over){
                    exit_game = 1;
            }else if (game_is_over){
                   game_status = GAME_NOT_STARTED;
                   load_score("score.txt");
                   if (current_player==1){
                     scores[current_player-1].score+=red_on_blue;
                   }else { scores[current_player-1].score+=blue_on_red; }
                   scores[current_player-1].win_number+=1;
                   save_score("score.txt");
                   delete_file_content("grid.txt");
                   reset_game();
                   printf("Quit");

            };
        break;
    case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT) {
                mouseX = event.button.x;
                mouseY = event.button.y;
                isMouseOverSaveExit = is_mouse_over_button(saveExitButtonRect, mouseX, mouseY);
                isMouseOverExitGame = is_mouse_over_button(exitGameButtonRect, mouseX, mouseY);
                isMouseOverNewGame = is_mouse_over_button(newGameButtonRect, mouseX, mouseY);
                isMouseOverResumeGame = is_mouse_over_button(resumeGameButtonRect, mouseX, mouseY);
                isMouseOverScores = is_mouse_over_button(scoresButtonRect,mouseX,mouseY);
                isMouseOverExit = is_mouse_over_button(exitButtonRect, mouseX, mouseY);

                if (game_status == GAME_NOT_STARTED) {
                    // Check if the mouse click is within the start button area
                    if (isMouseOverNewGame) {
                         initialize_grid();
                         initialize_soldiers();
                         reset_game();
                         // Initialize random seed based on the current time
                         srand((unsigned int)time(NULL));
                         // Initialize currentPlayer with a random value of 1 or 2
                         current_player = (rand() % 2) + 1;
                         printf("Current player : %d",current_player);
                         scores[0].num_player=1;
                         scores[1].num_player=2;
                         game_status = GAME_START;
                    }
                    if (isMouseOverResumeGame && !is_file_empty("grid.txt")){
                        load_grid("grid.txt");
                        show_all_grid_info();
                        game_status = GAME_START;
                    }else if (isMouseOverScores){
                         if(!is_file_empty("score.txt")){
                            load_score("score.txt");
                            printf("here");
                         }
                         game_status = GAME_SCORES;
                    }
                    // Check if the mouse click is within the exit button area
                    else if(isMouseOverExit){
                          game_is_running = FALSE;
                    }
                    // Add similar checks for other menu buttons as needed
                }
                else if (exit_game ){
                    // Check if the mouse click is within the start button area
                    if (isMouseOverContinue) {
                        exit_game = FALSE;
                    } else if (isMouseOverSaveExit) {
                        save_grid("grid.txt");
                        save_score("score.txt");
                        show_all_grid_info();
                        game_status = GAME_NOT_STARTED;
                        exit_game = FALSE;
                    }
                    else if (isMouseOverExitGame){
                        game_status = GAME_NOT_STARTED;
                        exit_game = FALSE;
                    }
                }
                else if (game_status == GAME_START && game_is_over != 1 ) {
                    handle_selection();
                }
            }
            break;
    case SDL_MOUSEMOTION:
            mouseX = event.motion.x;
            mouseY = event.motion.y;

            // Check if the mouse is over the New Game button
            isMouseOverNewGame = (mouseX >= newGameButtonRect.x &&
                                  mouseX <= newGameButtonRect.x + newGameButtonRect.w &&
                                  mouseY >= newGameButtonRect.y &&
                                  mouseY <= newGameButtonRect.y + newGameButtonRect.h);
            // Check if the mouse is over the Resume Game button
            isMouseOverResumeGame = (mouseX >= resumeGameButtonRect.x &&
                                  mouseX <= resumeGameButtonRect.x + resumeGameButtonRect.w &&
                                  mouseY >= resumeGameButtonRect.y &&
                                  mouseY <= resumeGameButtonRect.y + resumeGameButtonRect.h);
            // Check if the mouse is over the Scores button
            isMouseOverScores = (mouseX >= scoresButtonRect.x &&
                                  mouseX <= scoresButtonRect.x + scoresButtonRect.w &&
                                  mouseY >= scoresButtonRect.y &&
                                  mouseY <= scoresButtonRect.y + scoresButtonRect.h);
            // Check if the mouse is over the Exit button
            isMouseOverExit= (mouseX >= exitButtonRect.x &&
                                  mouseX <= exitButtonRect.x + exitButtonRect.w &&
                                  mouseY >= exitButtonRect.y &&
                                  mouseY <= exitButtonRect.y + exitButtonRect.h);
            // Check if the mouse is over the Save&Exit button
            isMouseOverSaveExit = (mouseX >= saveExitButtonRect.x &&
                                  mouseX <= saveExitButtonRect.x + saveExitButtonRect.w &&
                                  mouseY >= saveExitButtonRect.y &&
                                  mouseY <= saveExitButtonRect.y + saveExitButtonRect.h);
            // Check if the mouse is over the Save&Exit button
            isMouseOverContinue = (mouseX >= continueButtonRect.x &&
                                  mouseX <= continueButtonRect.x + continueButtonRect.w &&
                                  mouseY >= continueButtonRect.y &&
                                  mouseY <= continueButtonRect.y + continueButtonRect.h);
            // Check if the mouse is over the Exit Game button
            isMouseOverExitGame = (mouseX >= exitGameButtonRect.x &&
                                  mouseX <= exitGameButtonRect.x + exitGameButtonRect.w &&
                                  mouseY >= exitGameButtonRect.y &&
                                  mouseY <= exitGameButtonRect.y + exitGameButtonRect.h);

            break;
    }

}

/** Draw text function */
void draw_text(const char* text, int x, int y, int fontSize) {

    TTF_Font* font = TTF_OpenFont("Poppins-BlackItalic.ttf", fontSize);
    SDL_Color textColor = {0, 0, 0, 0};  // Black color for text

    SDL_Surface* surfaceMessage = TTF_RenderText_Solid(font, text, textColor);
    SDL_Texture* message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);

    SDL_Rect messageRect = {x, y, surfaceMessage->w, surfaceMessage->h};
    SDL_RenderCopy(renderer, message, NULL, &messageRect);

    SDL_FreeSurface(surfaceMessage);
    SDL_DestroyTexture(message);
    TTF_CloseFont(font);
}

/** Draw text function */
void draw_white_text(const char* text, int x, int y, int fontSize) {

    TTF_Font* font = TTF_OpenFont("Poppins-BlackItalic.ttf", fontSize);
    SDL_Color textColor = {255, 255, 255, 0};  // Black color for text

    SDL_Surface* surfaceMessage = TTF_RenderText_Solid(font, text, textColor);
    SDL_Texture* message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);

    SDL_Rect messageRect = {x, y, surfaceMessage->w, surfaceMessage->h};
    SDL_RenderCopy(renderer, message, NULL, &messageRect);

    SDL_FreeSurface(surfaceMessage);
    SDL_DestroyTexture(message);
    TTF_CloseFont(font);
}
/** Load image function */
SDL_Texture* load_texture(const char* path) {
    // Load image
    SDL_Surface* image = IMG_Load(path);
    if (!image) {
        printf("Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError());
        return NULL;
    }
    // Create texture from surface pixels
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, image);
    if (!texture) {
        printf("Unable to create texture from %s! SDL Error: %s\n", path, SDL_GetError());
    }
    // Free loaded surface
    SDL_FreeSurface(image);

    return texture;
}

/** Draw image function*/
void draw_image (SDL_Texture* image,int size,int x,int y){
    SDL_Rect destinationRect = { x, y, size, size };
    // Draw
    SDL_RenderCopy(renderer, image, NULL, &destinationRect);
}

/** Draw a menu button*/
void draw_menu_button(int isMouseOver,SDL_Rect ButtonRect){
        //SDL_SetRenderDrawColor(renderer, 36, 37, 44, 0);
        // Change the color based on whether the mouse is over the button
        //SDL_SetRenderDrawColor(renderer, isMouseOver ? 0 : 36, isMouseOver ? 0 : 37, 44, 0);
        SDL_SetRenderDrawColor(renderer, isMouseOver ? 0 : 255, isMouseOver ? 0 : 255, 255, 0);

        // Fill rectangles for buttons
        SDL_RenderFillRect(renderer,&ButtonRect);
        // Set color back to black for text
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        // Render text for each button

}

/** Draw starter menu */
void draw_menu(){

    draw_menu_button(isMouseOverNewGame,newGameButtonRect);
    draw_text("New Game", newGameButtonRect.x + 50, newGameButtonRect.y + 10, 20);
    draw_menu_button(isMouseOverResumeGame,resumeGameButtonRect);
    draw_text("Resume Game", resumeGameButtonRect.x + 25, resumeGameButtonRect.y + 10, 20);
    draw_menu_button(isMouseOverScores,scoresButtonRect);
    draw_text("Scores", scoresButtonRect.x + 65, scoresButtonRect.y + 10, 20);
    draw_menu_button(isMouseOverExit,exitButtonRect);
    draw_text("Exit", exitButtonRect.x + 75, exitButtonRect.y + 10, 20);

}

/** Update soldiers on combat zone (grid)*/
void update_soldiers(){
        // Draw the selected image at the updated position
         for (int i = 0; i < GRID_SIZE; ++i) {
            for (int j = 0; j < GRID_SIZE; ++j) {
                SDL_Rect cellRect = {xOffset + i * CELL_SIZE, yOffset + j * CELL_SIZE, CELL_SIZE, CELL_SIZE};
                if(grid[i][j].filled && compare_colors(grid[i][j].soldier_color,RED)){
                    grid[i][j].image=image_texture_red;
                    draw_image(image_texture_red,70,xOffset + i * CELL_SIZE,yOffset + j * CELL_SIZE);
                }
                else if(grid[i][j].filled && compare_colors(grid[i][j].soldier_color,BLUE)){
                    grid[i][j].image = image_texture_blue;
                    draw_image(image_texture_blue,70,xOffset + i * CELL_SIZE,yOffset + j * CELL_SIZE);
                }
            }
         }
}

/** Render game interface */
void render(){
    // Play audio
    Mix_PlayChannel(-1, audio_chunk, 0);

    if (game_status == GAME_SCORES){
      load_score("score.txt");
      SDL_RenderCopy(renderer,bg_scores,NULL,NULL) ;
      // Convert the integer to a string
      char winStr[20];  // Assuming a reasonable length for the string
      sprintf(winStr, "%d", scores[0].win_number);
       // Concatenate the string and the score
      char* combinedText = malloc(strlen("Win : ") + strlen(winStr) + 1);
      strcpy(combinedText, "Win : ");
      strcat(combinedText, winStr);
      draw_white_text(combinedText,100,240,20);
      sprintf(winStr, "%d", scores[0].score);
       // Concatenate the string and the score
      combinedText = malloc(strlen("Score : ") + strlen(winStr) + 1);
      strcpy(combinedText, "Score : ");
      strcat(combinedText, winStr);
      draw_white_text(combinedText,100,290,20);
      //char winStr[20];  // Assuming a reasonable length for the string
      sprintf(winStr, "%d", scores[1].win_number);
       // Concatenate the string and the score
      combinedText = malloc(strlen("Win : ") + strlen(winStr) + 1);
      strcpy(combinedText, "Win : ");
      strcat(combinedText, winStr);
      draw_white_text(combinedText,500,240,20);

      sprintf(winStr, "%d", scores[1].score);
       // Concatenate the string and the score
      combinedText = malloc(strlen("Score : ") + strlen(winStr) + 1);
      strcpy(combinedText, "Score : ");
      strcat(combinedText, winStr);
      draw_white_text(combinedText,500,290,20);

    } else {
      SDL_RenderCopy(renderer,bg_image,NULL,NULL) ;
    }
    if (game_status == GAME_START) {
        draw_combat_zone();
        update_soldiers();

         if(exit_game){
            draw_menu_button(isMouseOverContinue,continueButtonRect);
            draw_text("Continue",continueButtonRect.x+20,continueButtonRect.y+10,20);
            draw_menu_button(isMouseOverSaveExit,saveExitButtonRect);
            draw_text("Save & exit",saveExitButtonRect.x+20,saveExitButtonRect.y+10,20);
            draw_menu_button(isMouseOverExitGame,exitGameButtonRect);
            draw_text("Exit Game",exitGameButtonRect.x+20,exitGameButtonRect.y+10,20);
         }

    } else if (game_status == GAME_NOT_STARTED){
        draw_menu();
    }
    if (game_is_over == 1 && game_status == GAME_START){
            draw_image(win_left_texture,200, -5, 200 );
            draw_image(win_right_texture,200, 605, 200 );
            //draw_image(winner,400,200,100);
    }
    SDL_RenderPresent(renderer);
}

/** Draw circle for current player */
void draw_filled_circle(int centerX, int centerY, int radius, SDL_Color color) {
    const int segments = 100;
    const float angleIncrement = 2 * M_PI / segments;

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    for (int i = 0; i < segments; ++i) {
        float angle1 = i * angleIncrement;
        float angle2 = (i + 1) * angleIncrement;

        int x1 = centerX + radius * cos(angle1);
        int y1 = centerY + radius * sin(angle1);

        int x2 = centerX + radius * cos(angle2);
        int y2 = centerY + radius * sin(angle2);

        // Draw a triangle with the center of the circle and the two points on the circumference
        SDL_RenderDrawLine(renderer, centerX, centerY, x1, y1);
        SDL_RenderDrawLine(renderer, centerX, centerY, x2, y2);
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    }
}

/** Draw the combat zone (grid)*/
void draw_combat_zone() {
    draw_text("Player 1",150,20,20);
    if(current_player==1){
        draw_filled_circle(250, 30, 12, RED);
    }
    // Draw the grid
    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE; ++j) {
            SDL_Rect cellRect = {xOffset + i * CELL_SIZE, yOffset + j * CELL_SIZE, CELL_SIZE, CELL_SIZE};
           // Set color to blank (white) or selected color
            if (grid[i][j].selected) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Yellow for selected cells
            } else {
                //SDL_SetRenderDrawColor(renderer, 36, 37, 44, 0); // Grey for other cells
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
            }

            if (grid[i][j].filled){
                draw_image(image_texture_red,45,xOffset + i * CELL_SIZE,yOffset + j * CELL_SIZE);
            }
            // Draw a filled rectangle
            SDL_RenderFillRect(renderer, &cellRect);
            // Set color back to grid color (WHITE)
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
            // Draw the grid outline
            SDL_RenderDrawRect(renderer, &cellRect);
            grid[i][j].pos.x = xOffset + i * CELL_SIZE;
            grid[i][j].pos.y = yOffset + j * CELL_SIZE;
        }

   }
    draw_text("Player 2",550,550,20);
    if(current_player==2){
        draw_filled_circle(530, 570, 12, BLUE);
    }

}

int main(int argc, char *argv[]) {

    game_is_running = initialize_window();

    // Load images
    bg_image = load_texture("oggy.png");
    bg_scores = load_texture("bg-scores.jpg");
    image_texture_red = load_texture("zay-zay.png");
    image_texture_blue = load_texture("zay-zay-zay.png");
    win_right_texture = load_texture("win-right.png");
    win_left_texture = load_texture("win-left.png");
    winner = load_texture("winner.png");

    switch (GRID_SIZE){
    case 6:
        player_number=6;
        break;
    case 8:
        player_number=10;
        break;
    case 10:
        player_number=15;
        break;
    }

    while(game_is_running){
        process_input();
        render();
    }

    destroy_window();

    return 0;
}
