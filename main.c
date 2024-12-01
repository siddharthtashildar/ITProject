#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "./constants.h"
#include <time.h>
#include <SDL2/SDL_image.h> 
#include <SDL2/SDL_ttf.h>

// Global variables
int game_is_running = false;
int last_frame_time = 0;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *background_texture = NULL; // Texture for the background

int score = 0;

// Define the Car struct
typedef struct {
    int x, y;       
    int w, h;       
    int speed;      
} Car;

Car player;
Car traffic[TRAFFIC_COUNT];


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

SDL_Texture* load_texture(const char *file_path) {
    SDL_Surface *surface = IMG_Load(file_path); // Load the image file
    if (!surface) {
        printf("Error loading image: %s\n", IMG_GetError());
        return NULL;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface); // Free the surface after creating texture
    return texture;
}


// Function to poll SDL events and process keyboard input
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

                // Ensure player stays within bounds
                if (player.x < 160) player.x = 160;
                if (player.x > 480 - player.w) player.x = 480 - player.w;
                break;
        }
    }
}

bool check_collision(Car a, Car b) {
    return (a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y);
}


// Setup function that runs once at the beginning of our program
void setup(void) {

    srand(time(0));
    // Load the background image
    background_texture = load_texture("assets\\bg.bmp"); // Replace with your image path
    if (!background_texture) {
        printf("Failed to load background image.\n");
        game_is_running = false;
    }
    // Initialize player car
    player.x = WINDOW_WIDTH / 2 - CAR_WIDTH / 2;
    player.y = WINDOW_HEIGHT - CAR_HEIGHT - 20;
    player.w = CAR_WIDTH;
    player.h = CAR_HEIGHT;
    player.speed = 10;

    // Initialize traffic cars
    // for (int i = 0; i < TRAFFIC_COUNT; i++) {
    //     int temp_pos = ROAD_START +(rand() % (ROAD_END - CAR_WIDTH));
    //     if(temp_pos - CAR_WIDTH < ROAD_END && temp_pos > ROAD_START ){
    //         traffic[i].x = temp_pos;
    //         traffic[i].y = -(rand() % 300 + 100); // Spawn off-screen
    //         traffic[i].w = CAR_WIDTH;
    //         traffic[i].h = CAR_HEIGHT;
    //         traffic[i].speed = rand() % 5 + 3; // Random speed
    //     }

    // }
    for (int i = 0; i < TRAFFIC_COUNT; i++) {
        int valid_position = 0;
        while (!valid_position) {
            valid_position = 1; // Assume the position is valid
            int temp_pos = ROAD_START + (rand() % (ROAD_END - CAR_WIDTH));
            traffic[i].x = temp_pos;
            traffic[i].y = -(rand() % 300 + 100);
            traffic[i].w = CAR_WIDTH;
            traffic[i].h = CAR_HEIGHT;
            traffic[i].speed = rand() % 5 + 3;

            // Check for overlaps with other traffic cars
            for (int j = 0; j < i; j++) {
                if (check_collision(traffic[i], traffic[j])) {
                    valid_position = 0; // Overlap found, retry
                    break;
                }
            }
        }
    }

}


// Update function with a fixed time step
void update(void) {

    // Move traffic cars
    for (int i = 0; i < TRAFFIC_COUNT; i++) {
        int temp_pos = ROAD_START+(rand() % (ROAD_END - CAR_WIDTH));
        traffic[i].y += traffic[i].speed;
        // Respawn car if it moves off-screen
        if (traffic[i].y > WINDOW_HEIGHT) {
            score += 10;
            if( temp_pos - CAR_WIDTH < ROAD_END && temp_pos > ROAD_START ){
                traffic[i].x = temp_pos;
                //traffic[i].x = rand() % (WINDOW_WIDTH - CAR_WIDTH);
                traffic[i].y = -(rand() % 300 + 100);
                traffic[i].speed = rand() % 5 + 3;
            }

        }
        // Check collision
        if (check_collision(player, traffic[i])) {
            
            printf("Game Over!\n");
            game_is_running = false;
        }
        for (int j = 0; j < i; j++) {
                if (check_collision(traffic[i], traffic[j])) {
                    if(traffic[i].y > traffic[j].y){
                        traffic[j].y -= 30;
                        traffic[j].speed = traffic[i].speed;
                    }
                    else{
                        traffic[i].y -= 30;
                        traffic[i].speed = traffic[j].speed;
                    }
                    
                }
            }
    }

}

// Render the score on the screen
void render_score(void) {
    // Set the font and size
    TTF_Font *font = TTF_OpenFont("assets\\RetroGaming.ttf", 24); // Replace with your font file path
    if (!font) {
        printf("Error loading font: %s\n", TTF_GetError());
        return;
    }

    // Set the score text
    char score_text[50];
    sprintf(score_text, "Score: %d", score);

    // Create the text surface
    SDL_Color white = {255, 255, 255, 255}; // White color
    SDL_Surface *surface = TTF_RenderText_Solid(font, score_text, white);
    if (!surface) {
        printf("Error creating text surface: %s\n", TTF_GetError());
        TTF_CloseFont(font);
        return;
    }

    // Convert the surface to a texture
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) {
        printf("Error creating text texture: %s\n", SDL_GetError());
        TTF_CloseFont(font);
        return;
    }

    // Set the destination rectangle for the text
    SDL_Rect dest_rect = {600, 10, 150, 30}; // Position and size of the score display
    SDL_RenderCopy(renderer, texture, NULL, &dest_rect);

    // Clean up
    SDL_DestroyTexture(texture);
    TTF_CloseFont(font);
}

// Render function to draw game objects in the SDL window
void render(void) {

    // Render the background
    SDL_RenderCopy(renderer, background_texture, NULL, NULL);

    render_score();
    // Draw player car
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Green player
    SDL_Rect player_rect = {player.x, player.y, player.w, player.h};
    SDL_RenderFillRect(renderer, &player_rect);

    // Draw traffic cars
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red traffic
    for (int i = 0; i < TRAFFIC_COUNT; i++) {
        SDL_Rect traffic_rect = {traffic[i].x, traffic[i].y, traffic[i].w, traffic[i].h};
        SDL_RenderFillRect(renderer, &traffic_rect);
    }
    
    SDL_RenderPresent(renderer);
}


// Function to destroy SDL window and renderer
void destroy_window(void) {
    if (background_texture) {
        SDL_DestroyTexture(background_texture);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}


// Main function
int main(int argc, char* args[]) {
    game_is_running = initialize_window();

    setup();

    Uint32 frame_start;
    int frame_time;

    while (game_is_running) {
        frame_start = SDL_GetTicks();

        process_input();
        update();
        render();

        frame_time = SDL_GetTicks() - frame_start;
        if (FRAME_DELAY > frame_time) {
            SDL_Delay(FRAME_DELAY - frame_time);
        }
    }

    destroy_window();

    return 0;
}