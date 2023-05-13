#include <iostream>
#include <functional>
#include <cmath>
#include <unordered_map>
#include <fstream>
#include <omp.h>
#include <SFML/Graphics.hpp>
#include <chrono>


using namespace std;
class Node {
public:
    // Data members
    Node* nw;
    Node* ne;
    Node* sw;
    Node* se;
    std::vector<std::vector<int>> data;
    int depth;
    int area;
    static int numNodes;

    // Constructors
    explicit Node() : nw(nullptr), ne(nullptr), sw(nullptr), se(nullptr), data(), depth(0), area(0) {
        numNodes++;
    }

    explicit Node(Node* nw, Node* ne, Node* sw, Node* se)
            : nw(nw), ne(ne), sw(sw), se(se) {
        numNodes++;
        if (sw->depth == se->depth && se->depth == ne->depth && ne->depth == nw->depth) {
            this->depth = sw->depth + 1;
            this->area = sw->area + se->area + ne->area + nw->area;
            this->data = {};
        }
    }

    explicit Node(int a, int b, int c, int d)
            : sw(nullptr), se(nullptr), nw(nullptr), ne(nullptr), data(), depth(1), area(a + b + c + d) {
        numNodes++;
        data.resize(2, std::vector<int>(2));
        data[0][0] = a;        data[0][1] = b;        data[1][0] = c;        data[1][1] = d;
    }

    // Destructor
    ~Node() {
        numNodes--;

        if (!data.empty()) {
            unsigned long rows = data.size();
            for (int i = 0; i < rows; i++) {
                data[i].clear();
            }
            data.clear();
        }
    }

    struct NodeEqual {
        bool operator()(const Node* c, const Node* o) const {
            // debug purpose: cout << "[-] NodeEqual: so sánh " << c << " va " << o << "\n";
            if (c->depth != 1 and o->depth != 1) {
                return c == o || (c->sw == o->sw && c->se == o->se && c->nw == o->nw &&
                                  c->ne == o->ne);
            } else {
                return c->data[0][0] == o->data[0][0] && c->data[0][1] == o->data[0][1]
                    && c->data[1][0] == o->data[1][0] && c->data[1][1] == o->data[1][1];
            }
        }
    };

    struct NodeHash {
        std::size_t operator()(const Node *node) const {
            std::size_t h1, h2, h3, h4;
            if (node->nw != nullptr && node->ne != nullptr && node->sw != nullptr && node->se != nullptr) {
                h1 = std::hash<void *>()(node->sw);
                h2 = std::hash<void *>()(node->se);
                h3 = std::hash<void *>()(node->nw);
                h4 = std::hash<void *>()(node->ne);
            } else {
                if (!node->data.empty()) {
                    h1 = std::hash<int>()(node->data[0][0]);
                    h2 = std::hash<int>()(node->data[0][1]);
                    h3 = std::hash<int>()(node->data[1][0]);
                    h4 = std::hash<int>()(node->data[1][1]);
                } else {
                    // debug purpose: cout << "[-] NodeHash: gia tri bam cua " << node << " la: " << 0 << "\n";
                    return 0;
                }
            }
            // debug purpose: cout << "[-] NodeHash: gia tri bam cua " << node << " la: " << (h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3)) << "\n";
            return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
        }
    };
};

class HashLife {
public:
    // Variable
    std::vector<std::vector<int>> cells;
    std::unordered_map<Node*, Node*, Node::NodeHash, Node::NodeEqual> memoNodes;
    std::unordered_map<Node*, Node*, Node::NodeHash, Node::NodeEqual> memoRes;
    Node* quadTree;
    unsigned int generation;
    unsigned int worldDepth;




