
# Space Invaders with SDL2 (Restart Mechanic & Score Display)

This repository contains a **Space Invaders**–style game built in **C** using **SDL2**, **SDL2_image**, and **SDL2_ttf**. It includes the following features:

- **Player ship** controlled with left/right arrow keys, shoots bullets with the spacebar.
- **Aliens** that move side-to-side and descend after hitting a boundary.
- Basic **collision detection** between bullets and aliens.
- **Score** and **lives** system displayed on the screen.
- **Victory** when you destroy all aliens, and **Game Over** when you lose all lives or aliens reach your row.
- A **restart mechanic** where pressing **R** after Game Over or Victory restarts the game.
- On-screen prompts for **score, lives, and restart instructions**.

---

## Features

1. **Player Movement & Shooting**
   - Use **left/right arrow keys** to move your ship.
   - Press **Space** to shoot bullets (max 5 on screen).

2. **Alien Movement**
   - Aliens move side-to-side, reversing direction and descending upon hitting the window boundary.

3. **Collisions**
   - Bullets destroy aliens, increasing your score.
   - Aliens reaching the player's row deduct a life.

4. **Victory & Game Over**
   - **Victory:** If all aliens are destroyed.
   - **Game Over:** If you lose all lives or aliens reach the bottom row.

5. **Restart Mechanic**
   - Press **R** after Game Over or Victory to restart the game.

6. **Dynamic Score & Lives Display**
   - Real-time score and lives are shown in the top-left corner.

7. **SDL2_image and SDL2_ttf Integration**
   - **SDL2_image** loads textures for the ship and aliens (`ship.png` and `alien.jpg`).
   - **SDL2_ttf** renders text for score, Game Over, and restart instructions.

---

## Prerequisites

- A C compiler (e.g., `gcc`, `clang`)
- [SDL2](https://www.libsdl.org/download-2.0.php)
- [SDL2_image](https://www.libsdl.org/projects/SDL_image/)
- [SDL2_ttf](https://www.libsdl.org/projects/SDL_ttf/)

### macOS (Homebrew)

```bash
brew install sdl2 sdl2_image sdl2_ttf
```

### Linux (Debian/Ubuntu)

```bash
sudo apt-get update
sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev
```

### Windows

Download development libraries for **SDL2**, **SDL2_image**, and **SDL2_ttf** and configure your compiler paths to include them.

---

## Project Structure

```
.
├── space_invaders.c   // Main C source file
├── ship.png           // Player ship texture
├── alien.jpg          // Alien texture
├── README.md          // This README file
```

- **`space_invaders.c`**: Main C source code for the game.
- **`ship.png`**: Texture for the player's ship.
- **`alien.jpg`**: Texture for the aliens.
- **`README.md`**: Documentation for the project.

---

## How to Build and Run

1. **Clone the repository**
   ```bash
   git clone https://github.com/<your-username>/<your-repo>.git
   cd <your-repo>
   ```

2. **Compile**
   On macOS/Linux:
   ```bash
   gcc space_invaders.c -o space_invaders        `sdl2-config --cflags --libs`        -lSDL2_image -lSDL2_ttf
   ```
   Or, specify paths manually if needed:
   ```bash
   gcc space_invaders.c -o space_invaders        -I/opt/homebrew/include        -L/opt/homebrew/lib        -lSDL2 -lSDL2_image -lSDL2_ttf
   ```

3. **Run**
   ```bash
   ./space_invaders
   ```

4. **Play**
   - Use **Left/Right Arrow Keys** to move.
   - Press **Space** to shoot bullets.
   - Press **R** after the game ends to restart.

---

## How It Works

### 1. Game States and Restart Mechanic

The game uses global variables to track its state:

- **Score (`gScore`)**: Increases by 10 for each alien destroyed.
- **Lives (`gLives`)**: Starts at 3 and decreases when aliens reach your row.
- **Game Over (`gGameOver`)**: Set to true when lives reach 0 or you win.

The **`resetGame()`** function resets all game variables (player, aliens, bullets, score, and lives) to their initial state. This function is triggered by pressing **R** after a game ends.

### 2. Texture Loading

Textures for the **ship** and **aliens** are loaded using `SDL2_image`:
```c
SDL_Texture* shipTex = loadTexture(renderer, "ship.png");
SDL_Texture* alienTex = loadTexture(renderer, "alien.jpg");
```

### 3. Text Rendering

Text such as "Score", "Game Over!", and "Press R to Restart" is rendered using `SDL2_ttf`. For example:
```c
SDL_Texture* textTexture = renderText(renderer, font, "Game Over!", color, &textW, &textH);
SDL_RenderCopy(renderer, textTexture, NULL, &dstRect);
```

### 4. Real-Time Rendering

Each frame:
- The player, bullets, and aliens are drawn on the screen.
- The score and lives are updated dynamically at the top-left corner.
- At the end of the game, the appropriate message and restart prompt are shown.

---

## Controls

- **Left/Right Arrow Keys**: Move the player ship.
- **Space**: Shoot bullets.
- **R**: Restart the game after Game Over or Victory.
- **ESC**: Quit the game.

---

## Troubleshooting

1. **Textures not loading**
   - Ensure `ship.png` and `alien.jpg` are in the same working directory as the executable.
   - Use absolute paths if necessary.

2. **Undefined symbols for `IMG_Init` or `TTF_Init`**
   - Ensure you link with `-lSDL2_image -lSDL2_ttf`.

3. **Cannot find `SDL2/SDL.h`**
   - Pass the correct include path (e.g., `-I/opt/homebrew/include` on macOS).

---

## Future Enhancements

- **Multiple Rows of Aliens**: Add more rows for greater difficulty.
- **Sound Effects**: Use `SDL2_mixer` for shooting and explosion sounds.
- **High Scores**: Add a persistent high-score system.
- **Animations**: Implement animations for explosions and alien movements.

---

## License

This project is open source under the [MIT License](LICENSE).

---

## Credits

- **SDL2** by [libsdl.org](https://www.libsdl.org/)
- **SDL2_image** for texture loading
- **SDL2_ttf** for text rendering
- Icons (`ship.png`, `alien.jpg`) are placeholders. Replace with your own graphics.
