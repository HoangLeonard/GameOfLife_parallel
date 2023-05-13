#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <SFML/Graphics.hpp>
#include <cstring>
#include <chrono>
#include <omp.h>

#define SCREEN_WIDTH 1366
#define SCREEN_HEIGHT 768

using namespace std;

// Define constants
const double cellSize = 6;
const float delayTime = 0.07;

vector<vector<bool>> read_rle_file(const string& file_name) {
    vector<vector<bool>> matrix;
    ifstream file(file_name);
    if (!file) {
        cout << "Cannot open file: " << file_name << endl;
        return matrix;
    }

    string line;
    int x = 0, y = 0;
    int rows = 0, cols = 0;
    while (getline(file, line)) {
        if (line[0] == '#' || line.empty()) continue;
        else if (line[0] == 'x') {
            // khởi tạo và gán giá trị mặc định là false
            sscanf(line.c_str(), "x = %d, y = %d", &cols, &rows);
            matrix = vector<vector<bool>>(rows, vector<bool>(cols, false));
        }
        else {
            // đọc số lần lặp lại của một kí tự, nếu không có mặc định là 1
            int count = 1;
            for (int i = 0; i < line.length(); i++) {
                if (isdigit(line[i])) {
                    // đọc số lần lặp lại
                    count = 0;
                    while (isdigit(line[i])) {
                        count = count * 10 + (line[i] - '0');
                        i++;
                    }
                    i--;
                }
                    // nếu liền sau là 'o' chèn count ô là true
                else if (line[i] == 'o') {
                    for (int j = 0; j < count; j++)
                        matrix[y][x+j] = true;
                    x += count;
                    count = 1;
                    // nếu liền sau là 'b' nhảy qua count kí tự
                } else if (line[i] == 'b') {
                    x += count;
                    count = 1;
                }
                    // nếu liền sau là '$' nhảy đến đầu dòng dưới count lần
                else if (line[i] == '$') {
                    for (int j = 0; j < count; j++) {
                        y += 1;
                        x = 0;
                    }
                    count = 1;
                }
                    // nếu đọc được '!' kết thúc.
                else if (line[i] == '!') {
                    break;
                }
            }
        }
    }
    file.close();
    return matrix;
}

int getNeighbours(vector<vector<bool>>& matrix, int row, int col) {
    int numCols = matrix[0].size();
    int numRows = matrix.size();

    int count = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0) {
                continue;
            }
            int x = row + i;
            int y = col + j;
            if (x < 0 || x >= numRows || y < 0 || y >= numCols) {
                continue;
            }
            if (matrix[x][y] == 1) {
                count++;
            }
        }
    }
    return count;
}

void updateBoard(vector<vector<bool>>& gameBoard) {
    vector<vector<bool>> newBoard(gameBoard.size(), vector<bool>(gameBoard[0].size(), false));
    int numRows = gameBoard.size();
    int numCols = gameBoard[0].size();


    for (int i = 0; i < numRows; i++) {
        for (int j = 0; j < numCols; j++) {
            int neighbours = getNeighbours(gameBoard, i, j);
            if (gameBoard[i][j]) {
                if (neighbours < 2 || neighbours > 3) {
                    newBoard[i][j] = 0;
                } else {
                    newBoard[i][j] = 1;
                }
            } else {
                if (neighbours == 3) {
                    newBoard[i][j] = 1;
                } else {
                    newBoard[i][j] = 0;
                }
            }
        }
    }

    gameBoard.clear();
    gameBoard = newBoard;
}

int main() {
    vector<vector<bool>> gameBoard = read_rle_file("/home/minhhn/Documents/ParCom/GameOfLife/Sample/fermat-primes.rle");
    int numCols = gameBoard[0].size();
    int numRows = gameBoard.size();

    sf::RenderWindow window(sf::VideoMode(numCols * cellSize, numRows * cellSize), "Game Of Life", sf::Style::Fullscreen);

    sf::Clock clock;

    while (window.isOpen()) {
        sf::Time elapsed = clock.getElapsedTime();
        if (elapsed.asSeconds() >= delayTime) {
            updateBoard(gameBoard);
            clock.restart();
        }

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        window.clear();

        for (int i = 0; i < numRows; i++) {
            for (int j = 0; j < numCols; j++) {
                if (gameBoard[i][j]) {
                    sf::RectangleShape cell(sf::Vector2f(0.9*cellSize, 0.9*cellSize));
                    cell.setPosition(j * cellSize, i * cellSize);
                    cell.setFillColor(sf::Color::White);
                    cell.setOutlineThickness(0);
                    window.draw(cell);
                }
            }
        }
        window.display();
    }
}