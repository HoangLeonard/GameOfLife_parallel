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
int numRows = 50;
int numCols = 50;
const double cellSize = 6;
const float delayTime = 0.07;

bool** read_rle_file(const string& file_name) {
    bool** matrix;
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
            numRows = rows;
            numCols = cols;
            matrix = new bool*[rows];
            for (int i = 0; i < rows; i++) {
                matrix[i] = new bool[cols];
                for (int j = 0; j < cols; j++) {
                    matrix[i][j] = false;
                }
            }
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

void print_matrix(bool** matrix, int n, int m) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            cout << matrix[i][j] << " ";
        }
        cout << endl;
    }
}

int getNeighbours(bool** matrix, int row, int col) {
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

void updateBoard(bool**& gameBoard) {
    bool** newBoard = new bool*[numRows];
    for (int i = 0; i < numRows; i++) {
        newBoard[i] = new bool[numCols];
    }

    #pragma omp parallel sections
    {
        #pragma omp section
        {
            for (int i = 0; i < numRows/2; i++) {
                for (int j = 0; j < numCols/2; j++) {
                    int neighbours = getNeighbours(gameBoard, i, j);
                    if (gameBoard[i][j] == 1) {
                        if (neighbours < 2 || neighbours > 3) newBoard[i][j] = 0;
                        else newBoard[i][j] = 1;
                    } else {
                        if (neighbours == 3) newBoard[i][j] = 1;
                        else newBoard[i][j] = 0;
                    }
                }
            }
        }

        #pragma omp section
        {
            for (int i = numRows; i < numRows; i++) {
                for (int j = numCols/2; j < numCols; j++) {
                    int neighbours = getNeighbours(gameBoard, i, j);
                    if (gameBoard[i][j] == 1) {
                        if (neighbours < 2 || neighbours > 3) newBoard[i][j] = 0;
                        else newBoard[i][j] = 1;
                    } else {
                        if (neighbours == 3) newBoard[i][j] = 1;
                        else newBoard[i][j] = 0;
                    }
                }
            }
        }

        #pragma omp section
        {
            for (int i = numRows/2; i < numRows; i++) {
                for (int j = 0; j < numCols/2; j++) {
                    int neighbours = getNeighbours(gameBoard, i, j);
                    if (gameBoard[i][j] == 1) {
                        if (neighbours < 2 || neighbours > 3) newBoard[i][j] = 0;
                        else newBoard[i][j] = 1;
                    } else {
                        if (neighbours == 3) newBoard[i][j] = 1;
                        else newBoard[i][j] = 0;
                    }
                }
            }
        }

        #pragma omp section
        {
            for (int i = numRows/2; i < numRows; i++) {
                for (int j = numCols/2; j < numCols; j++) {
                    int neighbours = getNeighbours(gameBoard, i, j);
                    if (gameBoard[i][j] == 1) {
                        if (neighbours < 2 || neighbours > 3) newBoard[i][j] = 0;
                        else newBoard[i][j] = 1;
                    } else {
                        if (neighbours == 3) newBoard[i][j] = 1;
                        else newBoard[i][j] = 0;
                    }
                }
            }
        }
    }

    for (int i=0; i<numRows; i++) delete[] gameBoard[i];
    delete gameBoard;
    gameBoard = newBoard;
}

bool** generateRandomMatrix(int n) {
    numCols = n;
    numRows = n;
    bool** matrix = new bool*[n];
    for (int i = 0; i < n; i++) {
        matrix[i] = new bool[n];
        for (int j = 0; j < n; j++) {
            matrix[i][j] = rand() % 2; // gán giá trị ngẫu nhiên 0 hoặc 1
        }
    }
    return matrix;
}

int main() {
//    bool** gameBoard = read_rle_file("/home/minhhn/Documents/ParCom/GameOfLife/Sample/fermat-primes.rle");
    bool **gameBoard = generateRandomMatrix(10000);
    while(true) {
        auto start = std::chrono::high_resolution_clock::now();
        updateBoard(gameBoard);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        cout << numRows << "x" << numCols << " " << duration.count() << endl;
    }
//
//    sf::RenderWindow window(sf::VideoMode(numCols * cellSize, numRows * cellSize), "Game Of Life", sf::Style::Fullscreen);
//
//    sf::Clock clock;
//
//    while (window.isOpen()) {
//        sf::Time elapsed = clock.getElapsedTime();
//        if (elapsed.asSeconds() >= delayTime) {
//            auto start = std::chrono::high_resolution_clock::now();
//            updateBoard(gameBoard);
//            auto end = std::chrono::high_resolution_clock::now();
//            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
//            cout << numCols << 'x' << numRows << " " << duration.count() << endl;
//            clock.restart();
//        }
//
//        sf::Event event;
//        while (window.pollEvent(event)) {
//            if (event.type == sf::Event::Closed) {
//                window.close();
//            }
//        }
//
//        window.clear();
//
//        for (int i = 0; i < numRows; i++) {
//            for (int j = 0; j < numCols; j++) {
//                if (gameBoard[i][j]) {
//                    sf::RectangleShape cell(sf::Vector2f(0.9*cellSize, 0.9*cellSize));
//                    cell.setPosition(j * cellSize, i * cellSize);
//                    cell.setFillColor(sf::Color::White);
//                    cell.setOutlineThickness(0);
//                    window.draw(cell);
//                }
//            }
//        }
//        window.display();
//    }
}