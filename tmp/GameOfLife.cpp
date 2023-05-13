//
// Created by minhhn on 4/17/23.
//

#include <iostream>
#include "GameOfLife.h"
#define screenX 600
#define screenY 800
#define CELL_SIZE 10

sf::Color GameOfLife::bg;
sf::Color GameOfLife::fg;
sf::Color GameOfLife::alive;
sf::Color GameOfLife::born;
sf::Color GameOfLife::dead;

// Constructors / Destructors
GameOfLife::GameOfLife() {
    this->initVariables();
    this->initBackground();
    this->displayCells();
}

GameOfLife::~GameOfLife() {
    delete this->window;
}

// private functions
void GameOfLife::initVariables(){
    this->initGOL("/home/minhhn/Documents/ParCom/GameOfLife/Sample/die658.rle", 5, 5);
    sizeCell = CELL_SIZE;
    this->initWindow();
    sf::Vector2f size(window->getSize());
    bgView = sf::View(sf::FloatRect(0,0, size.x, size.y));

    bg = sf::Color::Blue;
    fg = sf::Color::Green;
    alive = sf::Color::Red;
    born = sf::Color::White;
    dead = sf::Color::Magenta;
}

void GameOfLife::initGOL(const string& fileName, int p, int q) {
    // init matrix
    matrix = GameOfLife::readMatrix(fileName);
    rows = matrix.size();
    cols = matrix[0].size();
    GameOfLife::padding(matrix, p, q);
}

void GameOfLife::initWindow() {
    this->window = new sf::RenderWindow(sf::VideoMode(screenX, screenY), "Conway's game of life!", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);
}

void GameOfLife::initBackground() {
    window->clear(GameOfLife::bg);
    sf::Vector2f size = bgView.getSize();
    window->setView(bgView);
    sf::RectangleShape background(sf::Vector2f(0.92*size.x, 0.75*size.y));
    background.setPosition(0.04*size.x, 0.06*size.y);
    background.setFillColor(sf::Color::Transparent);
    background.setOutlineThickness(5.f);
    background.setOutlineColor(fg);
    window->draw(background);
    window->display();
}

void GameOfLife::displayCells() {
    sf::Vector2f size = bgView.getSize();
    golView = sf::View(sf::FloatRect(0, 0, cols*sizeCell, rows*sizeCell));
    golView.setCenter(size.x/2.f, size.y/2.f);
    golVP = sf::FloatRect(0.04*size.x, 0.06*size.y, 0.92*size.x, 0.75*size.y);
    golView.setViewport(golVP);
    window->setView(golView);

    // draw the field
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (matrix[i][j] == 1) {
                sf::RectangleShape cell(sf::Vector2f(sizeCell, sizeCell));
                cell.setFillColor(fg);
                cell.setOutlineColor(bg);
                cell.setOutlineThickness(0.4f);
                cell.setPosition(j * sizeCell, i * sizeCell);
                window->draw(cell);
            }
        }
    }

    window->display();
}

vector<vector<int>> GameOfLife::readMatrix(const string& fileName) {
    vector<vector<int>> m;
    ifstream file(fileName);
    if (!file) {
        cout << "Cannot open file: " << fileName << endl;
        return m;
    }

    string line;
    int x = 0, y = 0;
    int rows = 0, cols = 0;
    while (getline(file, line)) {
        if (line[0] == '#') continue;
        else if (line[0] == 'x') {
            sscanf(line.c_str(), "x = %d, y = %d", &rows, &cols);
            m.resize(rows);
            for (int i = 0; i < rows; i++) {
                m[i].resize(cols);
            }
        }
        else {
            // xác định số kí tự cần chèn tiếp theo bằng prefix b/o, nếu không có mặc định là 1
            int count = 1;
            for (int i = 0; i < line.length(); i++) {
                if (isdigit(line[i])) {
                    count = 0;
                    while (isdigit(line[i])) {
                        count = count * 10 + (line[i] - '0');
                        i++;
                    }
                    i--;
                } else if (line[i] == 'o') {
                    // chèn count kí tự 1 tương ứng với 'o'
                    for (int j = 0; j < count; j++) {
                        m[y][x+j] = 1;
                    }
                    x += count;
                    count = 1;
                } else if (line[i] == 'b') {
                    // chèn count kí tự 0 tương ứng với 'b'
                    for (int j = 0; j < count; j++) {
                        m[y][x+j] = 0;
                    }
                    x += count;
                    count = 1;
                }
                else if (line[i] == '$') {
                    y += 1;
                    x = 0;
                    count = 1;
                }
                else if (line[i] == '!') {
                    break;
                }
            }
        }
    }
    file.close();
    return m;
}

void GameOfLife::padding(vector<vector<int>>& vec, int p, int q) {
    int n = vec.size(); // Số hàng của vector ban đầu
    int m = vec[0].size(); // Số cột của vector ban đầu

    // Đệm thêm p hàng ở đầu vector
    for (int i = 0; i < p; i++) {
        vector<int> temp(m); // Tạo vector tạm để thêm vào vector ban đầu
        vec.insert(vec.begin(), temp); // Thêm vector tạm vào đầu vector ban đầu
        vec.push_back(temp); // Thêm vector tạm vào cuối vector ban đầu
    }

    // Đệm thêm q cột ở đầu mỗi hàng
    for (int i = 0; i < n + 2 * p; i++) {
        for (int j = 0; j < q; j++) {
            vec[i].insert(vec[i].begin(), 0); // Thêm giá trị 0 vào đầu mỗi hàng
            vec[i].push_back(0); // Thêm giá trị 0 vào cuối mỗi hàng
        }
    }
}

// Accessors
bool GameOfLife::isRunning() const {
    return this->window->isOpen();
}

// Functions
void GameOfLife::update() {
    // Event polling
    this->pollEvents();
}

void GameOfLife::render() {
    /*
    @ return
        - clear old frame
        - render objects
        - display frame in window

        Renders the game objects.
    */
//    if (reRenderBG) {
//        initBackground();
//        reRenderBG=false;
//    }
//    if (reRenderGOL) {
//        displayCells();
//        reRenderGOL=false;
//    }
}

void GameOfLife::pollEvents() {
    while (this->window->pollEvent(this->event)) {
        switch (this->event.type) {
            case sf::Event::Closed:
                this->window->close();
                break;
            case sf::Event::KeyPressed:
                if (event.key.code == sf::Keyboard::Left) {
                    // move to previous Generations
                    break;
                } else if (event.key.code == sf::Keyboard::Right) {
                    // move to next Generations
                    break;
                } else if (event.key.code == sf::Keyboard::Up) {
                    // speed up
                    break;
                } else if (event.key.code == sf::Keyboard::Down) {
                    // speed down
                    break;
                } else if (event.key.code == sf::Keyboard::Escape) {
                    this->window->close();
                } else {
                    break;
                }
            case sf::Event::Resized:
                this->bgView.setSize(event.size.width, event.size.height);
                this->bgView.setCenter(event.size.width / 2.f, event.size.height / 2.f);
                this->reRenderBG = true;
                this->reRenderGOL = true;
                break;
        }
    }
}


