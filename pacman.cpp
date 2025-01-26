#include<graphics.h>
#include<iostream>
#include<vector>
#include<unordered_map>
#include<ctime>
#include<chrono>
#include<fstream>
#include<string>
#include<queue>
using namespace std;

#define ROWS 15
#define COLS 15
#define CELL_SIZE 30

#define WALL 1
#define OBSTACLE 2
#define DOT 3
#define POWER_UP 4
#define EMPTY 0

int maze[ROWS][COLS];

int playerX = 1;
int playerY = 1;

int ghostCount;
int ghostsX[10];
int ghostsY[10];
int ghostMoveCounter = 0;
const int ghostMoveFrequency = 5;

chrono::time_point<chrono::steady_clock> powerUpCollectedTime;  

//up,down,left,right
int DX[] = {-1, 1, 0, 0};
int DY[] = {0, 0, -1, 1}; 
int score = 0;
int level;
bool gameOver = false;
bool invincible = false;

struct Node {
        int x;
        int y;
        int g;
        int h; // g = cost from start, h = heuristic
        Node* parent;

        int f() const {
             return g + h; 
        }

        bool operator<(const Node& other) const {
            if (f() > other.f()) {
                return true; // Current node has a higher f(), so it is "less than" the other node for priority queue purposes.
            } 
            else {
                return false; 
            }
        }
};

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
                return "easy_maze.txt"; 
            }
            else if (x >= 350 && x <= 550 && y >= 100 && y <= 200)
            {
                 level = 2;
                return "medium_maze.txt"; 
            }
            else if (x >= 600 && x <= 800 && y >= 100 && y <= 200) {
                level = 3;
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
            }
            else if (line[col] == 'E') {
                maze[row][col] = EMPTY;
            }
        }
        row++;
    }
    file.close();
    maze[playerX][playerY] = EMPTY; // Ensure player starts at a valid position
        
    for (int i = 0; i < ghostCount; ++i) {
            maze[ghostsX[i]][ghostsY[i]] = EMPTY; // Ensure ghosts start at valid positions
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
                case EMPTY:
                    setfillstyle(SOLID_FILL, BLACK);
                    bar(x, y, x + CELL_SIZE, y + CELL_SIZE);
                    break;
            }
        }
    }
    // draw player
    setfillstyle(SOLID_FILL, GREEN);
    fillellipse(playerY * CELL_SIZE + CELL_SIZE / 2, playerX * CELL_SIZE + CELL_SIZE / 2, 10, 10);

    // draw ghosts
    for (int i = 0; i < ghostCount; ++i) {
        setfillstyle(SOLID_FILL, RED);
        fillellipse(ghostsY[i] * CELL_SIZE + CELL_SIZE / 2, ghostsX[i] * CELL_SIZE + CELL_SIZE / 2, 10, 10);
    }
        
}

//maze er niche score and level no dekha jabe
void drawScoreAndLevel() {
    char scoreText[20], levelText[20];

    sprintf(scoreText, "SCORE: %d", score);
    sprintf(levelText, "LEVEL: %d", level);

    setcolor(WHITE);  //text color
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 2.5);

    outtextxy(50, 600, scoreText);  
    outtextxy(300, 600, levelText);
}

bool isValidMove(int x, int y) {
    if (x < 0 || x >= ROWS) {
        return false; // Out of bounds in x
    } 
    else if (y < 0 || y >= COLS) {
        return false; // Out of bounds in the y direction
    } 
    else if (maze[x][y] == WALL) {
        return false;
    } 
    else if (maze[x][y] == OBSTACLE) {
        return false;
    } 
    else {
        return true; // Valid move
    }
}

int manhattanDistance(int x1, int y1, int x2, int y2) {
    int dx, dy;
    //absolute difference for x-coordinates
    if (x1 > x2) {
        dx = x1 - x2;
    } 
    else {
        dx = x2 - x1;
    }

    //absolute difference for y-coordinates
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
    priority_queue<Node> openSet;
    unordered_map<int, unordered_map<int, bool>> closedSet;

    Node startNode = {startX, startY, 0, manhattanDistance(startX, startY, goalX, goalY), nullptr};
    openSet.push(startNode);

    vector<Node> path;

    while (!openSet.empty()) {
        Node currentNode = openSet.top();
        openSet.pop();

        if (currentNode.x == goalX && currentNode.y == goalY) {
            Node* node = &currentNode;
            while (node != nullptr) {
                path.push_back(*node);
                node = node->parent;
            }
            reverseVector(path);
                // Debugging: Print the path
            cout << "Path from (" << startX << "," << startY << ") to (" << goalX << "," << goalY << "): ";
            for (const auto& step : path) {
                cout << "(" << step.x << "," << step.y << ") ";
            }
            cout << endl;
            return path;
        }

        closedSet[currentNode.x][currentNode.y] = true;

        for (int i = 0; i < 4; ++i) {
            int newX = currentNode.x + DX[i];
            int newY = currentNode.y + DY[i];

            if (isValidMove(newX, newY) && !closedSet[newX][newY]) {
                int newG = currentNode.g + 1;
                int newH = manhattanDistance(newX, newY, goalX, goalY);
                Node neighbor = {newX, newY, newG, newH, new Node(currentNode)};
                openSet.push(neighbor);
            }
        }
    }

    return path; // Return empty path if no path is found
}