    // Canonical Nodes:
    // CN1:       CN2:        CN3:       CN4:
    //      . .        . x        x .        x x
    //      . .        . .        . .        . .
    // CN5:       CN6:        CN7:       CN8:
    //      . .        . x        x .        x x
    //      . x        . x        . x        . x
    // CN9:       CN10:       CN11:      CN12:
    //      . .        . x        x .        x x
    //      x .        x .        x .        x .
    // CN13:      CN14:       CN15:      CN16:
    //      . .        . x        x .        x x
    //      x x        x x        x x        x x
    // create Canonical Nodes:
    Node* CN1 = new Node(0,0,0,0);    Node* CN2 =  new Node(0,0,0,1);    Node* CN3 = new Node(0,0,1,0);    Node* CN4 = new Node(0,0,1,1);
    Node* CN5 = new Node(0,1,0,0);    Node* CN6 =  new Node(0,1,0,1);    Node* CN7 = new Node(0,1,1,0);    Node* CN8 = new Node(0,1,1,1);
    Node* CN9 = new Node(1,0,0,0);    Node* CN10 = new Node(1,0,0,1);   Node* CN11 = new Node(1,0,1,0);   Node* CN12 = new Node(1,0,1,1);
    Node* CN13 = new Node(1,1,0,0);   Node* CN14 = new Node(1,1,0,1);   Node* CN15 = new Node(1,1,1,0);   Node* CN16 = new Node(1,1,1,1);

    // List of canonical nodes to be used for canonization of pixels
    std::vector<Node*> CNList = {CN1, CN2, CN3, CN4, CN5, CN6, CN7, CN8, CN9, CN10, CN11, CN12, CN13, CN14, CN15, CN16};

    // auxiliary matrices to run canonimal rules
    // These are pre-alocated to try to speed up the computation
    int CM[6][6] = {{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0}};
    int CMR[6][6] = {{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0}};

    explicit HashLife(const vector<std::vector<int>> &cells) : cells(cells) {
        generation = 0;
        worldDepth = 0;
        for (Node* i : CNList) memoNodes.emplace(i, i);
    }

