#include<graphics.h>
#include<iostream>
#include<vector>
#include<unordered_map>
#include<ctime>
#include<fstream>
#include<string>

using namespace std;

#define ROWS 15
#define COLS 22
#define CELL_SIZE 25

#define WALL 1
#define OBSTACLE 2
#define DOT 3
#define POWER_UP 4
#define RED_ZONE 5
#define EMPTY 0

int maze[ROWS][COLS];

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
int redZoneX[MAX_RED_ZONES], redZoneY[MAX_RED_ZONES];
const int SCREEN_HEIGHT = 600; 

std::vector<int> readScoresFromFile(const std::string& filename);
void drawTimer();
bool isGhostAtPosition(int x, int y);
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
    return (clickX >= x && clickX <= x + width && clickY >= y && clickY <= y + height);
}

void displayScores(const string& filename) {
    vector<int> scores = readScoresFromFile(filename);

    // If no scores exist, show only one default score
    if (scores.empty()) {
        cout << "No scores available.\n";
        return;
    }

    cleardevice();
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 3);
    outtextxy(100, 50, (char*)"Top Scores:");

    // Display up to 10 scores
    int numScoresToShow = min(10, (int)scores.size());

    for (int i = 0; i < numScoresToShow; i++) {
        char scoreText[50];
        sprintf(scoreText, "%d: %d", i + 1, scores[i]);
        outtextxy(100, 100 + i * 30, scoreText);
    }

    getch();
}


void displayRules() {
    cleardevice();
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 3);
    outtextxy(100, 50, (char*)"Game Rules:");
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);
    outtextxy(100, 100, (char*)"1. Navigate the maze.");
    outtextxy(100, 140, (char*)"2. Avoid obstacles and ghosts.");
    outtextxy(100, 180, (char*)"3. Collect points to win.");
    outtextxy(100, 220, (char*)"4. Use arrow keys for movement.");
    getch();
}

void mainMenu() {
    bool redraw = true;  // Flag to track when to redraw the screen

    while (true) {
        if (redraw) {
            cleardevice(); 
            settextstyle(DEFAULT_FONT, HORIZ_DIR, 3);
            outtextxy(100, 50, (char*)"Main Menu");
            drawButton(100, 120, 300, 50, "1. Level Selection");
            drawButton(100, 200, 300, 50, "2. Scores");
            drawButton(100, 280, 300, 50, "3. Rules");
            drawButton(100, 360, 300, 50, "4. Exit");
            redraw = false;  // Reset the flag after drawing
        }

        if (ismouseclick(WM_LBUTTONDOWN)) {
            int clickX, clickY;
            getmouseclick(WM_LBUTTONDOWN, clickX, clickY);

            if (isButtonClicked(100, 120, 300, 50, clickX, clickY)) {
                return;  
            } 
            else if (isButtonClicked(100, 200, 300, 50, clickX, clickY)) {
                displayScores("score.txt");
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
                redZoneCount = 2;
                return "easy_maze.txt"; 
            }
            else if (x >= 350 && x <= 550 && y >= 100 && y <= 200)
            {
                 level = 2;
                 redZoneCount = 4;
                return "medium_maze.txt"; 
            }
            else if (x >= 600 && x <= 800 && y >= 100 && y <= 200) {
                level = 3;
                redZoneCount = 6;
                return "hard_maze.txt"; 
            }
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

    string line;
    int row = 0;
    while (getline(file, line) && row < ROWS) {
        for (int col = 0; col < line.size() && col < COLS; ++col)
        {
            if (line[col] == '#') {
                maze[row][col] = WALL;
            }
            else if (line[col] == 'O') {
                maze[row][col] = OBSTACLE;
            }
            else if (line[col] == 'P') {
                maze[row][col] = POWER_UP;
            }
            else if (line[col] == '.') {
                maze[row][col] = DOT;
                remainingDots++;
            }
            else if (line[col] == 'E') {
                maze[row][col] = EMPTY;
            }
        }
        row++;
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
void drawScoreAndLevel() {
    char scoreText[20], levelText[20];

    sprintf(scoreText, "SCORE: %d", score);
    sprintf(levelText, "LEVEL: %d", level);

    setcolor(WHITE); 
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 2.5);

    outtextxy(50, 600, scoreText);  
    outtextxy(300, 600, levelText);
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
                } else {
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

void placePowerUps() {
    int powerUpsToPlace = 0;
    if (level == 1) {  
        powerUpsToPlace = 2;
    } 
    else if (level == 2) {  
        powerUpsToPlace = 4;
    } 
    else if (level == 3) {  
        powerUpsToPlace = 6;
    }

    int powerUpsPlaced = 0;

    while (powerUpsPlaced < powerUpsToPlace) {
        int x = rand() % ROWS;
        int y = rand() % COLS;

        if (maze[x][y] == EMPTY) {
            maze[x][y] = POWER_UP;  
            powerUpsPlaced++;
        }
    }
}

int ghostCooldown ;
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
            score += 10;
        } 
        else if (maze[newX][newY] == POWER_UP)
        {
            score += 30;
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
    // যদি প্লেয়ার Ghost এর অবস্থানে যায়, গেম ওভার হবে
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

    //যদি সকল dot collect হয়ে যায়, গেম ওভার হবে
    if (remainingDots == 0) {
        gameOver = true;
        cout << "Congratulations! You collected all the dots!" << endl;
        return;
    }

    //যদি প্লেয়ার Red Zone এ ঢুকে, গেম ওভার হবে
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

            if (isButtonClicked(200, 300, 300, 50, clickX, clickY)) {
                cleardevice();
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

void updateScores(const string& filename, int newScore) {
    vector<int> scores;

    // Read scores
    ifstream infile(filename);
    int score;
    while (infile >> score) {
        scores.push_back(score);
    }
    infile.close();

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

int main(){
    srand(time(0));
    initwindow(1200,1200);
    mainMenu();
    string mazeFile = selectLevel();
    setGhostCount(mazeFile);
    loadMazeFromFile(mazeFile);
    initializeGhostPositions();
    while (!gameOver) {
        drawMaze();
        drawScoreAndLevel();

        if (ghostMoveCounter % ghostMoveFrequency == 0){
          moveGhosts(); 
        }

        ghostMoveCounter++;
        checkCollision();

        if (gameOver) break;
        if (kbhit()) {
            char move = getch();
            if (move == 'q') 
                break; 
            movePlayer(move);
        }
        delay(200);
    }
    updateScores("score.txt", score);
    gameOverScreen(score);
    getch();
    closegraph();
    return 0;
}