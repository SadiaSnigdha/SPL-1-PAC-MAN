#include<graphics.h>
#include<iostream>
#include<vector>
#include<unordered_map>
#include<ctime>
#include<fstream>
#include<string>

using namespace std;

#define CELL_SIZE 20

#define WALL 1
#define OBSTACLE 2
#define DOT 3
#define POWER_UP 4
#define RED_ZONE 5
#define EMPTY 0

vector<vector<int>> maze;
int ROWS ;
int COLS ;

int playerX = 1;
int playerY = 1;

int ghostCount;
int ghostsX[10];
int ghostsY[10];
int ghostMoveCounter = 0;
const int ghostMoveFrequency = 5; 

int DX[] = {-1, 1, 0, 0};
int DY[] = {0, 0, -1, 1}; 

int score = 0;
int level;
bool gameOver = false;
bool invincible = false;

int remainingDots = 0; 
const int MAX_RED_ZONES = 10; 
int redZoneCount ;
int redZoneX[MAX_RED_ZONES];
int redZoneY[MAX_RED_ZONES];
const int SCREEN_HEIGHT = 600;

int startTime; 
int elapsedTime;
bool redraw = true;  

void loadMazeFromFile(const string& filename);
void initializeGhostPositions();
bool isGhostAtPosition(int x, int y);
void setGhostCount(const string& level);
void showLevelSelectionForScores();
void drawScoreAndLevel(int elapsedTime);
void startGame(const string& levelFile); 
void drawTimer();
string selectLevel();
vector<int> readScoresFromFile(const string& filename);

struct Node {
    int x, y;
    int g, h;
    Node* parent;

    Node(int x, int y, int g, int h, Node* parent = nullptr)
        : x(x), y(y), g(g), h(h), parent(parent) {}

    int f() const {
        return g + h;
    }
};

void drawButton(int x, int y, int width, int height, const char* label) {
    setfillstyle(SOLID_FILL, LIGHTBLUE);
    bar(x, y, x + width, y + height);
    setcolor(BLACK);
    rectangle(x, y, x + width, y + height);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);
    setbkcolor(BLACK);
    setcolor(WHITE);
    outtextxy(x + 10, y + 10, (char*)label);
}

bool isButtonClicked(int x, int y, int width, int height, int clickX, int clickY) {
    if (clickX >= x && clickX <= x + width) {  
        if (clickY >= y && clickY <= y + height) { 
            return true;  
        }
    }
    return false;
}

void displayScores(const string& level) {
    string filename;
    if (level == "easy") {
        filename = "easy_scores.txt";
    } 
    else if (level == "medium") {
        filename = "medium_scores.txt";
    } 
    else if (level == "hard") {
        filename = "hard_scores.txt";
    }
    vector<int> scores = readScoresFromFile(filename);

    if (scores.empty()) {
        cleardevice();
        settextstyle(DEFAULT_FONT, HORIZ_DIR, 3);
        outtextxy(100, 100, (char*)"No scores available.");
        outtextxy(100, 200, (char*)"Press ESC to return to Main Menu.");
        
        while (true) {
            if (kbhit()) {
                char key = getch();
                if (key == 27) {
                    showLevelSelectionForScores();
                    return; 
                }
            }
        }
    }

    cleardevice();
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 3);
    outtextxy(100, 50, (char*)"Top Scores:");

    int numScoresToShow = min(10, (int)scores.size());

    for (int i = 0; i < numScoresToShow; i++) {
        char scoreText[50];
        sprintf(scoreText, "%d: %d", i + 1, scores[i]);
        outtextxy(100, 100 + i * 30, scoreText);
    }

    outtextxy(100, 400, (char*)"Press ESC to return to Main Menu.");

    while (true) {
        if (kbhit()) {
            char key = getch();
            if (key == 27) {
                showLevelSelectionForScores();
                return; 
            }
        }
    }
}

