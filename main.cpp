#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <termios.h>
#include <thread>
#include <unistd.h>
#include <vector>

using namespace std;

atomic<char> direction(' ');
atomic<bool> running(true);
int score = 0;
int level = 1;

// Enable raw mode for terminal in order to read input
void enableRawMode() {
  termios term;
  tcgetattr(STDIN_FILENO, &term);
  term.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
}

// Disable raw mode for terminal
void disableRawMode() {
  termios term;
  tcgetattr(STDIN_FILENO, &term);
  term.c_lflag |= ICANON | ECHO;
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
}

// Base Figure class
class Figure {
public:
  Figure(int x, int y) : x(x), y(y) {
    // Initialize matrix to zeros
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        matrix[i][j] = 0;
      }
    }
  }

  // Copy constructor for cloning figures
  Figure(const Figure &other) {
    x = other.x;
    y = other.y;
    for (int y = 0; y < 4; y++) {
      for (int x = 0; x < 4; x++) {
        matrix[y][x] = other.matrix[y][x];
      }
    }
  }

  // Move functions
  void moveLeft() { x--; }
  void moveRight() { x++; }
  void moveDown() { y++; }

  // Rotate function in order to rotate the figure
  void rotate() {
    int newMatrix[4][4] = {0};
    for (int y = 0; y < 4; y++) {
      for (int x = 0; x < 4; x++) {
        newMatrix[x][3 - y] = matrix[y][x];
      }
    }

    int minX = 4, minY = 4;
    for (int y = 0; y < 4; y++) {
      for (int x = 0; x < 4; x++) {
        if (newMatrix[y][x] == 1) {
          minY = min(minY, y);
          minX = min(minX, x);
        }
      }
    }

    // we need to shift the matrix to the left top corner
    // in order to rotate it properly
    int shiftedMatrix[4][4] = {0};
    for (int y = 0; y < 4; y++) {
      for (int x = 0; x < 4; x++) {
        if (newMatrix[y][x] == 1 && y - minY < 4 && x - minX < 4) {
          shiftedMatrix[y - minY][x - minX] = 1;
        }
      }
    }

    // Copy the rotated matrix back to the original matrix
    for (int y = 0; y < 4; y++) {
      for (int x = 0; x < 4; x++) {
        matrix[y][x] = shiftedMatrix[y][x];
      }
    }
  }

  // Check if the character at the given position is active
  bool isActiveChar(int x, int y) { return matrix[y][x] == 1; }

  // Check if the given position is active
  bool isActiveAt(int x, int y) {
    return x >= 0 && x < 4 && y >= 0 && y < 4 && matrix[y][x] == 1;
  }

  // Getters and setters
  int getX() { return x; }
  int getY() { return y; }
  void setX(int x) { this->x = x; }
  void setY(int y) { this->y = y; }
  virtual string getName() { return ""; }

protected:
  int x, y;
  int matrix[4][4];
};

// L-shaped figure
class LFigure : public Figure {
public:
  LFigure(int x, int y) : Figure(x, y) {
    matrix[0][0] = 1;
    matrix[1][0] = 1;
    matrix[2][0] = 1;
    matrix[2][1] = 1;
  }

protected:
  string getName() { return "L"; }
};

// T-shaped figure
class TFigure : public Figure {
public:
  TFigure(int x, int y) : Figure(x, y) {
    matrix[0][1] = 1;
    matrix[1][0] = 1;
    matrix[1][1] = 1;
    matrix[2][1] = 1;
  }
  string getName() { return "T"; }
};

// O-shaped figure
class OFigure : public Figure {
public:
  OFigure(int x, int y) : Figure(x, y) {
    matrix[0][0] = 1;
    matrix[0][1] = 1;
    matrix[1][0] = 1;
    matrix[1][1] = 1;
  }

  string getName() { return "O"; }
};

