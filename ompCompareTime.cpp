//
// Created by minhhn on 5/5/23.
//
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
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

void updateBoard_par_div4(bool**& gameBoard) {
    bool** newBoard = new bool*[numRows];
    for (int i = 0; i < numRows; i++) {
        newBoard[i] = new bool[numCols];
    }

    #pragma omp parallel sections num_threads(4)
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
            for (int i = 0; i < numRows/2; i++) {
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

void updateBoard_seq_standard(bool**& gameBoard) {
    bool** newBoard = new bool*[numRows];
    for (int i = 0; i < numRows; i++) {
        newBoard[i] = new bool[numCols];
    }

    for (int i = 0; i < numRows; i++) {
        for (int j = 0; j < numCols; j++) {
            int neighbours = getNeighbours(gameBoard, i, j);
            if (gameBoard[i][j] == 1) {
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

    for (int i=0; i<numRows; i++) delete[] gameBoard[i];
    delete gameBoard;
    gameBoard = newBoard;
}

void updateBoard_seq_row_by_row(bool**& gameBoard) {
    bool** tmp = new bool*[2];
    tmp[0] = new bool[numCols];
    tmp[1] = new bool[numCols];

    for (int i = 0; i < numRows; i++) {
        for (int j = 0; j < numCols; j++) {
            int neighbours = getNeighbours(gameBoard, i, j);
            if (gameBoard[i][j] == 1) {
                if (neighbours < 2 || neighbours > 3) tmp[i%2][j] = 0;
                else tmp[i%2][j] = 1;
            } else {
                if (neighbours == 3) tmp[i%2][j] = 1;
                else tmp[i%2][j] = 0;
            }

            if (i > 0) gameBoard[i-1][j] = tmp[(i-1)%2][j];
        }
    }

    for (int j = 0; j < numCols; j++) gameBoard[numRows-1][j] = tmp[(numRows-1)%2][j];

    delete[] tmp[0];
    delete[] tmp[1];
    delete[] tmp;
}

void updateBoard_par_row_by_row_div4(bool**& gameBoard) {
    bool** tmp = new bool*[8];
    for (int i = 0; i < 8; i++)
        tmp[i] = new bool[numCols];

    #pragma omp parallel sections num_threads(4)
    {
        #pragma omp section
        {
            for (int i = 0; i < numRows/2; i++) {
                for (int j = 0; j < numCols/2; j++) {
                    int neighbours = getNeighbours(gameBoard, i, j);
                    if (gameBoard[i][j] == 1) {
                        if (neighbours < 2 || neighbours > 3) tmp[i%2][j] = 0;
                        else tmp[i%2][j] = 1;
                    } else {
                        if (neighbours == 3) tmp[i%2][j] = 1;
                        else tmp[i%2][j] = 0;
                    }

                    if (i > 0) gameBoard[i-1][j] = tmp[(i-1)%2][j];
                }
            }

            for (int j = 0; j < numCols/2; j++) gameBoard[numRows/2-1][j] = tmp[(numRows/2-1)%2][j];
        }

        #pragma omp section
        {
            for (int i = 0; i < numRows/2; i++) {
                for (int j = numCols/2; j < numCols; j++) {
                    int neighbours = getNeighbours(gameBoard, i, j);
                    if (gameBoard[i][j] == 1) {
                        if (neighbours < 2 || neighbours > 3) tmp[2+i%2][j] = 0;
                        else tmp[2+i%2][j] = 1;
                    } else {
                        if (neighbours == 3) tmp[2+i%2][j] = 1;
                        else tmp[2+i%2][j] = 0;
                    }

                    if (i > 0) gameBoard[i-1][j] = tmp[2+(i-1)%2][j];
                }
            }

            for (int j = numCols/2; j < numCols; j++) gameBoard[numRows-1][j] = tmp[2+(numRows-1)%2][j];
        }

        #pragma omp section
        {
            for (int i = numRows/2; i < numRows; i++) {
                for (int j = 0; j < numCols/2; j++) {
                    int neighbours = getNeighbours(gameBoard, i, j);
                    if (gameBoard[i][j] == 1) {
                        if (neighbours < 2 || neighbours > 3) tmp[4+i%2][j] = 0;
                        else tmp[4+i%2][j] = 1;
                    } else {
                        if (neighbours == 3) tmp[4+i%2][j] = 1;
                        else tmp[4+i%2][j] = 0;
                    }

                    if (i > 0) gameBoard[i-1][j] = tmp[4+(i-1)%2][j];
                }
            }

            for (int j = 0; j < numCols/2; j++) gameBoard[numRows-1][j] = tmp[4+(numRows-1)%2][j];
        }

        #pragma omp section
        {
            for (int i = numRows/2; i < numRows; i++) {
                for (int j = numCols/2; j < numCols; j++) {
                    int neighbours = getNeighbours(gameBoard, i, j);
                    if (gameBoard[i][j] == 1) {
                        if (neighbours < 2 || neighbours > 3) tmp[6+i%2][j] = 0;
                        else tmp[6+i%2][j] = 1;
                    } else {
                        if (neighbours == 3) tmp[6+i%2][j] = 1;
                        else tmp[6+i%2][j] = 0;
                    }

                    if (i > 0) gameBoard[i-1][j] = tmp[6+(i-1)%2][j];
                }
            }

            for (int j = numCols/2; j < numCols; j++) gameBoard[numRows-1][j] = tmp[6+(numRows-1)%2][j];
        }
    }

    for (int i=0; i<8; i++) delete[] tmp[i];
    delete[] tmp;
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
    bool **gameBoard;
    cout<< "# omp Compare time:\n";
    cout<< "size(nxn) seq_standard seq_row_by_row par_div4 par_row_by_row_div4\n";
    for (int l = 0; l<3; l++) {
        int n = 0;
        while (n < 1000) {
            n += 100;
            cout << n << " ";
            gameBoard = generateRandomMatrix(n);

            auto start = std::chrono::high_resolution_clock::now();
            updateBoard_seq_standard(gameBoard);
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            cout << duration.count() << " ";

            start = std::chrono::high_resolution_clock::now();
            updateBoard_seq_row_by_row(gameBoard);
            end = std::chrono::high_resolution_clock::now();
            duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            cout << duration.count() << " ";


            start = std::chrono::high_resolution_clock::now();
            updateBoard_par_div4(gameBoard);
            end = std::chrono::high_resolution_clock::now();
            duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            cout << duration.count() << " ";

            start = std::chrono::high_resolution_clock::now();
            updateBoard_par_row_by_row_div4(gameBoard);
            end = std::chrono::high_resolution_clock::now();
            duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            cout << duration.count() << "\n";

            for (int i = 0; i < numRows; i++) delete[] gameBoard[i];
            delete gameBoard;
        }
    }
}