void displayRules() {
    cleardevice();
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 3);
    outtextxy(100, 50, (char*)"Game Rules:");
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);

    outtextxy(100, 100, (char*)"1. Navigate the maze.");
    outtextxy(100, 140, (char*)"2. Avoid red zones and ghosts.");
    outtextxy(100, 180, (char*)"3. Collect:");
    outtextxy(120, 210, (char*)"   Dots → +5 points");
    outtextxy(120, 240, (char*)"   Power-ups → +10 points & temporary power");
    outtextxy(100, 280, (char*)"4. Use W, S, A, D keys for movement:");
    outtextxy(120, 310, (char*)"   W → Up");
    outtextxy(120, 340, (char*)"   S → Down");
    outtextxy(120, 370, (char*)"   A → Left");
    outtextxy(120, 400, (char*)"   D → Right");
    outtextxy(100, 430, (char*)"5. final score = (score/time) * 100.");

    while (true) {
        if (kbhit()) {
            char key = getch();
            if (key == 27) 
                return; 
        }
    }
}

void mainMenu() {
    redraw = true;  // flag to track when to redraw the screen

    while (true) {
        if (redraw) {
            cleardevice(); 
            settextstyle(DEFAULT_FONT, HORIZ_DIR, 3);
            outtextxy(100, 50, (char*)"Main Menu");
            drawButton(100, 120, 300, 50, "1. Level Selection");
            drawButton(100, 200, 300, 50, "2. Scores");
            drawButton(100, 280, 300, 50, "3. Rules");
            drawButton(100, 360, 300, 50, "4. Exit");
            redraw = false;  
        }

        if (ismouseclick(WM_LBUTTONDOWN)) {
            int clickX, clickY;
            getmouseclick(WM_LBUTTONDOWN, clickX, clickY);
            clearmouseclick(WM_LBUTTONDOWN);

            if (isButtonClicked(100, 120, 300, 50, clickX, clickY)) {
                string levelFile = selectLevel();
                startGame(levelFile);
                redraw = true; 
            } 
            else if (isButtonClicked(100, 200, 300, 50, clickX, clickY)) {
                cleardevice();
                showLevelSelectionForScores();  
                redraw = true; 
            } 
            else if (isButtonClicked(100, 280, 300, 50, clickX, clickY)) {
                displayRules();
                redraw = true; 
            } 
            else if (isButtonClicked(100, 360, 300, 50, clickX, clickY)) {
                closegraph(); 
                exit(0);   
            }
        }

        delay(50); 
    }
}

void showLevelSelectionForScores() {
    cleardevice();
    settextstyle(DEFAULT_FONT, HORIZ_DIR , 2.5);

    // Easy button
    setfillstyle(SOLID_FILL, GREEN);
    bar(100, 100, 300, 150);
    setcolor(WHITE);
    outtextxy(160, 120, (char*)"Easy");

    // Medium button
    setfillstyle(SOLID_FILL, YELLOW);
    bar(100, 180, 300, 230);
    setcolor(WHITE);
    outtextxy(160, 200, (char*)"Medium");

    // Hard button
    setfillstyle(SOLID_FILL, RED);
    bar(100, 260, 300, 310);
    setcolor(WHITE);
    outtextxy(160, 280, (char*)"Hard");

    outtextxy(100, 350, (char*)"Press ESC to return to Main Menu."); 

    while (true) {
        if (ismouseclick(WM_LBUTTONDOWN)) {
            int x, y;
            getmouseclick(WM_LBUTTONDOWN, x, y);

            if (x >= 100 && x <= 300 && y >= 100 && y <= 150) {
                cleardevice();
                displayScores("easy");
                return;
            }

            if (x >= 100 && x <= 300 && y >= 180 && y <= 230) {
                cleardevice();
                displayScores("medium");
                return;
            }

            if (x >= 100 && x <= 300 && y >= 260 && y <= 310) {
                cleardevice();
                displayScores("hard");
                return;
            }
        }

        if (kbhit()) {
            char key = getch();
            if (key == 27) {  
                mainMenu(); 
                return;
            }
        }
    }
}

