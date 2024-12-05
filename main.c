#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "./constants.h"
#include <time.h>
#include <SDL2/SDL_image.h> 
#include <SDL2/SDL_ttf.h>


int game_is_running = false;
int last_frame_time = 0;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

SDL_Texture *background_texture = NULL;
SDL_Texture *PlayerTexture = NULL;
SDL_Texture *TrafficTexture = NULL;
SDL_Texture *TreeTexture = NULL;
SDL_Texture *road_texture = NULL;
int road_scroll_y = 0;

int score = 0;
int highScore = 0;

typedef struct {
    int x, y;       
    int w, h;       
    int speed;      
} Car;

typedef struct {
    int x, y;       
    int w, h;       
    int speed;      
} Tree;

Car player;
Car traffic[TRAFFIC_COUNT];
Tree tree[TREE_COUNT];

typedef enum {
    GAME_RUNNING,
    GAME_OVER,
    GAME_RETRY
} GameState;

GameState current_state = GAME_RUNNING;

int get_high_score(){
    FILE *file = fopen("highscore.txt","r");
    if (file == NULL) {
        printf("Error: Could not open file!\n");
        return -1;  
    }

    int value;
    if (fscanf(file, "%d", &value) != 1) {  
        printf("Error: Could not read from the file\n");
        fclose(file);
        return -1;  
    }

    fclose(file);  
    return value; 

}

void update_high_score(int HighScore){
    FILE *file = fopen("highscore.txt", "w");  
    if (file == NULL) {
        printf("Error: Could not open file!\n");
        return;  
    }

    fprintf(file, "%d", HighScore);  
    fclose(file);  
}

SDL_Texture* load_texture(const char *file_path) {
    SDL_Surface *surface = IMG_Load(file_path); 
    if (!surface) {
        printf("Error loading image: %s\n", IMG_GetError());
        return NULL;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface); 
    return texture;
}



bool check_collision(Car a, Car b) {
    return (a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y);
}

// initialize our SDL window
int initialize_window(void) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initializing SDL.\n");
        return false;
    }
    window = SDL_CreateWindow(
        "Traffic Racer",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        0
    );
    if (!window) {
        fprintf(stderr, "Error creating SDL Window.\n");
        return false;
    }
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        fprintf(stderr, "Error creating SDL Renderer.\n");
        return false;
    }
    if (TTF_Init() == -1) {
        printf("Error initializing SDL_ttf: %s\n", TTF_GetError());
        return false;
    }

    return true;
}

void render_game_over() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
    SDL_RenderClear(renderer);

    TTF_Font *font = TTF_OpenFont("assets/RetroGaming.ttf", 36);
    if (!font) {
        printf("Error loading font: %s\n", TTF_GetError());
        return;
    }

    SDL_Color red = {255, 0, 0, 255};

    // Game Over Text
    SDL_Surface *surface = TTF_RenderText_Solid(font, "GAME OVER", red);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dest_rect = {200, 200, 400, 50};
    SDL_RenderCopy(renderer, texture, NULL, &dest_rect);

    char score_text[100];
    sprintf(score_text, "Your Score: %d", score);
    SDL_Surface *Ssurface = TTF_RenderText_Solid(font, score_text, red);
    SDL_Texture *Stexture = SDL_CreateTextureFromSurface(renderer, Ssurface);
    SDL_Rect Sdest_rect = {250, 250, 300, 50};
    SDL_RenderCopy(renderer, Stexture, NULL, &Sdest_rect);

    char Highscore_text[100];
    sprintf(Highscore_text, "High Score: %d", highScore);
    SDL_Surface *HSsurface = TTF_RenderText_Solid(font, Highscore_text, red);
    SDL_Texture *HStexture = SDL_CreateTextureFromSurface(renderer, HSsurface);
    SDL_Rect HSdest_rect = {250, 300, 300, 50};
    SDL_RenderCopy(renderer, HStexture, NULL, &HSdest_rect);

    // Retry or Exit Text
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    surface = TTF_RenderText_Solid(font, "Press R or ENTER to Retry or ESC to Exit", red);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect retry_rect = {150, 500, 500, 50};
    SDL_RenderCopy(renderer, texture, NULL, &retry_rect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    TTF_CloseFont(font);

    SDL_RenderPresent(renderer); // Present the game-over screen
}

