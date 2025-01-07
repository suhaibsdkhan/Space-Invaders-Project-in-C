/*
    Space Invaders (SDL2) with a Restart Mechanic and Score Display
    ---------------------------------------------------------------
    - Move left/right with arrow keys, shoot with SPACE.
    - Single row of aliens moves side-to-side; descending upon hitting a boundary.
    - If all aliens are destroyed, you see "Victory!".
    - If they reach your row (or you run out of lives), "Game Over!".
    - Press 'R' after game over or victory to restart.
    - SDL2_image loads PNG/JPG textures (ship/alien).
    - SDL2_ttf draws on-screen text for messages and score.

    Compile example (macOS/Linux):
      gcc space_invaders.c -o space_invaders \
          `sdl2-config --cflags --libs` \
          -lSDL2_image -lSDL2_ttf

    Run:
      ./space_invaders
*/

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

// ------------------ Window Settings ------------------
#define WINDOW_WIDTH   640
#define WINDOW_HEIGHT  480

// ------------------ Player Settings ------------------
#define PLAYER_SPEED       5
#define PLAYER_WIDTH      32    // ship.png width
#define PLAYER_HEIGHT     32    // ship.png height
#define PLAYER_LIVES       3

// ------------------ Bullet Settings ------------------
#define BULLET_SPEED       7
#define BULLET_WIDTH       4
#define BULLET_HEIGHT     10
#define MAX_BULLETS        5

// ------------------ Alien Settings -------------------
#define ALIEN_COUNT        8
#define ALIEN_WIDTH       32    // alien.jpg width
#define ALIEN_HEIGHT      32    // alien.jpg height
#define ALIEN_START_X     50
#define ALIEN_START_Y     50
#define ALIEN_SPACING     50
#define ALIEN_SPEED        1
#define ALIEN_DESCENT     20

// ------------------ Game Logic Globals ----------------
static bool gRunning    = true;
static bool gGameOver   = false;
static int  gLives      = PLAYER_LIVES;
static int  gScore      = 0;
static int  gAlienMoveDir = 1; // +1: right, -1: left

// ------------------ Data Structures -------------------
typedef struct {
    int x, y;
    int w, h;
    int vx;  // velocity in x-axis
} Player;

typedef struct {
    int x, y;
    int w, h;
    bool active;
} Bullet;

typedef struct {
    int x, y;
    int w, h;
    bool active;
} Alien;

// ------------------ Collision Check -------------------
bool rect_collide(int x1, int y1, int w1, int h1,
                  int x2, int y2, int w2, int h2)
{
    return (x1 < x2 + w2) && (x1 + w1 > x2) &&
           (y1 < y2 + h2) && (y1 + h1 > y2);
}

// ------------------ Texture Loading -------------------
SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* path)
{
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        printf("IMG_Load failed for %s: %s\n", path, IMG_GetError());
        return NULL;
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return tex;
}

// ------------------ Text Rendering --------------------
SDL_Texture* renderText(SDL_Renderer* renderer, TTF_Font* font,
                        const char* message, SDL_Color color,
                        int* outWidth, int* outHeight)
{
    SDL_Surface* surf = TTF_RenderText_Solid(font, message, color);
    if (!surf) {
        printf("TTF_RenderText_Solid failed: %s\n", TTF_GetError());
        return NULL;
    }
    SDL_Texture* textTex = SDL_CreateTextureFromSurface(renderer, surf);
    *outWidth  = surf->w;
    *outHeight = surf->h;
    SDL_FreeSurface(surf);
    return textTex;
}

// ------------------ Game Reset Function ----------------
void resetGame(Player* player, Bullet* bullets, Alien* aliens, int alienCount)
{
    // Reset global states
    gLives      = PLAYER_LIVES;
    gScore      = 0;
    gGameOver   = false;
    gAlienMoveDir = 1;

    // Reset player
    player->w  = PLAYER_WIDTH;
    player->h  = PLAYER_HEIGHT;
    player->x  = (WINDOW_WIDTH - player->w) / 2;
    player->y  = WINDOW_HEIGHT - (player->h + 40);
    player->vx = 0;

    // Reset bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        bullets[i].active = false;
        bullets[i].x = 0;
        bullets[i].y = 0;
        bullets[i].w = BULLET_WIDTH;
        bullets[i].h = BULLET_HEIGHT;
    }

    // Reset aliens
    for (int i = 0; i < alienCount; i++) {
        aliens[i].active = true;
        aliens[i].w = ALIEN_WIDTH;
        aliens[i].h = ALIEN_HEIGHT;
        aliens[i].x = ALIEN_START_X + i * ALIEN_SPACING;
        aliens[i].y = ALIEN_START_Y;
    }
}