//level selection screen
string selectLevel(){
    cleardevice();
    settextstyle(DEFAULT_FONT, HORIZ_DIR , 2.5);

    //button
    setfillstyle(SOLID_FILL , GREEN);
    bar(100, 100, 300, 200);
    setcolor(WHITE);
    outtextxy(160, 140, (char*)"EASY");

    setfillstyle(SOLID_FILL, YELLOW);
    bar(350, 100, 550, 200);
    setcolor(WHITE);
    outtextxy(410, 140, (char*)"MEDIUM");

    setfillstyle(SOLID_FILL, GREEK_CHARSET);
    bar(600, 100, 800, 200);
    setcolor(WHITE);
    outtextxy(660, 140, (char*)"HARD");

    //click button
    while (true) {
        if (ismouseclick(WM_LBUTTONDOWN)) {
            int x;
            int y;
            getmouseclick(WM_LBUTTONDOWN, x, y);

            if (x >= 100 && x <= 300 && y >= 100 && y <= 200)
            {
                level = 1;
                redZoneCount = 1;
                return "easy_maze.txt"; 
            }
            else if (x >= 350 && x <= 550 && y >= 100 && y <= 200)
            {
                 level = 2;
                 redZoneCount = 2;
                return "medium_maze.txt"; 
            }
            else if (x >= 600 && x <= 800 && y >= 100 && y <= 200) {
                level = 3;
                redZoneCount = 3;
                return "hard_maze.txt"; 
            }
        }
        if (kbhit()) {
            char key = getch();
            if (key == 27) 
                return "";  
        }
    }

}

//load maze from file
void loadMazeFromFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error: Could not open file " << filename << endl;
        exit(1);
    }

    vector<string> tempMaze;  
    string line;
    
    cout << "Reading Maze File: \n";
    while (getline(file, line)) {
        //cout << line << endl;  // Debug: Print each line
        tempMaze.push_back(line);
    }
    file.close();

    ROWS = tempMaze.size();
    COLS = (ROWS > 0) ? tempMaze[0].size() : 0;
    maze.resize(ROWS, vector<int>(COLS, EMPTY));

    remainingDots = 0;
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            char cell = tempMaze[i][j];
            switch (cell) {
                case '#': 
                    maze[i][j] = WALL; 
                    break;
                case 'O': 
                    maze[i][j] = OBSTACLE; 
                    break;
                case 'P': 
                    maze[i][j] = POWER_UP; 
                    break;
                case '.': 
                    maze[i][j] = DOT; 
                    remainingDots++;  
                    break;
                case 'E': 
                    maze[i][j] = EMPTY; 
                    break;
            }
        }
    }

    if (remainingDots == 0) {
        cout << "Error: No dots found in the maze! Check your maze initialization." << endl;
        exit(1);
    }
    file.close();
    maze[playerX][playerY] = EMPTY;
        
    for (int i = 0; i < ghostCount; ++i) {
            maze[ghostsX[i]][ghostsY[i]] = EMPTY; 
    }

    srand(time(0)); 
    int count = 0;
    while (count < redZoneCount) {
        int x = rand() % ROWS;
        int y = rand() % COLS;

        if (maze[x][y] == EMPTY && !(x == playerX && y == playerY)) {
            redZoneX[count] = x;
            redZoneY[count] = y;
            maze[x][y] = RED_ZONE; 
            count++;
        }
    }
}