void process_input(void) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                game_is_running = false;
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    printf("ECS key pressed\n");
                    game_is_running = false;
                }
                const Uint8 *state = SDL_GetKeyboardState(NULL);
                if (state[SDL_SCANCODE_A]) player.x -= player.speed;
                if (state[SDL_SCANCODE_D]) player.x += player.speed;
                if (state[SDL_SCANCODE_LEFT]) player.x -= player.speed;
                if (state[SDL_SCANCODE_RIGHT]) player.x += player.speed;

              
                if (player.x < 160) player.x = 160;
                if (player.x > 480 - player.w) player.x = 480 - player.w;
                break;
        }
    }
}

void process_game_over_input() {
    // printf("In Process Input Function\n");
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                game_is_running = false;
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    printf("ECS key pressed\n");
                    game_is_running = false;
                }
                const Uint8 *state = SDL_GetKeyboardState(NULL);
                if (state[SDL_SCANCODE_R]) {
                    printf("R key detected!\n");
                    current_state = GAME_RETRY;
                };
                if(state[SDL_SCANCODE_RETURN]){
                    printf("Enter key detected!\n");
                    current_state = GAME_RETRY;  
                }

                break;
        }
    }
}

void reset_game() {
    score = 0;
    player.x = WINDOW_WIDTH / 2 - CAR_WIDTH / 2;
    player.y = WINDOW_HEIGHT - CAR_HEIGHT - 20;

    for (int i = 0; i < TRAFFIC_COUNT; i++) {
        int valid_position = 0;
        while (!valid_position) {
            valid_position = 1; 
            int temp_pos = ROAD_START + (rand() % (ROAD_END - CAR_WIDTH-50));
            traffic[i].x = temp_pos;
            traffic[i].y = -(rand() % 300 + 100);
            traffic[i].w = CAR_WIDTH;
            traffic[i].h = CAR_HEIGHT;
            traffic[i].speed = rand() % 5 + 3;

            
            for (int j = 0; j < i; j++) {
                if (check_collision(traffic[i], traffic[j])) {
                    valid_position = 0; 
                    break;
                }
            }
        }
    }
    for (int k = 0; k < TREE_COUNT; k++) {
        int valid_position = 0;
        while (!valid_position) {
            int temp_pos_tree = (rand() % (WINDOW_WIDTH));
            if (temp_pos_tree < ROAD_START- 50 || temp_pos_tree > ROAD_END+100) {
                valid_position = 1; 
                tree[k].x = temp_pos_tree;
                tree[k].y = -(rand() % 300 + 100); 
                tree[k].w = TREE_WIDTH;
                tree[k].h = TREE_HEIGHT;
                tree[k].speed = 10;
            }
        }   
    }
    current_state = GAME_RUNNING; 

}

void setup(void) {

    srand(time(0));
    
    background_texture = load_texture("assets\\bg.bmp"); 
    if (!background_texture) {
        printf("Failed to load background image.\n");
        game_is_running = false;
    }

    PlayerTexture = load_texture("assets\\playerCar.png");

    if(!PlayerTexture){
        printf("Failed to load Player Car Texture.\n");
        game_is_running = false;
    }

    TrafficTexture = load_texture("assets\\trafficCar.png");
    //TrafficTexture = load_texture("assets\\trafficCarInverted.png");
    if(!TrafficTexture){
        printf("Failed to load Traffic Car Texture.\n");
        game_is_running = false;
    }

    road_texture = load_texture("assets/road.png"); // Replace with the path to your road sprite
    if (!road_texture) {
        printf("Failed to load road texture.\n");
        game_is_running = false;
    }

    TreeTexture = load_texture("assets/tree.png");
    if(!TreeTexture){
        printf("Failed to load Tree Texture.\n");
        game_is_running = false;
    }

    player.x = WINDOW_WIDTH / 2 - CAR_WIDTH / 2;
    player.y = WINDOW_HEIGHT - CAR_HEIGHT - 20;
    player.w = CAR_WIDTH;
    player.h = CAR_HEIGHT;
    player.speed = 10;

    highScore = get_high_score();

    for (int i = 0; i < TRAFFIC_COUNT; i++) {
        int valid_position = 0;
        while (!valid_position) {
            valid_position = 1; 
            int temp_pos = ROAD_START + (rand() % (ROAD_END - CAR_WIDTH-50));
            traffic[i].x = temp_pos;
            traffic[i].y = -(rand() % 300 + 100);
            traffic[i].w = CAR_WIDTH;
            traffic[i].h = CAR_HEIGHT;
            traffic[i].speed = rand() % 5 + 3;

            
            for (int j = 0; j < i; j++) {
                if (check_collision(traffic[i], traffic[j])) {
                    valid_position = 0; 
                    break;
                }
            }
        }
    }

    for (int k = 0; k < TREE_COUNT; k++) {
        int valid_position = 0;
        while (!valid_position) {
            int temp_pos_tree = (rand() % (WINDOW_WIDTH));
            if (temp_pos_tree < ROAD_START- 50 || temp_pos_tree > ROAD_END+100) {
                valid_position = 1; 
                tree[k].x = temp_pos_tree;
                tree[k].y = -(rand() % 300 + 100); 
                tree[k].w = TREE_WIDTH;
                tree[k].h = TREE_HEIGHT;
                tree[k].speed = 10;
            }
        }   
    }

}


