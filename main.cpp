#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <ios>
#include <iostream>
#include <string>
#include <termios.h>
#include <thread>
#include <tuple>
#include <unistd.h>
#include <vector>

using namespace std;

atomic<char> direction(' ');
atomic<bool> running(true);

void enableRawMode() {
  termios term;
  tcgetattr(STDIN_FILENO, &term);
  term.c_lflag &= ~(ICANON | ECHO); // disable canonical mode and echo
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
}

void disableRawMode() {
  termios term;
  tcgetattr(STDIN_FILENO, &term);
  term.c_lflag |= ICANON | ECHO; // enable canonical mode and echo
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
}

// Function to clear the terminal screen
void clearScreen() {
  cout << "\033[2J\033[1;1H"; // ANSI escape code to clear screen and reset
                              // cursor
}

struct Pixel {
  int x, y;
  char content;

  Pixel(int x, int y) {
    this->x = x;
    this->y = y;
    this->content = '.';
  }
};

class Figure {
public:
  int x, y;
  int matrix[4][4];

  Figure(int x, int y, int matrix[4][4]) {
    this->x = x;
    this->y = y;

    for (int y = 0; y < 4; y++) {
      for (int x = 0; x < 4; x++) {
        this->matrix[y][x] = matrix[y][x];
      }
    }
  }

  void moveLeft() { x--; }

  void moveRight() { x++; }

  void moveDown() { y++; }

  void rotate() {
    int newMatrix[4][4] = {0};

    // Rotate the matrix 90 degrees clockwise
    for (int y = 0; y < 4; y++) {
      for (int x = 0; x < 4; x++) {
        newMatrix[x][3 - y] = matrix[y][x];
      }
    }

    // Calculate the minimum x and y offset to shift the matrix up and left
    int minX = 4, minY = 4;
    for (int y = 0; y < 4; y++) {
      for (int x = 0; x < 4; x++) {
        if (newMatrix[y][x] == 1) {
          if (y < minY)
            minY = y;
          if (x < minX)
            minX = x;
        }
      }
    }

    // Shift the rotated matrix up and left based on the calculated offset
    int shiftedMatrix[4][4] = {0};
    for (int y = 0; y < 4; y++) {
      for (int x = 0; x < 4; x++) {
        if (newMatrix[y][x] == 1 && y - minY < 4 && x - minX < 4) {
          shiftedMatrix[y - minY][x - minX] = 1;
        }
      }
    }

    // Copy shifted matrix back into original matrix
    for (int y = 0; y < 4; y++) {
      for (int x = 0; x < 4; x++) {
        matrix[y][x] = shiftedMatrix[y][x];
      }
    }
  }

  bool isActiveChar(int x, int y) { return matrix[y][x] == 1; }

  static Figure *createL(int x, int y) {
    int matrix[4][4] = {{1, 0, 0, 0}, {1, 0, 0, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}};

    return new Figure(x, y, matrix);
  }

  static Figure *createT(int x, int y) {
    int matrix[4][4] = {{0, 1, 0, 0}, {1, 1, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}};

    return new Figure(x, y, matrix);
  }

  static Figure *createO(int x, int y) {
    int matrix[4][4] = {{1, 1, 0, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};

    return new Figure(x, y, matrix);
  }

  static Figure *createI(int x, int y) {
    int matrix[4][4] = {{1, 0, 0, 0}, {1, 0, 0, 0}, {1, 0, 0, 0}, {1, 0, 0, 0}};

    return new Figure(x, y, matrix);
  }

  static Figure *createZ(int x, int y) {
    int matrix[4][4] = {{1, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}};

    return new Figure(x, y, matrix);
  }
};

class Board {
public:
  int width, height;
  vector<vector<Pixel>> pixels;
  vector<Figure *> figures;
  Figure *currentFigure;
  string broadcastMessage;