//maze structure
void drawMaze() {
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            int x = j * CELL_SIZE;
            int y = i * CELL_SIZE;

            switch (maze[i][j]) {
                case WALL:
                    setfillstyle(SOLID_FILL, BLUE);
                    bar(x, y, x + CELL_SIZE, y + CELL_SIZE);
                    break;
                case OBSTACLE:
                    setfillstyle(SOLID_FILL, DARKGRAY);
                    bar(x, y, x + CELL_SIZE, y + CELL_SIZE);
                    break;
                case DOT:
                    setfillstyle(SOLID_FILL, BLACK);
                    bar(x, y, x + CELL_SIZE, y + CELL_SIZE);
                    setcolor(WHITE);
                    fillellipse(x + CELL_SIZE / 2, y + CELL_SIZE / 2, 3, 3);
                    break;
                case POWER_UP:
                    setfillstyle(SOLID_FILL, BLACK);
                    bar(x, y, x + CELL_SIZE, y + CELL_SIZE);
                    setcolor(YELLOW);
                    fillellipse(x + CELL_SIZE / 2, y + CELL_SIZE / 2, 8, 8);
                    break;
                case RED_ZONE:  
                    setfillstyle(SOLID_FILL, RED);
                    bar(x, y, x + CELL_SIZE, y + CELL_SIZE);
                    setcolor(BLACK);
                    rectangle(x, y, x + CELL_SIZE, y + CELL_SIZE);
                    break;
                case EMPTY:
                    setfillstyle(SOLID_FILL, BLACK);
                    bar(x, y, x + CELL_SIZE, y + CELL_SIZE);
                    break;
            }
        }
    }

    setfillstyle(SOLID_FILL, GREEN);
    fillellipse(playerY * CELL_SIZE + CELL_SIZE / 2, playerX * CELL_SIZE + CELL_SIZE / 2, 10, 10);

    for (int i = 0; i < ghostCount; ++i) {
        setfillstyle(SOLID_FILL, RED);
        fillellipse(ghostsY[i] * CELL_SIZE + CELL_SIZE / 2, ghostsX[i] * CELL_SIZE + CELL_SIZE / 2, 10, 10);
    }
      drawTimer();  
}

//maze er niche score and level no dekha jabe
void drawScoreAndLevel(int elapsedTime) {
    char scoreText[20];
    char levelText[20];
    char time[20];
    sprintf(scoreText, "SCORE: %d", score);
    sprintf(time, "Time: %d s", elapsedTime);
    sprintf(levelText, "LEVEL: %d", level);

    setcolor(WHITE); 
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 2.5);

    outtextxy(50, 600, scoreText); 
    outtextxy(200,600,time); 
    outtextxy(350, 600, levelText);
}

//min-heap
void heapifyUp(vector<Node*>& heap, int index) {
    while (index > 0) {
        int parentIdx = (index - 1) / 2;
        if (heap[index]->f() >= heap[parentIdx]->f()) 
            break;

        Node* temp = heap[index];
        heap[index] = heap[parentIdx];
        heap[parentIdx] = temp;

        index = parentIdx;
    }
}

void heapifyDown(vector<Node*>& heap, int index) {
    int leftChildIdx = 2 * index + 1;
    int rightChildIdx = 2 * index + 2;
    int smallestIdx = index;

    if (leftChildIdx < heap.size() && heap[leftChildIdx]->f() < heap[smallestIdx]->f()) {
        smallestIdx = leftChildIdx;
    }

    if (rightChildIdx < heap.size() && heap[rightChildIdx]->f() < heap[smallestIdx]->f()) {
        smallestIdx = rightChildIdx;
    }

    if (smallestIdx != index) {
        Node* temp = heap[index];
        heap[index] = heap[smallestIdx];
        heap[smallestIdx] = temp;
        heapifyDown(heap, smallestIdx);
    }
}

void pushToMinHeap(vector<Node*>& heap, Node* node) {
    heap.push_back(node);
    heapifyUp(heap, heap.size() - 1);
}

Node* popFromMinHeap(vector<Node*>& heap) {
    if (heap.empty()) 
        return nullptr;

    Node* top = heap[0];
    heap[0] = heap.back();
    heap.pop_back();
    heapifyDown(heap, 0);
    return top;
}

bool isHeapEmpty(const vector<Node*>& heap) {
    return heap.empty();
}

bool isValidMove(int x, int y) {
    if (x < 0 || x >= ROWS) {
        return false; 
    } 
    else if (y < 0 || y >= COLS) {
        return false; 
    } 
    else if (maze[x][y] == WALL) {
        return false;
    } 
    else if (maze[x][y] == OBSTACLE) {
        return false;
    } 
    else {
        return true;
    }
}

int manhattanDistance(int x1, int y1, int x2, int y2) {
    int dx, dy;
    
    if (x1 > x2) {
        dx = x1 - x2;
    } 
    else {
        dx = x2 - x1;
    }

    if (y1 > y2) {
        dy = y1 - y2;
    } 
    else {
        dy = y2 - y1;
    }
    int sum = dx + dy;
    return sum;
}