void update(void) {

    if (current_state == GAME_RUNNING) {
        //printf("Game Running...\n");
        srand(time(0));

        for (int i = 0; i < TRAFFIC_COUNT; i++) {

            traffic[i].y += traffic[i].speed;
        
            if (traffic[i].y > WINDOW_HEIGHT) {
            int valid_position = 0;
            int temp_pos = 0;
                while (!valid_position) {
                    valid_position = 1;
                    temp_pos = ROAD_START + (rand() % (ROAD_END));
                    if(temp_pos >= ROAD_START && temp_pos < ROAD_END){
                        traffic[i].x = temp_pos;
                    }
                    
                    traffic[i].y = -(rand() % 300 + 100);
                    
                    for (int j = 0; j < TRAFFIC_COUNT; j++) {
                        if (i != j && check_collision(traffic[i], traffic[j])) {
                            valid_position = 0;  
                            break;
                        }
                    }
                }

            traffic[i].speed = rand() % 5 + 3;
            score += 10;  // Increment score only once
        }

        for (int j = 0; j < i; j++) {
                    if (check_collision(traffic[i], traffic[j])) {
                        if(traffic[i].y > traffic[j].y){
                            traffic[j].y -= 20;
                            traffic[j].speed = traffic[i].speed;
                        }
                        else{
                            traffic[i].y -= 20;
                            traffic[i].speed = traffic[j].speed;
                        }
                        
                    }
            }


            if (check_collision(player, traffic[i])) {
                if(score > highScore){
                    update_high_score(score);
                }
                printf("Game Over!\n");
                printf("Your Score: %d\n",score);
                current_state = GAME_OVER;
                //game_is_running = false;
            }

        }
        for(int k = 0; k < TREE_COUNT;k++){
            
            tree[k].y += 10;
            int temp_pos_tree = rand() % (WINDOW_WIDTH);
            if (tree[k].y > WINDOW_HEIGHT) {
                
                if( temp_pos_tree < ROAD_START - 50 || temp_pos_tree > ROAD_END+100 ){
                    if(temp_pos_tree >= 0 && temp_pos_tree <= WINDOW_WIDTH){
                        tree[k].x = temp_pos_tree;
                        //traffic[i].x = rand() % (WINDOW_WIDTH - CAR_WIDTH);
                        tree[k].y = -(rand() % 300 + 100);
                        //tree[k].y = 0;
                        tree[k].speed = 10;
                    }

                }

            }

        }

    } 
    else if (current_state == GAME_OVER) {
        //printf("Invoking process_game_over_input\n");
        process_game_over_input();
    } 
    else if (current_state == GAME_RETRY) {
        //printf("Resetting the game...\n");
        reset_game();
    }
}


void render_score(void) {
   
    TTF_Font *font = TTF_OpenFont("assets\\RetroGaming.ttf", 24); 
    if (!font) {
        printf("Error loading font: %s\n", TTF_GetError());
        return;
    }

    char score_text[100];
    sprintf(score_text, "Your Score: %d", score);

   
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface *surface = TTF_RenderText_Solid(font, score_text, white);
    if (!surface) {
        printf("Error creating text surface: %s\n", TTF_GetError());
        TTF_CloseFont(font);
        return;
    }

    
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) {
        printf("Error creating text texture: %s\n", SDL_GetError());
        TTF_CloseFont(font);
        return;
    }

    
    SDL_Rect dest_rect = {550, 10, 170, 30};
    SDL_RenderCopy(renderer, texture, NULL, &dest_rect);

    
    SDL_DestroyTexture(texture);
    TTF_CloseFont(font);
}