// ------------------ Main -----------------------------
int main(int argc, char* argv[])
{
    // 1. Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL failed: %s\n", SDL_GetError());
        return 1;
    }

    // 2. Initialize SDL_image
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("IMG_Init failed: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }

    // 3. Initialize SDL_ttf
    if (TTF_Init() == -1) {
        printf("TTF_Init failed: %s\n", TTF_GetError());
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Create Window
    SDL_Window* window = SDL_CreateWindow(
        "Space Invaders (Restart & Score)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN
    );
    if (!window) {
        printf("Window creation failed: %s\n", SDL_GetError());
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Create Renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!renderer) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Load textures
    SDL_Texture* shipTex  = loadTexture(renderer, "ship.png");
    SDL_Texture* alienTex = loadTexture(renderer, "alien.jpg");
    if (!shipTex || !alienTex) {
        SDL_DestroyTexture(shipTex);
        SDL_DestroyTexture(alienTex);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Load font
    TTF_Font* font = TTF_OpenFont("/System/Library/Fonts/Supplemental/Arial.ttf", 32);
    if (!font) {
        printf("TTF_OpenFont failed: %s\n", TTF_GetError());
        SDL_DestroyTexture(shipTex);
        SDL_DestroyTexture(alienTex);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Setup Player
    Player player;
    player.w = PLAYER_WIDTH;
    player.h = PLAYER_HEIGHT;
    player.x = (WINDOW_WIDTH - player.w) / 2;
    player.y = WINDOW_HEIGHT - (player.h + 40);
    player.vx= 0;

    // Setup Bullets
    Bullet bullets[MAX_BULLETS];
    for (int i = 0; i < MAX_BULLETS; i++) {
        bullets[i].x      = 0;
        bullets[i].y      = 0;
        bullets[i].w      = BULLET_WIDTH;
        bullets[i].h      = BULLET_HEIGHT;
        bullets[i].active = false;
    }

    // Setup Aliens (single row)
    Alien aliens[ALIEN_COUNT];
    for (int i = 0; i < ALIEN_COUNT; i++) {
        aliens[i].x      = ALIEN_START_X + i * ALIEN_SPACING;
        aliens[i].y      = ALIEN_START_Y;
        aliens[i].w      = ALIEN_WIDTH;
        aliens[i].h      = ALIEN_HEIGHT;
        aliens[i].active = true;
    }

    bool allAliensDead = false;

    // Main loop
    while (gRunning)
    {
        // 1) Events
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                gRunning = false;
            }
            else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        gRunning = false;
                        break;

                    case SDLK_LEFT:
                        player.vx = -PLAYER_SPEED;
                        break;
                    case SDLK_RIGHT:
                        player.vx = PLAYER_SPEED;
                        break;

                    case SDLK_SPACE:
                        // Fire bullet if any free slot
                        if (!gGameOver) {
                            for (int i = 0; i < MAX_BULLETS; i++) {
                                if (!bullets[i].active) {
                                    bullets[i].active = true;
                                    bullets[i].x = player.x + (player.w/2) - (bullets[i].w/2);
                                    bullets[i].y = player.y - bullets[i].h;
                                    break;
                                }
                            }
                        }
                        break;

                    case SDLK_r:
                        // Press R to restart if game over
                        if (gGameOver) {
                            resetGame(&player, bullets, aliens, ALIEN_COUNT);
                        }
                        break;

                    default:
                        break;
                }
            }
            else if (e.type == SDL_KEYUP) {
                switch (e.key.keysym.sym) {
                    case SDLK_LEFT:
                        if (player.vx < 0) player.vx = 0;
                        break;
                    case SDLK_RIGHT:
                        if (player.vx > 0) player.vx = 0;
                        break;
                    default:
                        break;
                }
            }
        }

        // 2) Update Logic if not game over
        if (!gGameOver) {
            // Move player
            player.x += player.vx;
            if (player.x < 0) player.x = 0;
            if (player.x + player.w > WINDOW_WIDTH) {
                player.x = WINDOW_WIDTH - player.w;
            }

            // Update bullets
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (bullets[i].active) {
                    bullets[i].y -= BULLET_SPEED;
                    if (bullets[i].y + bullets[i].h < 0) {
                        bullets[i].active = false;
                    }
                }
            }

            // Check if aliens need to descend
            bool needDescend = false;
            for (int i = 0; i < ALIEN_COUNT; i++) {
                if (!aliens[i].active) continue;
                int newX = aliens[i].x + ALIEN_SPEED * gAlienMoveDir;
                if (newX < 0 || (newX + aliens[i].w > WINDOW_WIDTH)) {
                    needDescend = true;
                    break;
                }
            }

            if (needDescend) {
                gAlienMoveDir = -gAlienMoveDir;
                for (int i = 0; i < ALIEN_COUNT; i++) {
                    if (aliens[i].active) {
                        aliens[i].y += ALIEN_DESCENT;
                    }
                }
            } else {
                // Move aliens horizontally
                for (int i = 0; i < ALIEN_COUNT; i++) {
                    if (aliens[i].active) {
                        aliens[i].x += ALIEN_SPEED * gAlienMoveDir;
                    }
                }
            }

            // Collision: bullet vs. aliens
            for (int b = 0; b < MAX_BULLETS; b++) {
                if (!bullets[b].active) continue;
                for (int i = 0; i < ALIEN_COUNT; i++) {
                    if (!aliens[i].active) continue;
                    if (rect_collide(bullets[b].x, bullets[b].y,
                                     bullets[b].w, bullets[b].h,
                                     aliens[i].x, aliens[i].y,
                                     aliens[i].w, aliens[i].h))
                    {
                        aliens[i].active   = false;
                        bullets[b].active = false;
                        gScore += 10;
                        break;
                    }
                }
            }

            // Check if aliens reached bottom => lose life or game over
            for (int i = 0; i < ALIEN_COUNT; i++) {
                if (aliens[i].active) {
                    if (aliens[i].y + aliens[i].h >= player.y) {
                        // Aliens reached player row
                        gLives--;
                        if (gLives <= 0) {
                            gGameOver = true;
                        } else {
                            // Reset aliens & bullets
                            for (int a = 0; a < ALIEN_COUNT; a++) {
                                aliens[a].active = true;
                                aliens[a].x = ALIEN_START_X + a * ALIEN_SPACING;
                                aliens[a].y = ALIEN_START_Y;
                            }
                            for (int b = 0; b < MAX_BULLETS; b++) {
                                bullets[b].active = false;
                            }
                        }
                        break;
                    }
                }
            }
        }

        // Check if all aliens are dead => victory
        allAliensDead = true;
        for (int i = 0; i < ALIEN_COUNT; i++) {
            if (aliens[i].active) {
                allAliensDead = false;
                break;
            }
        }
        if (allAliensDead && !gGameOver) {
            gGameOver = true;
        }

        // 3) Render
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw player
        if (shipTex) {
            SDL_Rect shipRect = { player.x, player.y, player.w, player.h };
            SDL_RenderCopy(renderer, shipTex, NULL, &shipRect);
        }

        // Draw bullets (white rects)
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (bullets[i].active) {
                SDL_Rect bulletRect = {
                    bullets[i].x, bullets[i].y,
                    bullets[i].w, bullets[i].h
                };
                SDL_RenderFillRect(renderer, &bulletRect);
            }
        }

        // Draw aliens
        for (int i = 0; i < ALIEN_COUNT; i++) {
            if (aliens[i].active && alienTex) {
                SDL_Rect alienRect = { aliens[i].x, aliens[i].y,
                                       aliens[i].w, aliens[i].h };
                SDL_RenderCopy(renderer, alienTex, NULL, &alienRect);
            }
        }

        // Draw scoreboard (top-left corner)
        {
            char scoreBuf[64];
            sprintf(scoreBuf, "Score: %d   Lives: %d", gScore, gLives);
            SDL_Color white = {255, 255, 255, 255};
            int textW = 0, textH = 0;
            SDL_Texture* scoreTex = renderText(renderer, font, scoreBuf, white, &textW, &textH);
            if (scoreTex) {
                SDL_Rect scoreRect = { 10, 10, textW, textH };
                SDL_RenderCopy(renderer, scoreTex, NULL, &scoreRect);
                SDL_DestroyTexture(scoreTex);
            }
        }

        // If game over, display "Victory!" or "Game Over!" + "Press R"
        if (gGameOver) {
            SDL_Color color = {255, 0, 0, 255}; // Red text
            const char* msg = (allAliensDead && gLives > 0) ? "Victory!" : "Game Over!";
            int textW = 0, textH = 0;
            SDL_Texture* textTexture = renderText(renderer, font, msg, color, &textW, &textH);
            if (textTexture) {
                // Center the text
                SDL_Rect dstRect = {
                    (WINDOW_WIDTH - textW)/2,
                    (WINDOW_HEIGHT - textH)/2,
                    textW,
                    textH
                };
                SDL_RenderCopy(renderer, textTexture, NULL, &dstRect);
                SDL_DestroyTexture(textTexture);
            }

            // Additional prompt: Press R to restart
            {
                SDL_Color white = {255, 255, 255, 255};
                int rw=0, rh=0;
                SDL_Texture* restartTex = renderText(renderer, font,
                                                     "Press R to restart",
                                                     white, &rw, &rh);
                if (restartTex) {
                    SDL_Rect rdst = {
                        (WINDOW_WIDTH - rw)/2,
                        (WINDOW_HEIGHT - rh)/2 + 50, // some offset below
                        rw,
                        rh
                    };
                    SDL_RenderCopy(renderer, restartTex, NULL, &rdst);
                    SDL_DestroyTexture(restartTex);
                }
            }
        }

        SDL_RenderPresent(renderer);
    }

    // Cleanup
    TTF_CloseFont(font);
    SDL_DestroyTexture(alienTex);
    SDL_DestroyTexture(shipTex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    printf("\nFinal Score: %d\n", gScore);
    return 0;
}