void reverseVector(std::vector<Node>& path) {
    int n = path.size();
    for (int i = 0; i < n / 2; ++i) {
        Node temp = path[i];
        path[i] = path[n - i - 1];
        path[n - i - 1] = temp;
    }

}
vector<Node> aStar(int startX, int startY, int goalX, int goalY) {
    vector<Node*> openSet;
    unordered_map<int, unordered_map<int, bool>> closedSet;
    unordered_map<int, unordered_map<int, Node*>> allNodes;

    Node* startNode = new Node{startX, startY, 0, manhattanDistance(startX, startY, goalX, goalY), nullptr};
    pushToMinHeap(openSet, startNode);
    allNodes[startX][startY] = startNode;

    vector<Node> path;

    while (!isHeapEmpty(openSet)) {
        Node* currentNode = popFromMinHeap(openSet);

        // Goal reached
        if (currentNode->x == goalX && currentNode->y == goalY) {
            Node* node = currentNode;
            while (node != nullptr) {
                path.push_back(*node);
                node = node->parent;
            }
            reverseVector(path);
            break;
        }

        closedSet[currentNode->x][currentNode->y] = true;

        // Explore neighbors
        for (int i = 0; i < 4; ++i) {
            int newX = currentNode->x + DX[i];
            int newY = currentNode->y + DY[i];

            if (isValidMove(newX, newY) && !closedSet[newX][newY]) {
                int newG = currentNode->g + 1;
                int newH = manhattanDistance(newX, newY, goalX, goalY);

                if (allNodes[newX][newY]) {
                    Node* neighbor = allNodes[newX][newY];
                    if (newG < neighbor->g) {
                        neighbor->g = newG;
                        neighbor->parent = currentNode;
                        pushToMinHeap(openSet, neighbor);
                    }
                } 
                else {
                    Node* newNode = new Node{newX, newY, newG, newH, currentNode};
                    pushToMinHeap(openSet, newNode);
                    allNodes[newX][newY] = newNode;
                }
            }
        }
    }

    for (auto& row : allNodes) {
        for (auto& pair : row.second) {
            delete pair.second;
        }
    }

    return path;
}

void setGhostCount(const string& level) {
    if (level == "easy_maze.txt") {
        ghostCount = 2;
    } 
    else if (level == "medium_maze.txt") {
        ghostCount = 4;
    } 
    else if (level == "hard_maze.txt") {
        ghostCount = 6;
    } 
    else {
        ghostCount = 3;
    }
}

void initializeGhostPositions() {
    for (int i = 0; i < ghostCount; ++i) 
    {
        ghostsX[i] = ROWS / 2 + i % 2; 
        ghostsY[i] = COLS / 2 + (i / 2); 
        maze[ghostsX[i]][ghostsY[i]] = EMPTY; 
    }
}