// I-shaped figure
class IFigure : public Figure {
public:
  IFigure(int x, int y) : Figure(x, y) {
    matrix[0][0] = 1;
    matrix[1][0] = 1;
    matrix[2][0] = 1;
    matrix[3][0] = 1;
  }

  string getName() { return "I"; }
};

// Z-shaped figure
class ZFigure : public Figure {
public:
  ZFigure(int x, int y) : Figure(x, y) {
    matrix[0][0] = 1;
    matrix[1][0] = 1;
    matrix[1][1] = 1;
    matrix[2][1] = 1;
  }

  string getName() { return "Z"; }
};

class Board {
public:
  Board(int width, int height)
      : width(width), height(height), currentFigure(nullptr),
        nextFigure(nullptr) {
    this->nextFigures = vector<Figure *>();
    board.resize(height, vector<bool>(width, false));
    initializeBoard();
  }

  // This will help us to update the current figure
  void updateCurrentFigure() {
    // clone the figure
    Figure *testFigure = new Figure(*currentFigure);

    // depending on the direction we will move the figure
    switch (direction) {
    case 'a':
      testFigure->moveLeft();
      if (!isColliding(testFigure)) {
        currentFigure->moveLeft();
      }
      break;
    case 'd':
      testFigure->moveRight();
      if (!isColliding(testFigure)) {
        currentFigure->moveRight();
      }
      break;
    case 'w':
      testFigure->rotate();
      if (!isColliding(testFigure)) {
        currentFigure->rotate();
      }
      break;
    case 's':
      testFigure->moveDown();
      if (!isColliding(testFigure)) {
        currentFigure->moveDown();
      }
      break;
    }

    // save memory
    delete testFigure;

    // if the figure is colliding we will lock it
    // but if it's not we will move it down
    testFigure = new Figure(*currentFigure);
    testFigure->moveDown();

    if (!isColliding(testFigure)) {
      currentFigure->moveDown();
    } else {
      // execute the lock
      lockFigure();
    }

    delete testFigure;
  }

  // This function will draw the board with the current state
  void draw() {
    // we refresh the screen
    system("clear");

    // draw the board
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        // if the current figure is active at the given position
        // we will draw it as active so it's a #
        if (currentFigure &&
            currentFigure->isActiveAt(x - currentFigure->getX(),
                                      y - currentFigure->getY())) {
          cout << '#';
        } else if (board[y][x]) {
          cout << '#';
          // else we will draw the board as empty which is .
        } else {
          cout << '.';
        }
      }
      cout << endl;
    }

    bool hasNext = nextFigure != nullptr;
    string nextFigureName = hasNext ? nextFigure->getName() : "";

    Figure *next = nextFigure ? nextFigure : nextFigures.back();
    cout << "Score: " << score << " | Level: " << level << " | "
         << " Next Figure " << nextFigureName << endl;
  }

