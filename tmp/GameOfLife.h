//
// Created by minhhn on 4/17/23.
//

#pragma once
#ifndef DOCUMENTS_GAMEOFLIFE_H
#define DOCUMENTS_GAMEOFLIFE_H

#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

class GameOfLife {
private:
    // Variables
    std::vector<std::vector<int>> matrix;
    int rows, cols;
    int sizeCell;

    // Window
    sf::RenderWindow* window{};
    sf::View golView;
    sf::FloatRect golVP;
    sf::View bgView;
    sf::Event event{};
    bool reRenderBG{};
    bool reRenderGOL{};

    // private function
    void initVariables();
    void initWindow();
    void initBackground();
    void initGOL(const string& file_name, int p, int q);
    void displayCells();
    static vector<vector<int>> readMatrix(const string& fileName);
    static void padding(vector<vector<int>>& vec, int p, int q);

public:
    // variable
    static sf::Color bg;
    static sf::Color fg;
    static sf::Color alive;
    static sf::Color born;
    static sf::Color dead;

    // Constructors / Destructors
    GameOfLife();
    virtual ~GameOfLife();

    // accessors
    bool isRunning() const;

    // functions:
    void pollEvents();
    void update();
    void render();
};

#endif //DOCUMENTS_GAMEOFLIFE_H