void moveGhosts() {
    for (int i = 0; i < ghostCount; ++i) {
        int targetX = playerX;
        int targetY = playerY;

        //pathfinding -> each ghost
        if (i == 0 || i == 1) {
            if (i == 0) {
                targetX = playerX - 1;
                targetY = playerY;
            } 
            else if (i == 1) {
                targetX = playerX + 1;
                targetY = playerY;
            }
        } 
        else {
            if (i == 2) {
                targetX = playerX - 1;
                targetY = playerY - 1;
            } 
            else if (i == 3) {
                targetX = playerX + 1;
                targetY = playerY + 1;
            }
        }
        if (rand() % 2 == 0) {
            targetX += (rand() % 2) * 2 - 1; 
        }
        if (rand() % 2 == 0) {
            targetY += (rand() % 2) * 2 - 1; 
        }
        targetX = max(0, min(ROWS - 1, targetX));
        targetY = max(0, min(COLS - 1, targetY));

        bool occupied = false;
        for (int j = 0; j < ghostCount; ++j) {
            if (i != j && ghostsX[i] == ghostsX[j] && ghostsY[i] == ghostsY[j]) {
                occupied = true;
                break;
            }
        }

        if (occupied) {
            bool moved = false;
            for (int attempt = 0; attempt < 4; ++attempt) {
                int randomDir = rand() % 4;
                int newX = ghostsX[i] + DX[randomDir];
                int newY = ghostsY[i] + DY[randomDir];

                if (isValidMove(newX, newY) && !isGhostAtPosition(newX, newY)) {
                    ghostsX[i] = newX;
                    ghostsY[i] = newY;
                    moved = true;
                    break;
                }
            }

            if (!moved) {
                cout << "Ghost " << i << " is stuck!" << endl;
            }
        } 

        else {
            vector<Node> path = aStar(ghostsX[i], ghostsY[i], targetX, targetY);

            if (!path.empty() && path.size() > 1) {
                Node nextNode = path[1];
                ghostsX[i] = nextNode.x;
                ghostsY[i] = nextNode.y;
            } 
            else {
                
                bool moved = false;
                for (int attempt = 0; attempt < 4; ++attempt) {
                    int randomDir = rand() % 4;
                    int newX = ghostsX[i] + DX[randomDir];
                    int newY = ghostsY[i] + DY[randomDir];

                    if (isValidMove(newX, newY) && !isGhostAtPosition(newX, newY)) {
                        ghostsX[i] = newX;
                        ghostsY[i] = newY;
                        moved = true;
                        break;
                    }
                }

                if (!moved) {
                    cout << "Ghost " << i << " is stuck!" << endl;
                }
            }
        }
    }
}

bool isGhostAtPosition(int x, int y) {
    for (int i = 0; i < ghostCount; ++i) {
        if (ghostsX[i] == x && ghostsY[i] == y) {
            return true;
        }
    }
    return false;
}

int ghostCooldown;

//player movement
void movePlayer(char direction) {
    int newX = playerX;
    int newY = playerY;
    switch (direction) {
        case 'w':
            newX--; 
            break;
        case 's': 
            newX++; 
            break;
        case 'a': 
            newY--; 
            break;
        case 'd': 
            newY++; 
            break;
        default: 
            return;
    }

    if (newX >= 0 && newX < ROWS && newY >= 0 && newY < COLS && maze[newX][newY] != WALL && maze[newX][newY] != OBSTACLE) {
        playerX = newX;
        playerY = newY;
        if (maze[newX][newY] == DOT)
        {
            remainingDots--;
            maze[newX][newY] = EMPTY;
            score += 5;
        } 
        else if (maze[newX][newY] == POWER_UP)
        {
            score += 10;
            maze[newX][newY] = EMPTY;
            ghostCooldown = 5; 
            invincible = true;
            cout << "Power-Up collected! Ghosts can't kill you for 5 moves!" << endl;
    
        }
        if (ghostCooldown > 0) {
             ghostCooldown--;
            if (ghostCooldown == 0) {
                invincible = false;
                cout << "Power-Up expired! You can now be caught by ghosts." << endl;
            }
        }
    }    
}

void checkCollision() {
    for (int i = 0; i < ghostCount; ++i) {
        if (playerX == ghostsX[i] && playerY == ghostsY[i]) {
            if (!invincible) {
                gameOver = true;
                cout << "Game Over! You were caught by a ghost!" << endl;
            } else {
                cout << "You're invincible! Ghosts can't kill you right now!" << endl;
            }
            return;
        }
    }

    if (remainingDots == 0) {
        gameOver = true;
        cout << "Congratulations! You collected all the dots!" << endl;
        return;
    }

    for (int i = 0; i < redZoneCount; ++i) {
        if (playerX == redZoneX[i] && playerY == redZoneY[i]) {
            gameOver = true;
            cout << "Game Over! You entered a restricted Red Zone!" << endl;
            return;
        }
    }
}

void drawTimer() {
    if (ghostCooldown > 0) { 
        setcolor(WHITE);
        setbkcolor(BLACK);
        char timerText[30];
        sprintf(timerText, "Invincible: %d moves left", ghostCooldown);
        outtextxy(50, SCREEN_HEIGHT - 30, timerText);
    }
}