//ghost count based on difficulty
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
        ghostCount = 3; // Default value
    }
}

void initializeGhostPositions() {
    for (int i = 0; i < ghostCount; ++i) 
    {
        ghostsX[i] = ROWS / 2 + i % 2; // Stagger initial positions
        ghostsY[i] = COLS / 2 + (i / 2); // Distribute evenly
        maze[ghostsX[i]][ghostsY[i]] = EMPTY; // Ensure ghosts start on valid cells
    }
}
void moveGhosts() {
    for (int i = 0; i < ghostCount; ++i) {
        int targetX = playerX;
        int targetY = playerY;

        // Different pathfinding strategy for each ghost
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

        // Ensure the target is within maze bounds
        targetX = max(0, min(ROWS - 1, targetX));
        targetY = max(0, min(COLS - 1, targetY));

        // Calculate the path to the target using A* pathfinding
        vector<Node> path = aStar(ghostsX[i], ghostsY[i], targetX, targetY);

        if (!path.empty() && path.size() > 1) {
            // Move the ghost to the next position on the path
            Node nextNode = path[1];
            ghostsX[i] = nextNode.x;
            ghostsY[i] = nextNode.y;
        } 
        else {
            // If no path is found, move randomly
            int randomDir = rand() % 4;
            int newX = ghostsX[i] + DX[randomDir];
            int newY = ghostsY[i] + DY[randomDir];

            if (isValidMove(newX, newY)) {
                ghostsX[i] = newX;
                ghostsY[i] = newY;
            }
        }
    }
}

    //check collision with ghosts
void checkCollision() {
    for (int i = 0; i < ghostCount; ++i) {
        if (playerX == ghostsX[i] && playerY == ghostsY[i]) {
            gameOver = true;
            cout << "Game Over! You were caught by a ghost!" << endl;
        }
    }
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

    // Randomly place power-ups in the maze
    while (powerUpsPlaced < powerUpsToPlace) {
        int x = rand() % ROWS;
        int y = rand() % COLS;

        // Check if the position is valid (not a wall, obstacle, or already occupied)
        if (maze[x][y] == EMPTY) {
            maze[x][y] = POWER_UP;  // Place power-up here
            powerUpsPlaced++;
        }
    }
}

void drawRemainingTime(int remainingTime) {
    char timeText[20];
    sprintf(timeText, "TIME LEFT: %d", remainingTime);  // Displaying countdown
    setcolor(WHITE);  // Text color
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 2.5);
    outtextxy(50, 650, timeText);  // Position the text lower on the screen
}

// Check invincibility timer and update time remaining
void checkInvincibility() {
    if (invincible) {
        auto now = chrono::steady_clock::now();
        auto duration = chrono::duration_cast<chrono::seconds>(now - powerUpCollectedTime).count();
        
        int remainingTime = 5 - duration;  // 5 seconds - elapsed time
        if (remainingTime <= 0) {
            invincible = false;
            remainingTime = 0;  // Ensure the time doesn't go negative
        }

        // Display remaining time (countdown)
        drawRemainingTime(remainingTime);
    }
}

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
            maze[newX][newY] = EMPTY;
            score += 10;
        } 
        else if (maze[newX][newY] == POWER_UP)
        {
            maze[newX][newY] = EMPTY;
            invincible = true;
        }
    }    
}

int main(){
    srand(time(0));
    initwindow(900,900);
    string mazeFile = selectLevel();
    setGhostCount(mazeFile);
    placePowerUps();
    loadMazeFromFile(mazeFile);
    initializeGhostPositions();
    while (!gameOver) {
        drawMaze();
        drawScoreAndLevel();
        checkInvincibility();

        if (ghostMoveCounter % ghostMoveFrequency == 0){
          moveGhosts(); 
        }

        ghostMoveCounter++;
        checkCollision();

        if (gameOver) break;
        if (kbhit()) {
            char move = getch();
            if (move == 'q') break; 
            movePlayer(move);
        }
        delay(200);
    }

}