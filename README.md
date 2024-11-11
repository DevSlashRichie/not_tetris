# README for Tetris-like Console Game

## Overview

This project is a console-based Tetris-like game built in C++. The game involves moving and rotating shapes, represented by different figures, as they fall within a grid. The player can control the movement of figures using the arrow keys (`a`, `d`, `w` for left, right, and rotate, respectively) and can quit the game by pressing `q`.

## Features

- **Dynamic Figures:** Includes various shapes (L, T, O, I, Z) that can be spawned at random.
- **Real-time Input Handling:** Captures user input for controlling the falling figures.
- **Grid Representation:** Displays a 2D grid with falling figures using ASCII characters.
- **Collision Detection:** Detects when figures collide with the grid boundaries or other figures.
- **Smooth Rotation:** Allows for rotation of the figures while adjusting their position to prevent out-of-bounds errors.

## Required Objects

### 1. **Pixel**
### 2. **Figure**
### 3. **Board**
### 4. **Input Listener (Thread)**

## Instructions

1. **Movement:**
   - Press `a` to move the figure left.
   - Press `d` to move the figure right.
   - Press `w` to rotate the figure.

2. **Quit Game:**
   - Press `q` to quit the game.

3. **Game Loop:**
   - The figures will fall down automatically.
   - The game continues until the player quits by pressing `q`.

## Installation

1. Clone this repository to your local machine.
2. Open a terminal and navigate to the directory containing the program.
3. Compile the program with a C++ compiler (e.g., `g++ -o tetris game.cpp`).
4. Run the program with `./tetris`.

## Notes

- The game uses `termios` to enable raw mode for real-time input without waiting for the Enter key.
- The game is displayed in the terminal using simple ASCII characters.
- You can modify the game speed by adjusting the `usleep()` value in the game loop.