void gameOverScreen(int score) {
    remove("savegame.txt");
    cleardevice();

    settextstyle(DEFAULT_FONT, HORIZ_DIR, 5);
    setcolor(RED);
    outtextxy(200, 100, (char*)"Game Over!");

    // Display the final score
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 3);
    setcolor(WHITE);
    char scoreText[50];
    sprintf(scoreText, "Your Score: %d", score);
    outtextxy(200, 200, scoreText);

    drawButton(200, 300, 300, 50, "1. Main Menu");
    drawButton(200, 380, 300, 50, "2. Exit");

    while (true) {
        if (ismouseclick(WM_LBUTTONDOWN)) {
            int clickX, clickY;
            getmouseclick(WM_LBUTTONDOWN, clickX, clickY);
            clearmouseclick(WM_LBUTTONDOWN);  
            if (isButtonClicked(200, 300, 300, 50, clickX, clickY)) {
                cleardevice();
                redraw = true;  
                mainMenu();  
                return;      
            }

            // "Exit" -> close
            else if (isButtonClicked(200, 380, 300, 50, clickX, clickY)) {
                closegraph();  
                exit(0);       
            }
        }

        delay(50);  
    }
}

vector<int> readScoresFromFile(const string& filename) {
    vector<int> scores;
    ifstream file(filename);

    if (file.is_open()) {
        int score;
        while (file >> score) {
            scores.push_back(score);
        }
        file.close();
    }

    return scores;
}

void writeScoresToFile(const string& filename, const vector<int>& scores) {
    ofstream file(filename);

    if (file.is_open()) {
        for (int score : scores) {
            file << score << endl;
        }
        file.close();
    }
}

void merge(vector<int>& scores, int l, int mid, int h) {
    int n1 = mid - l + 1;
    int n2 = h - mid;

    vector<int> L(n1), R(n2);

    for (int i = 0; i < n1; i++)
        L[i] = scores[l + i];
    for (int j = 0; j < n2; j++)
        R[j] = scores[mid + 1 + j];

    int i = 0, j = 0, k = l;

    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            scores[k] = L[i];
            i++;
        } else {
            scores[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        scores[k] = L[i];
        i++;
        k++;
    }

    while (j < n2) {
        scores[k] = R[j];
        j++;
        k++;
    }
}

void mergeSort(vector<int>& scores, int l, int h) {
    if (l < h) {
        int mid = l + (h - l) / 2;

        mergeSort(scores, l, mid);
        mergeSort(scores, mid + 1, h);

        merge(scores, l, mid, h);
    }
}

void updateScores(const string& level, int newScore) {
    vector<int> scores;
    string filename;
    if (level == "easy") {
        filename = "easy_scores.txt";
    } 
    else if (level == "medium") {
        filename = "medium_scores.txt";
    } 
    else if (level == "hard") {
        filename = "hard_scores.txt";
    }
    else {
        cout << "Invalid level!" << endl;
        return;
    }
    // Read scores
    ifstream infile(filename);
    if (!infile) { 
        cout << "Creating file: " << filename << endl;
        ofstream createFile(filename);  // Create file if it doesn't exist
        
        if (!createFile) {
            cerr << "Error: Unable to create file: " << filename << endl;
            return;
        }
        createFile.close();
    } 
    else {
        int score;
        while (infile >> score) {
            scores.push_back(score);
        }

        infile.close();
    }

    scores.push_back(newScore);

    // Sort the scores in descending order
    mergeSort(scores, 0, scores.size() - 1);
    int l = 0;
    int r = scores.size() - 1;

    while (l < r) {
        int t = scores[l];
        scores[l] = scores[r];
        scores[r] = t;

        l++;
        r--;
    }
    // top 100 scores
    if (scores.size() > 100) {
        scores.resize(100);
    }

    // Write the top 100 scores
    ofstream outfile(filename);
    for (int s : scores) {
        outfile << s << endl;
    }
    outfile.close();
}

