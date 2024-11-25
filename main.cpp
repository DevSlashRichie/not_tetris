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

void enableRawMode() {
  termios term;
  tcgetattr(STDIN_FILENO, &term);
  term.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
}

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

  Figure(const Figure &other) {
    x = other.x;
    y = other.y;
    for (int y = 0; y < 4; y++) {
      for (int x = 0; x < 4; x++) {
        matrix[y][x] = other.matrix[y][x];
      }
    }
  }

  virtual ~Figure() = default;

  void moveLeft() { x--; }
  void moveRight() { x++; }
  void moveDown() { y++; }

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

    int shiftedMatrix[4][4] = {0};
    for (int y = 0; y < 4; y++) {
      for (int x = 0; x < 4; x++) {
        if (newMatrix[y][x] == 1 && y - minY < 4 && x - minX < 4) {
          shiftedMatrix[y - minY][x - minX] = 1;
        }
      }
    }

    for (int y = 0; y < 4; y++) {
      for (int x = 0; x < 4; x++) {
        matrix[y][x] = shiftedMatrix[y][x];
      }
    }
  }

  bool isActiveChar(int x, int y) { return matrix[y][x] == 1; }

  bool isActiveAt(int x, int y) {
    return x >= 0 && x < 4 && y >= 0 && y < 4 && matrix[y][x] == 1;
  }

  int getX() { return x; }
  int getY() { return y; }
  void setX(int x) { this->x = x; }
  void setY(int y) { this->y = y; }

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
};

// Factory class for creating figures
class FigureFactory {
public:
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
};

class Board {
public:
  Board(int width, int height)
      : width(width), height(height), currentFigure(nullptr),
        nextFigure(nullptr) {
    board.resize(height, vector<bool>(width, false));
    initializeBoard();
  }

  void updateCurrentFigure() {
    Figure *testFigure = new Figure(*currentFigure);

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

    delete testFigure;

    testFigure = new Figure(*currentFigure);
    testFigure->moveDown();

    if (!isColliding(testFigure)) {
      currentFigure->moveDown();
    } else {
      lockFigure();
    }

    delete testFigure;
  }

  void draw() {
    system("clear");

    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        if (currentFigure &&
            currentFigure->isActiveAt(x - currentFigure->getX(),
                                      y - currentFigure->getY())) {
          cout << '#';
        } else if (board[y][x]) {
          cout << '#';
        } else {
          cout << '.';
        }
      }
      cout << endl;
    }

    cout << "Score: " << score << " | Level: " << level << endl;
  }

private:
  int width, height;
  vector<vector<bool>> board;
  Figure *currentFigure;
  Figure *nextFigure;

  Figure *createRandomFigure(int x, int y) {
    int randNum = (rand() % 5) + 1;
    switch (randNum) {
    case 1:
      return FigureFactory::createFigure('L', x, y);
    case 2:
      return FigureFactory::createFigure('T', x, y);
    case 3:
      return FigureFactory::createFigure('O', x, y);
    case 4:
      return FigureFactory::createFigure('I', x, y);
    case 5:
      return FigureFactory::createFigure('Z', x, y);
    default:
      return nullptr;
    }
  }

  void initializeBoard() {
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        board[y][x] = false;
      }
    }
    spawnFigure();
  }

  void spawnFigure() {
    if (!nextFigure) {
      nextFigure = createRandomFigure(width / 2 - 2, 0);
    }

    currentFigure = nextFigure;
    nextFigure = createRandomFigure(width / 2 - 2, 0);

    if (isColliding(currentFigure)) {
      gameOver();
    }
  }

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

  void gameOver() {
    running = false;
    cout << "Game Over! Final Score: " << score << endl;
  }
};

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

  enableRawMode();

  thread inputThread(inputListener);

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
