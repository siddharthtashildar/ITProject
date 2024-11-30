#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "./constants.h"

///////////////////////////////////////////////////////////////////////////////
// Global variables
///////////////////////////////////////////////////////////////////////////////
int game_is_running = false;
int last_frame_time = 0;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;


///////////////////////////////////////////////////////////////////////////////
// Define the Car struct
///////////////////////////////////////////////////////////////////////////////
typedef struct {
    int x, y;       // Position
    int w, h;       // Size
    int speed;      // Speed for movement
} Car;

Car car; // Declare the car as a global variable

///////////////////////////////////////////////////////////////////////////////
// Function to initialize our SDL window
///////////////////////////////////////////////////////////////////////////////
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
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Function to poll SDL events and process keyboard input
///////////////////////////////////////////////////////////////////////////////
void process_input(void) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                game_is_running = false;
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    game_is_running = false;
                }
                break;
                // Get the current state of the keyboard
                const Uint8 *state = SDL_GetKeyboardState(NULL);

                // Move the car left (A key) or right (D key)
                if (state[SDL_SCANCODE_A]) {
                    printf("A key pressed\n");
                    car.x -= car.speed;
                }
                if (state[SDL_SCANCODE_D]) {
                    printf("D key pressed\n");
                    car.x += car.speed;
                }
                break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// Setup function that runs once at the beginning of our program
///////////////////////////////////////////////////////////////////////////////
void setup(void) {

    // Initialize car properties
    car.x = WINDOW_WIDTH / 2 - 25; // Center horizontally
    car.y = WINDOW_HEIGHT - 300;  // Near the bottom
    car.w = 50;                   // Width of the car
    car.h = 100;                  // Height of the car
    car.speed = 10;               // Speed for sideways movement

}

///////////////////////////////////////////////////////////////////////////////
// Update function with a fixed time step
///////////////////////////////////////////////////////////////////////////////
void update(void) {
    // Ensure car stays within screen bounds
    if (car.x < 0) car.x = 0;
    if (car.x > WINDOW_WIDTH - car.w) car.x = WINDOW_WIDTH - car.w;

}

///////////////////////////////////////////////////////////////////////////////
// Render function to draw game objects in the SDL window
///////////////////////////////////////////////////////////////////////////////
void render(void) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Draw the car
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red car
    SDL_Rect car_rect = {car.x, car.y, car.w, car.h};
    SDL_RenderFillRect(renderer, &car_rect);

    SDL_RenderPresent(renderer);
}

///////////////////////////////////////////////////////////////////////////////
// Function to destroy SDL window and renderer
///////////////////////////////////////////////////////////////////////////////
void destroy_window(void) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

///////////////////////////////////////////////////////////////////////////////
// Main function
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* args[]) {
    game_is_running = initialize_window();

    setup();

    while (game_is_running) {
        process_input();
        update();
        render();
    }

    destroy_window();

    return 0;
}