void drawQuitMenu() {
    int x = 400, y = 400, width = 400, height = 200;

    setfillstyle(SOLID_FILL, LIGHTGRAY);
    bar(x, y, x + width, y + height);
    setcolor(BLACK);
    rectangle(x, y, x + width, y + height);

    setcolor(WHITE);
    settextstyle(3, HORIZ_DIR, 3);
    outtextxy(x + 50, y + 30, (char*)"Do you want to quit?");

    setfillstyle(SOLID_FILL, RED);
    bar(x + 50, y + 100, x + 150, y + 150);
    setcolor(WHITE);
    outtextxy(x + 80, y + 115, (char*)"Yes");

    setfillstyle(SOLID_FILL, GREEN);
    bar(x + 250, y + 100, x + 350, y + 150);
    setcolor(WHITE);
    outtextxy(x + 280, y + 115, (char*)"No");
}

void clearQuitMenu() {
    int x = 400, y = 400, width = 400, height = 200;
    setfillstyle(SOLID_FILL, BLACK);  
    bar(x, y, x + width, y + height);
}

bool askQuitConfirmation() {
    drawQuitMenu();
    
    while (true) {
        if (kbhit()) {
            char key = getch();
            if (key == 'y' || key == 'Y') {
                return true;  
            }
            if (key == 'n' || key == 'N') {
                return false;
            }
        }

        if (ismouseclick(WM_LBUTTONDOWN)) {
            int mx, my;
            getmouseclick(WM_LBUTTONDOWN, mx, my);
            clearmouseclick(WM_LBUTTONDOWN);
            if (mx >= 450 && mx <= 550 && my >= 500 && my <= 550) {
                mainMenu();
                return true;
            }
            if (mx >= 650 && mx <= 750 && my >= 500 && my <= 550) {
                delay(300); 
                clearQuitMenu();
                return false; 
            }
        }
    }
}

string getLevelFromFile(const string& levelFile) {

    if (levelFile == "easy_maze.txt") {
        return "easy";
    }
    if (levelFile == "medium_maze.txt"){
        return "medium";
    }
    if (levelFile == "hard_maze.txt") {
        return "hard";
    }

    return "unknown"; 
}

void startGame(const string& levelFile){
    playerX = 1;
    playerY = 1;
    gameOver = false;
    score = 0; 

    setGhostCount(levelFile);
    loadMazeFromFile(levelFile);
    initializeGhostPositions();
    startTime = time(0);

    while (!gameOver) {
        elapsedTime = time(0) - startTime;
        drawMaze();
        drawScoreAndLevel(elapsedTime);

        if (ghostMoveCounter % ghostMoveFrequency == 0){
          moveGhosts(); 
        }

        ghostMoveCounter++;
        checkCollision();

        if (gameOver) 
            break;

        if (kbhit()) {
            char move = getch();
            if (move == 27) { 
                    if (askQuitConfirmation()) {
                        closegraph();
                        return; 
                    }
                } 
            movePlayer(move);
        }
        delay(200);
    }

    string level = getLevelFromFile(levelFile);
    int finalScore;

    if (level != "unknown") {
        finalScore = (score / max(1, elapsedTime)) * 100;
        updateScores(level, finalScore);
    }
    gameOverScreen(finalScore);
}

int main(){
    srand(time(0));
    initwindow(1200,1200);

    mainMenu();
    string mazeFile = selectLevel();
    setGhostCount(mazeFile);
    loadMazeFromFile(mazeFile);
    initializeGhostPositions();
    startTime = time(0);

    while (!gameOver) {
        elapsedTime = time(0) - startTime;
        drawMaze();
        drawScoreAndLevel(elapsedTime);

        if (ghostMoveCounter % ghostMoveFrequency == 0){
          moveGhosts(); 
        }

        ghostMoveCounter++;
        checkCollision();

        if (gameOver) 
            break;

        if (kbhit()) {
            char move = getch();
            if (move == 27) { 
                if (askQuitConfirmation()) {
                    closegraph();
                    return 0; 
                }
            } 
            movePlayer(move);
        }
        delay(200);
    }

    string level = getLevelFromFile(mazeFile);
    int finalScore;

    if (level != "unknown") {
        finalScore = (score / max(1, elapsedTime)) * 100;
        updateScores(level, finalScore);
    }

    gameOverScreen(finalScore);
    getch();
    closegraph();
    return 0;
}