void render_highScore(void) {
   
    TTF_Font *font = TTF_OpenFont("assets\\RetroGaming.ttf", 24); 
    if (!font) {
        printf("Error loading font: %s\n", TTF_GetError());
        return;
    }

    char highScore_text[100];
    sprintf(highScore_text, "High Score: %d", highScore);

   
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface *surface = TTF_RenderText_Solid(font, highScore_text, white);
    if (!surface) {
        printf("Error creating text surface: %s\n", TTF_GetError());
        TTF_CloseFont(font);
        return;
    }

    
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) {
        printf("Error creating text texture: %s\n", SDL_GetError());
        TTF_CloseFont(font);
        return;
    }

    
    SDL_Rect Highscore_dest_rect = {550, 40, 170, 30};
    SDL_RenderCopy(renderer, texture, NULL, &Highscore_dest_rect);

    
    SDL_DestroyTexture(texture);
    TTF_CloseFont(font);
}

void render(void) {

    if (current_state == GAME_RUNNING) {
        SDL_RenderCopy(renderer, background_texture, NULL, NULL);
            
        SDL_Rect road_rect = {150, road_scroll_y, 340, WINDOW_HEIGHT}; // Adjust width/height as needed
        SDL_RenderCopy(renderer, road_texture, NULL, &road_rect);

        
        SDL_Rect road_rect2 = {150, road_scroll_y - WINDOW_HEIGHT, 340, WINDOW_HEIGHT};
        SDL_RenderCopy(renderer, road_texture, NULL, &road_rect2);
        
        // road_scroll_y += 10; 
        // if (road_scroll_y >= WINDOW_HEIGHT) {
        //     road_scroll_y = 0; 
        // }
        road_scroll_y -= 10;
        if (road_scroll_y <= 0) {
            road_scroll_y = WINDOW_HEIGHT; 
        }

        for (int k = 0; k < TREE_COUNT; k++) {
            SDL_Rect tree_rect = {tree[k].x, tree[k].y, tree[k].w, tree[k].h};
            //SDL_RenderFillRect(renderer, &traffic_rect);
            SDL_RenderCopy(renderer,TreeTexture,NULL,&tree_rect);
        }

        render_score();
        render_highScore();
        
        //SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Green player
        SDL_Rect player_rect = {player.x, player.y, player.w, player.h};
        //SDL_RenderFillRect(renderer, &player_rect);
        SDL_RenderCopy(renderer,PlayerTexture,NULL,&player_rect);

        
        //SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red traffic
        for (int i = 0; i < TRAFFIC_COUNT; i++) {
            SDL_Rect traffic_rect = {traffic[i].x, traffic[i].y, traffic[i].w, traffic[i].h};
            //SDL_RenderFillRect(renderer, &traffic_rect);
            SDL_RenderCopy(renderer,TrafficTexture,NULL,&traffic_rect);
        }

        
        SDL_RenderPresent(renderer);
    } 
    else if (current_state == GAME_OVER) {
        render_game_over();
    }

}



void destroy_window(void) {
    if (background_texture) {
        SDL_DestroyTexture(background_texture);
    }
    if(PlayerTexture){
        SDL_DestroyTexture(PlayerTexture);
    }
    if(TrafficTexture){
        SDL_DestroyTexture(TrafficTexture);
    }
    if (road_texture) {
        SDL_DestroyTexture(road_texture);
    }
    if(TreeTexture){
        SDL_DestroyTexture(TreeTexture);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}



int main(int argc, char* args[]) {
    game_is_running = initialize_window();

    setup();

    Uint32 frame_start;
    int frame_time;

    while (game_is_running) {

        if (current_state == GAME_RUNNING) {
            frame_start = SDL_GetTicks();

            process_input();
            update();
            render();

            frame_time = SDL_GetTicks() - frame_start;
            if (FRAME_DELAY > frame_time) {
                SDL_Delay(FRAME_DELAY - frame_time);
            }
        } 
        else if (current_state == GAME_OVER) {
            process_game_over_input(); // Handle game over inputs
        } 
        else if (current_state == GAME_RETRY) {
            reset_game();
        }

    }

    destroy_window();

    return 0;
}