    static std::vector<std::vector<int>> read_cells_from_rle_file(const std::string& file_name) {
        std::vector<std::vector<int>> matrix;
        std::ifstream file(file_name);
        if (!file) {
            std::cout << "Cannot open file: " << file_name << "\n";
            return matrix;
        }

        std::string line;
        int x = 0, y = 0;
        int rows = 0, cols = 0;
        while (getline(file, line)) {
            if (line[0] == '#' || line.empty()) continue;
            else if (line[0] == 'x') {
                // khởi tạo và gán giá trị mặc định là false
                sscanf(line.c_str(), "x = %d, y = %d", &cols, &rows);
                matrix.resize(rows);
                for (int i = 0; i < rows; i++) {
                    matrix[i].resize(cols);
                    for (int j = 0; j < cols; j++) {
                        matrix[i][j] = 0;
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

    static void padding_matrix(std::vector<std::vector<int>>& matrix, unsigned long a, unsigned long b, unsigned long c, unsigned long d) {
        unsigned int rows = matrix.size();
        unsigned int cols = matrix[0].size();

        // Thêm padding vào bên trái và bên phải
        for(int i = 0; i < matrix.size(); i++) {
            // Thêm b cột 0 vào bên trái
            for(int j = 0; j < b; j++) {
                matrix[i].insert(matrix[i].begin(), 0);
            }
            // Thêm d cột 0 vào bên phải
            for(int j = 0; j < d; j++) {
                matrix[i].push_back(0);
            }
        }

        // Thêm padding vào đầu và cuối ma trận
        for(int i = 0; i < a; i++) {
            // Thêm hàng chứa (cols + b + d) phần tử 0 vào đầu ma trận
            matrix.insert(matrix.begin(), vector<int>(cols + b + d, 0));
        }
        for(int i = 0; i < c; i++) {
            // Thêm hàng chứa (cols + b + d) phần tử 0 vào cuối ma trận
            matrix.emplace_back(cols + b + d, 0);
        }
    }

    void matrix_to_quad_tree() {
        unsigned long rows = cells.size();
        unsigned long cols = cells[0].size();
        if (!(rows == cols && (rows & (rows - 1)) == 0)) {
            unsigned long m = max(rows, cols);
            unsigned long a, b, c, d;
            unsigned long p = std::ceil(std::log2(m));
            p = std::pow(2, p);
            a = (p - rows) / 2;
            c = p - a - rows;
            b = (p - cols) / 2;
            d = p - b - cols;
            padding_matrix(cells, a, b, c, d);
        }
        quadTree = makeTree(0, cells.size(), 0, cells[0].size());
        worldDepth = quadTree->depth;
        generation = 0;
    }

    Node* getCN(int a, int b, int c, int d) {
        Node * res;
        for (Node* i: CNList) {
            if (a == i->data[0][0] && b == i->data[0][1] && c == i->data[1][1] && d == i->data[1][0]) {
                res = i;
                break;
            }
        }
        return res;
    }

    Node* createNode(Node* nw, Node* ne, Node* sw, Node* se) {
        Node* node = new Node(nw, ne, sw, se);
        if (memoNodes.count(node) != 0) {
            Node* existingNode = memoNodes[node];
            delete node;
            return existingNode;
        }
        memoNodes.insert({node, node});
        return node;
    }

    Node* makeTree(int x1, int x2, int y1, int y2) {
//        cout << "[" << x1 << "," << x2 << "," << y1 << "," << y2 << "]\n";
        if (x2-x1 == y2-y1 and (x2-x1)%2 == 0 and (y2-y1)%2 == 0) {
            if (x2-x1 == 2) {
                Node* CN = getCN(cells[x1][y1], cells[x1][y2-1], cells[x2-1][y1], cells[x2-1][y2-1]);
                return CN;
            } else {
                Node* nw = makeTree(x1, x1 + (x2-x1)/2, y1, y1 + (y2-y1)/2);
                Node* ne = makeTree(x1, x1 + (x2-x1)/2, y1 + (y2-y1)/2, y2);
                Node* sw = makeTree(x1 + (x2-x1)/2, x2, y1, y1 + (y2-y1)/2);
                Node* se = makeTree(x1 + (x2-x1)/2, x2, y1 + (y2-y1)/2, y2);
                return createNode(nw, ne, sw, se);
            }
        } else {
            throw std::invalid_argument("\n[Error] CreatTree: invalid arguments.");
        }
    }

    Node* generateCN1(unsigned long depth) {
        if (depth == 0) {
            return nullptr;
        }

        Node* n1 = CN1;
        for (int i = 2; i<depth + 1; i++) {
            n1 = createNode(n1, n1, n1, n1);
        }
        return n1;
    }

    Node* addBorder(Node* node) {
        unsigned long depth = node->depth;
        Node* nodeBorder = generateCN1(depth-1);
        Node* resNW = createNode(node->nw, nodeBorder, nodeBorder, nodeBorder);
        Node* resNE = createNode(nodeBorder, node->ne, nodeBorder, nodeBorder);
        Node* resSW = createNode(nodeBorder, nodeBorder, node->sw, nodeBorder);
        Node* resSE = createNode(nodeBorder, nodeBorder, nodeBorder, node->se);
        return createNode(resNW, resNE, resSW, resSE);
    }

    Node* getCenter(Node* node) {
        if (node->depth > 2)
            return createNode(node->nw->se, node->ne->sw, node->sw->ne, node->se->nw);
        else if (node->depth == 2)
            return getCN(node->sw->data[1][1], node->se->data[1][0], node->nw->data[0][1], node->ne->data[0][0]);
        else throw std::invalid_argument("\n[Error]: depth < 2 can not get Center");
    }

    void processCM(){
        bool n1, n2, n3, n4, n5, n6, n7, n8, res;
        for (int i=1; i<5; i++) {
            for (int j=1; j<5 ; j++) {
                n1 = CM[i-1][j-1];
                n2 = CM[i][j-1];
                n3 = CM[i+1][j-1];
                n4 = CM[i+1][j];
                n5 = CM[i+1][j+1];
                n6 = CM[i][j+1];
                n7 = CM[i-1][j+1];
                n8 = CM[i-1][j];
                res = CM[i][j];
                unsigned int nAlive = n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8;
                if (nAlive < 2 || nAlive > 3) res = 0;
                else if (nAlive == 3) res = 1;
                CMR[i][j] = res;
            }
        }
    }

    void NodeToCM(Node* node) {
        if (node->depth == 2) {
            CM[1][1] = node->nw->data[0][0];
            CM[1][2] = node->nw->data[1][0];
            CM[1][3] = node->sw->data[0][0];
            CM[1][4] = node->sw->data[1][0];

            CM[2][1] = node->nw->data[0][1];
            CM[2][2] = node->nw->data[1][1];
            CM[2][3] = node->sw->data[0][1];
            CM[2][4] = node->sw->data[1][1];

            CM[3][1] = node->ne->data[0][0];
            CM[3][2] = node->ne->data[1][0];
            CM[3][3] = node->se->data[0][0];
            CM[3][4] = node->se->data[1][0];

            CM[4][1] = node->ne->data[0][1];
            CM[4][2] = node->ne->data[1][1];
            CM[4][3] = node->se->data[0][1];
            CM[4][4] = node->se->data[1][1];
        } else throw std::invalid_argument("\n[Error] nodeToAuxiliaryMatrix");
    }

    Node* CMRtoNode () {
        Node* nw = getCN(CMR[1][1], CMR[1][2], CMR[2][1], CMR[2][2]);
        Node* ne = getCN(CMR[1][3], CMR[1][4], CMR[2][3], CMR[2][4]);
        Node* sw = getCN(CMR[3][1], CMR[3][2], CMR[4][1], CMR[4][2]);
        Node* se = getCN(CMR[3][3], CMR[3][4], CMR[4][3], CMR[4][4]);

        return createNode(nw, ne, sw, se);
    }

    void NodetoMatrix(Node* node, int x, int y) {
        if (memoRes[memoNodes[node]]->depth == 1) {
            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 2; j++) {
                    cells[x+i][y+j] = memoRes[node]->data[i][j];
                }
            }
        } else {
            NodetoMatrix(node->nw, x, y);
            NodetoMatrix(node->ne, x, y + pow(2, node->depth-1));
            NodetoMatrix(node->sw, x + pow(2, node->depth-1), y);
            NodetoMatrix(node->se, x + pow(2, node->depth-1), y + pow(2, node->depth-1));
        }
    }

    Node* stepNodePar(Node* node) {
        Node* tmp_nw;
        Node* tmp_ne;
        Node* tmp_sw;
        Node* tmp_se;
        #pragma omp parallel sections num_threads(4)
        {
            #pragma omp section
            {
                tmp_nw = stepNode(node->nw);
            }

            #pragma omp section
            {
                tmp_ne = stepNode(node->ne);
            }

            #pragma omp section
            {
                tmp_sw = stepNode(node->sw);
            }

            #pragma omp section
            {
                tmp_se = stepNode(node->se);
            }
        }
        return stepNode(createNode(tmp_nw, tmp_ne, tmp_sw, tmp_se));
    }

    Node* stepNode(Node* node) {
        if (node->depth == worldDepth) {
            generation += (int) std::pow(2, worldDepth-2);
        }

        if (node->area == 0 && node->depth > 1) {
            return getCenter(node);
        }

        if (memoRes.count(node) != 0) {
            return memoRes[node];
        }

        Node* result;
        if (node->depth == 2) {
            NodeToCM(node);
            processCM();
            result = getCenter(CMRtoNode());
        } else {
            // Creat 9 auxiliary nodes to combine the result.
            Node* node11 = createNode(node->nw->nw, node->nw->ne, node->nw->sw, node->nw->se);
            Node* node12 = createNode(node->nw->ne, node->ne->nw, node->nw->se, node->ne->sw);
            Node* node13 = createNode(node->ne->nw, node->ne->ne, node->ne->sw, node->ne->se);

            Node* node21 = createNode(node->nw->sw, node->nw->se, node->sw->nw, node->sw->ne);
            Node* node22 = createNode(node->nw->se, node->ne->sw, node->sw->ne, node->se->nw);
            Node* node23 = createNode(node->ne->sw, node->ne->se, node->se->nw, node->se->ne);

            Node* node31 = createNode(node->sw->nw, node->sw->ne, node->sw->sw, node->sw->se);
            Node* node32 = createNode(node->sw->ne, node->sw->se, node->se->nw, node->se->sw);
            Node* node33 = createNode(node->se->nw, node->se->ne, node->se->sw, node->se->ne);

            // compute the auxiliary Node
            Node* res11 = stepNode(node11);
            Node* res12 = stepNode(node12);
            Node* res13 = stepNode(node13);
            Node* res21 = stepNode(node21);
            Node* res22 = stepNode(node22);
            Node* res23 = stepNode(node23);
            Node* res31 = stepNode(node31);
            Node* res32 = stepNode(node32);
            Node* res33 = stepNode(node33);

            // combine the result
            Node* tmp_nw = getCenter(createNode(res11, res12, res21, res22));
            Node* tmp_ne = getCenter(createNode(res12, res13, res22, res23));
            Node* tmp_sw = getCenter(createNode(res21, res22, res31, res32));
            Node* tmp_se = getCenter(createNode(res22, res23, res32, res33));
            result = createNode(tmp_nw, tmp_ne, tmp_sw, tmp_se);
        }
//        if (node->depth == 3 ){
//            cout <<' ';
//        }
        memoRes[memoNodes[node]] = result;
        return result;
    }
};

int Node::numNodes = 0;

int main() {
    // test hash node và equal node trong Node.
//    std::unordered_map<Node*, Node*, Node::NodeHash, Node::NodeEqual> nodeMap;
//
//    Node* CN1 = new Node(0,0,0,0);
//    Node* CN2 = new Node(0,0,0,1);
//    Node* CN3 = new Node(0,0,1,0);
//    Node* CN4 = new Node(0,0,1,1);
//    Node* CN5 = new Node(0,1,0,0);
//    Node* CN6 = new Node(0,1,0,1);
//    Node* CN7 = new Node(0,1,1,0);
//    Node* CN8 = new Node(0,1,1,1);
//    Node* CN9 = new Node(1,0,0,0);
//    Node* CN10 = new Node(1,0,0,1);
//    Node* CN11 = new Node(1,0,1,0);
//    Node* CN12 = new Node(1,0,1,1);
//    Node* CN13 = new Node(1,1,0,0);
//    Node* CN14 = new Node(1,1,0,1);
//    Node* CN15 = new Node(1,1,1,0);
//    Node* CN16 = new Node(1,1,1,1);
//
//    Node* CN1_4 = new Node(CN1, CN2, CN3, CN4);
//    Node* CN5_8 = new Node(CN5, CN6, CN7, CN8);
//    Node* CN9_12 = new Node(CN9, CN10, CN11, CN12);
//    Node* CN13_16 = new Node(CN13, CN14, CN15, CN16);
//
//    Node* CN_16_1 = new Node(CN1_4, CN1_4, CN9_12, CN13_16);
//    Node* CN_16_2 = new Node(CN1_4, CN1_4, CN9_12, CN13_16);
//
//
//    nodeMap[CN_16_1] = CN_16_1;
//    std::cout << "Da them pair: [" << CN_16_1 << " : " << CN_16_1 << "] vao nodeMap\n";
//    std::cout << CN_16_2 << " chua duoc them vao map nhung neu gia tri bam da co trong mang se tra ve key tuong ung\n";
//    std::cout << nodeMap[CN_16_2] << "\n";

    // test class HashLife
    std::vector<std::vector<int>> cells = HashLife::read_cells_from_rle_file(
        "/home/minhhn/Documents/ParCom/GameOfLife/Sample/oscillator-syntheses.rle"
    );
    HashLife game(cells);
    game.matrix_to_quad_tree();
    cout << game.memoNodes.size() << " " << Node::numNodes << " ";
    cout << game.worldDepth << " " << game.generation << " " << game.quadTree->area << " " << endl;
    auto start = std::chrono::high_resolution_clock::now();
    game.stepNodePar(game.quadTree);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    cout << game.cells.size() << 'x' << game.cells[0].size() << " " << duration.count() << endl;

//
//    game.NodetoMatrix(game.quadTree, 0, 0);
//
//    const double cellSize = 6;
//    const float delayTime = 0.07;
//    int numCols = game.cells[0].size();
//    int numRows = game.cells.size();
//
//    sf::RenderWindow window(sf::VideoMode(numCols * cellSize, numRows * cellSize), "Game Of Life", sf::Style::Fullscreen);
//
//    sf::Clock clock;
//
//    while (window.isOpen()) {
//        sf::Time elapsed = clock.getElapsedTime();
//        if (elapsed.asSeconds() >= delayTime) {
//            auto start = std::chrono::high_resolution_clock::now();
//            game.matrix_to_quad_tree();
//            game.stepNode(game.quadTree);
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
//                if (game.cells[i][j]) {
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