  Board(int width, int height) {
    this->width = width;
    this->height = height;
    this->currentFigure = NULL;
    this->broadcastMessage = "Press 'q' to quit";

    for (int y = 0; y < height; y++) {
      vector<Pixel> row;
      for (int x = 0; x < width; x++) {
        Pixel pixel(x, y);
        row.push_back(pixel);
      }
      pixels.push_back(row);
    }
  }

  void fillWith(char content) {
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        pixels[y][x].content = content;
      }
    }
  }

  void spawnFigure() {
    // at the center

    int x = width / 2 - 2;
    int y = 0;

    int randNum = (rand() % 5) + 1;

    switch (randNum) {
    case 1:
      currentFigure = Figure::createL(x, y);
      break;
    case 2:
      currentFigure = Figure::createT(x, y);
      break;

    case 3:
      currentFigure = Figure::createO(x, y);
      break;

    case 4:
      currentFigure = Figure::createI(x, y);
      break;

    case 5:
      currentFigure = Figure::createZ(x, y);
      break;

    default:
      break;
    }
  }

  void updateCurrentFigure() {
    if (currentFigure == NULL) {
      spawnFigure();
    }

    placeFigure(currentFigure);

    if (!isColliding(currentFigure, 0)) {
      currentFigure->moveDown();
    }

    switch (direction) {
    case 'a':
      if (!isColliding(currentFigure, 3)) {
        currentFigure->moveLeft();
      }
      break;
    case 'd':
      if (!isColliding(currentFigure, 1)) {
        currentFigure->moveRight();
      }
      break;
    case 'w':
      currentFigure->rotate();

      int pixelsOutsideOfBounds = 0;

      for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
          if (currentFigure->isActiveChar(x, y)) {
            if (currentFigure->x + x >= width || currentFigure->x + x < 0) {
              pixelsOutsideOfBounds++;
            }
          }
        }
      }

      if (pixelsOutsideOfBounds > 0) {
        for (int i = 0; i < pixelsOutsideOfBounds; i++) {
          currentFigure->moveLeft();
        }
      }

      break;
    }
  }

  bool isColliding(Figure *figure, int direction) {
    int maxX = 0;
    int maxY = 0;

    for (int y = 0; y < 4; y++) {
      for (int x = 0; x < 4; x++) {
        if (figure->isActiveChar(x, y)) {
          maxX = max(maxX, figure->x + x);
          maxY = max(maxY, figure->y + y);
        }
      }
    }

    if (direction == 1) {
      return maxX >= width - 1;
    } else if (direction == 3) {
      return figure->x <= 0;
    } else {
      // this check if the figure is at the bottom
      return maxY >= height - 1;
    }

    return false;
  }

  void draw() {
    system("clear");

    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        cout << pixels[y][x].content;
      }

      if (y < height - 1) {
        cout << endl;
      }
    }

    cout << endl;
    cout << broadcastMessage << endl;
    cout << endl;
  }

  void placeFigure(Figure *figure) {
    for (int y = 0; y < 4; y++) {
      for (int x = 0; x < 4; x++) {
        if (figure->isActiveChar(x, y)) {
          int ny = figure->y + y;
          int nx = figure->x + x;

          broadcastMessage =
              "x: " + to_string(figure->x) + " y: " + to_string(figure->y);

          pixels[ny][nx].content = '#';
        }
      }
    }
  }

private:
  void appendMessage(string message) { broadcastMessage += message; }
};

void inputListener() {
  while (running) {
    char key = getchar();
    if (key == 'q') {
      running = false; // Stop the program if 'q' is pressed
    } else {
      direction = key; // Set the current direction
    }
  }
}

int main() {
  srand(time(NULL));
  Board board(20, 22);

  enableRawMode();

  thread inputThread(inputListener);

  while (running) {
    board.fillWith('.');

    board.updateCurrentFigure();

    // after we read it, we reset the direction
    direction = ' ';

    board.draw();
    usleep(1000000 / 5);

    if (direction == 'q' || !running) {
      break;
    }
  }

  inputThread.join();
  disableRawMode();

  return 0;
};