private:
  int width, height;
  vector<vector<bool>> board;
  Figure *currentFigure;
  Figure *nextFigure;
  vector<Figure *> nextFigures;

  // This function will create a figure based on the type
  // and return it
  // This is useful when we know which figure to create
  // and we need to create it
  static Figure *createFigure(char type, int x, int y) {
    switch (type) {
    case 'L':
      return new LFigure(x, y);
    case 'T':
      return new TFigure(x, y);
    case 'O':
      return new OFigure(x, y);
    case 'I':
      return new IFigure(x, y);
    case 'Z':
      return new ZFigure(x, y);
    default:
      return nullptr;
    }
  }

  // This function will create a random figure
  // and return it
  // This is useful when we need to create a new figure
  // and we don't know which one to create
  static Figure *createRandomFigure(int x, int y) {
    char types[] = {'L', 'T', 'O', 'I', 'Z'};
    int index = rand() % 5;
    return createFigure(types[index], x, y);
  }

  // This function will create the next figures
  // and add them to the list
  // this is useful when we need to create the next figures
  // and we don't know which one to create
  // we will create 5 figures and add them to the list
  // and then we will extract them one by one
  void createNextFigures() {
    for (int i = 0; i < 5; i++) {
      nextFigures.push_back(createRandomFigure(0, 0));
    }
  }

  // This function will create a random figure
  // and return it
  Figure *grabNextFigure(int x, int y) {
    // this should extract the next figure from the list

    if (nextFigures.empty()) {
      createNextFigures();
    }

    // get the next figure
    Figure *figure = nextFigures.back();
    // remove it from the list
    nextFigures.pop_back();

    return figure;
  }

  // This function will initialize the board
  // and spawn a new figure
  void initializeBoard() {
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        board[y][x] = false;
      }
    }
    spawnFigure();
  }

  // This function will spawn a new figure
  // and check if the game is over
  void spawnFigure() {
    if (!nextFigure) {
      nextFigure = grabNextFigure(width / 2 - 2, 0);
    }

    currentFigure = nextFigure;
    nextFigure = grabNextFigure(width / 2 - 2, 0);

    // check if the figure is colliding
    if (isColliding(currentFigure)) {
      // if it is we will end the game
      gameOver();
    }
  }

  // This function will check if the figure is colliding
  bool isColliding(Figure *figure) {
    for (int y = 0; y < 4; y++) {
      for (int x = 0; x < 4; x++) {
        if (figure->isActiveChar(x, y)) {
          int boardX = figure->getX() + x;
          int boardY = figure->getY() + y;

          if (boardX < 0 || boardX >= width || boardY >= height) {
            return true;
          }

          if (boardY >= 0 && board[boardY][boardX]) {
            return true;
          }
        }
      }
    }
    return false;
  }

  // The lock function will lock the figure in the board
  // and check if there are any lines to clear
  void lockFigure() {
    for (int y = 0; y < 4; y++) {
      for (int x = 0; x < 4; x++) {
        if (currentFigure->isActiveChar(x, y)) {
          int boardX = currentFigure->getX() + x;
          int boardY = currentFigure->getY() + y;
          board[boardY][boardX] = true;
        }
      }
    }
    checkAndClearLines();
    delete currentFigure;
    currentFigure = nullptr;
    spawnFigure();
  }

  // this function will check if there are any lines to clear
  // and update the score this happens when the figure is locked
  // and a line is full
  void checkAndClearLines() {
    int linesCleared = 0;
    for (int y = height - 1; y >= 0; y--) {
      bool fullLine = true;
      for (int x = 0; x < width; x++) {
        if (!board[y][x]) {
          fullLine = false;
          break;
        }
      }

      if (fullLine) {
        board.erase(board.begin() + y);
        board.insert(board.begin(), vector<bool>(width, false));
        linesCleared++;
        y++;
      }
    }

    updateScore(linesCleared);
  }

  // This function will update the score based on the lines cleared
  // and the level
  void updateScore(int linesCleared) {
    switch (linesCleared) {
    case 1:
      score += 100 * level;
      break;
    case 2:
      score += 300 * level;
      break;
    case 3:
      score += 500 * level;
      break;
    case 4:
      score += 800 * level;
      break;
    }

    level = score / 1000 + 1;
  }

  // This function will end the game
  // and print the final score
  void gameOver() {
    running = false;
    cout << "Game Over! Final Score: " << score << endl;
  }
};

// This function will listen for input
// and update the direction
void inputListener() {
  while (running) {
    char key = getchar();
    if (key == 'q') {
      running = false;
    } else {
      direction = key;
    }
  }
}

int main() {
  srand(time(NULL));
  Board board(20, 22);

  // in order to read input we need to enable raw mode
  enableRawMode();

  thread inputThread(inputListener);

  // game loop
  while (running) {
    board.updateCurrentFigure();
    direction = ' ';
    board.draw();
    usleep(1000000 / (level * 2)); // Speed increases with level

    if (direction == 'q' || !running) {
      break;
    }
  }

  inputThread.join();
  disableRawMode();

  return 0;
}
