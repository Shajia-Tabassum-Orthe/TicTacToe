// File: tictactoe.cpp
// Compile (example, on Windows with MinGW + freeglut):
// g++ tictactoe.cpp -o tictactoe.exe -lfreeglut -lopengl32 -lglu32

/*echo "# TicTacToe" >> README.md
git init
git add README.md
git commit -m "first commit"
git branch -M main
git remote add origin https://github.com/Shajia-Tabassum-Orthe/TicTacToe.git
git push -u origin main*/

#include <GL/freeglut.h>
#include <iostream>
#include <vector>
#include <ctime>
#include <cmath>
#include <limits>
#include <algorithm>

using namespace std;

// ---------- Window ----------
int WIN_W = 700;
int WIN_H = 700;
const int MARGIN = 40;

// ---------- Menus & Modes ----------
enum MenuStep { MODE_SELECT, SIZE_SELECT };
enum Mode { MODE_NONE, HUMAN_VS_COMPUTER, HUMAN_VS_HUMAN };
MenuStep menuStep = MODE_SELECT;
Mode selectedMode = MODE_NONE;
int selectedSize = 0; // 3 or 4

enum AppState { STATE_MENU, STATE_PLAY };
AppState appState = STATE_MENU;

// ---------- TicTacToe Class ----------
class TicTacToe {
private:
    int n;
    int** board;         // 0 empty, 1 = X (player1), 2 = O (player2 or computer)
    float** anim;        // animation scale for each cell (0..1)
    int currentPlayer;   // 1 or 2; X always starts
    bool gameOver;
    int winner;          // 0 draw/none, 1 X, 2 O
    int scoreX, scoreO;

public:
    TicTacToe(int size) : n(size) {
        board = new int*[n];
        anim = new float*[n];
        for (int i=0;i<n;i++){
            board[i] = new int[n];
            anim[i] = new float[n];
        }
        resetBoard();
        scoreX = scoreO = 0;
    }

    ~TicTacToe(){
        for (int i=0;i<n;i++){
            delete[] board[i];
            delete[] anim[i];
        }
        delete[] board;
        delete[] anim;
    }

    void resetBoard(){
        for (int i=0;i<n;i++)
            for (int j=0;j<n;j++){
                board[i][j] = 0;
                anim[i][j] = 0.0f;
            }
        currentPlayer = 1;
        gameOver = false;
        winner = 0;
    }

    int getN() const { return n; }
    int getCurrentPlayer() const { return currentPlayer; }
    bool isGameOver() const { return gameOver; }
    int getWinner() const { return winner; }
    int getScoreX() const { return scoreX; }
    int getScoreO() const { return scoreO; }

    // ---------- drawing helpers ----------
    void drawText(float x, float y, const char* text, float r=0, float g=0, float b=0) {
        glColor3f(r,g,b);
        glRasterPos2f(x,y);
        for (const char* c = text; *c; ++c) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    void drawGrid(float startX, float startY, float cellSize) {
        // white cells
        for(int i=0;i<n;i++){
            for(int j=0;j<n;j++){
                glColor3f(1,1,1);
                glBegin(GL_QUADS);
                    glVertex2f(startX + j*cellSize, startY + i*cellSize);
                    glVertex2f(startX + (j+1)*cellSize, startY + i*cellSize);
                    glVertex2f(startX + (j+1)*cellSize, startY + (i+1)*cellSize);
                    glVertex2f(startX + j*cellSize, startY + (i+1)*cellSize);
                glEnd();
            }
        }
        // grid lines
        glColor3f(0.6f,0.6f,0.6f);
        glLineWidth(3);
        float total = cellSize * n;
        for(int i=1;i<n;i++){
            glBegin(GL_LINES);
                glVertex2f(startX + i*cellSize, startY);
                glVertex2f(startX + i*cellSize, startY + total);
            glEnd();
            glBegin(GL_LINES);
                glVertex2f(startX, startY + i*cellSize);
                glVertex2f(startX + total, startY + i*cellSize);
            glEnd();
        }
        // border
        glColor3f(0.9f,0.65f,0.18f);
        glLineWidth(5);
        glBegin(GL_LINE_LOOP);
            glVertex2f(startX, startY);
            glVertex2f(startX + total, startY);
            glVertex2f(startX + total, startY + total);
            glVertex2f(startX, startY + total);
        glEnd();
        glLineWidth(1);
    }

    void drawMarks(float startX, float startY, float cellSize) {
        for(int i=0;i<n;i++){
            for(int j=0;j<n;j++){
                float x = startX + j*cellSize;
                float y = startY + i*cellSize;
                float scale = anim[i][j];
                if (board[i][j] == 1) {
                    // X - purple
                    float margin = 12 + (cellSize/2 - 12)*(1 - scale);
                    glColor3f(0.45f,0.12f,0.7f);
                    glLineWidth(6);
                    glBegin(GL_LINES);
                        glVertex2f(x+margin, y+margin);
                        glVertex2f(x+cellSize-margin, y+cellSize-margin);
                        glVertex2f(x+cellSize-margin, y+margin);
                        glVertex2f(x+margin, y+cellSize-margin);
                    glEnd();
                    glLineWidth(1);
                } else if (board[i][j] == 2) {
                    // O - peach
                    float cx = x + cellSize/2;
                    float cy = y + cellSize/2;
                    float radius = (cellSize/2.8f) * scale;
                    glColor3f(1.0f,0.42f,0.4f);
                    glLineWidth(6);
                    glBegin(GL_LINE_LOOP);
                        for(int k=0;k<=100;k++){
                            float a = 2*3.1415926f*k/100.0f;
                            glVertex2f(cx + cos(a)*radius, cy + sin(a)*radius);
                        }
                    glEnd();
                    glLineWidth(1);
                }
            }
        }
    }

    void updateAnimation() {
        bool changed=false;
        for(int i=0;i<n;i++) for(int j=0;j<n;j++){
            if(board[i][j] != 0 && anim[i][j] < 1.0f){
                anim[i][j] += 0.03f;
                if(anim[i][j] > 1.0f) anim[i][j] = 1.0f;
                changed = true;
            }
        }
        if(changed) glutPostRedisplay();
    }

    // ---------- game logic ----------
    bool checkWinFor(int p) {
        // rows & cols
        for(int i=0;i<n;i++){
            bool r=true, c=true;
            for(int j=0;j<n;j++){
                if(board[i][j] != p) r=false;
                if(board[j][i] != p) c=false;
            }
            if(r || c) return true;
        }
        // diagonals
        bool d1=true, d2=true;
        for(int i=0;i<n;i++){
            if(board[i][i] != p) d1=false;
            if(board[i][n-1-i] != p) d2=false;
        }
        return d1 || d2;
    }

    bool isDraw() {
        for(int i=0;i<n;i++)
            for(int j=0;j<n;j++)
                if(board[i][j] == 0) return false;
        return true;
    }

    // Place a move for human (or player2). x,y are window coords; returns true if placed
    bool placeAtWindowCoord(int wx, int wy, float startX, float startY, float cellSize) {
        int col = int((wx - startX) / cellSize);
        int row = int((wy - startY) / cellSize);
        if (row < 0 || row >= n || col < 0 || col >= n) return false;
        if (board[row][col] != 0) return false;
        board[row][col] = currentPlayer;
        anim[row][col] = 0.0f;
        // check end
        if (checkWinFor(currentPlayer)) {
            gameOver = true;
            winner = currentPlayer;
            if (winner == 1) scoreX++; else scoreO++;
        } else if (isDraw()) {
            gameOver = true;
            winner = 0; // draw
        } else {
            currentPlayer = 3 - currentPlayer;
        }
        return true;
    }

    // ---------- Minimax with alpha-beta ----------
    // Evaluate: +100 - depth if O wins, -100 + depth if X wins, 0 draw
    int evaluate(int depth) {
        if (checkWinFor(2)) return 100 - depth;
        if (checkWinFor(1)) return -100 + depth;
        return 0;
    }

    int minimaxAB(bool isMaximizing, int depth, int alpha, int beta) {
        if (checkWinFor(2) || checkWinFor(1) || isDraw()) {
            return evaluate(depth);
        }
        if (isMaximizing) {
            int best = numeric_limits<int>::min();
            for(int i=0;i<n;i++){
                for(int j=0;j<n;j++){
                    if(board[i][j] == 0){
                        board[i][j] = 2;
                        int val = minimaxAB(false, depth+1, alpha, beta);
                        board[i][j] = 0;
                        best = max(best, val);
                        alpha = max(alpha, best);
                        if(beta <= alpha) return best;
                    }
                }
            }
            return best;
        } else {
            int best = numeric_limits<int>::max();
            for(int i=0;i<n;i++){
                for(int j=0;j<n;j++){
                    if(board[i][j] == 0){
                        board[i][j] = 1;
                        int val = minimaxAB(true, depth+1, alpha, beta);
                        board[i][j] = 0;
                        best = min(best, val);
                        beta = min(beta, best);
                        if(beta <= alpha) return best;
                    }
                }
            }
            return best;
        }
    }

    // Choose best move for computer (player 2)
    pair<int,int> findBestMove() {
        int bestVal = numeric_limits<int>::min();
        pair<int,int> move = {-1,-1};
        for(int i=0;i<n;i++){
            for(int j=0;j<n;j++){
                if(board[i][j] == 0){
                    board[i][j] = 2;
                    int moveVal = minimaxAB(false, 0, numeric_limits<int>::min(), numeric_limits<int>::max());
                    board[i][j] = 0;
                    if(moveVal > bestVal){
                        bestVal = moveVal;
                        move = {i,j};
                    }
                }
            }
        }
        return move;
    }

    // Called to let computer play (with animation setup and updating state)
    void computerPlay() {
        if(gameOver) return;
        pair<int,int> m = findBestMove();
        if(m.first != -1){
            board[m.first][m.second] = 2;
            anim[m.first][m.second] = 0.0f;
            if (checkWinFor(2)) {
                gameOver = true;
                winner = 2;
                scoreO++;
            } else if (isDraw()) {
                gameOver = true;
                winner = 0;
            } else {
                currentPlayer = 1;
            }
        }
    }

    // Draw everything given current viewport; handles name/roll/score text
    void renderFull(float startX, float startY, float cellSize) {
        drawGrid(startX, startY, cellSize);
        drawMarks(startX, startY, cellSize);
        // texts: name/roll/score
        drawText(20, WIN_H - 30, "Name: Mst. Shajia Tabassum Orthe", 0,0,0);
        drawText(20, WIN_H - 55, "Roll: 230101", 0,0,0);
        char buf[64];
        sprintf(buf, "Score -> X: %d   O: %d", scoreX, scoreO);
        drawText(20, WIN_H - 80, buf, 0,0,0);

        // result
        if(gameOver){
            if(winner == 1) drawText(WIN_W/2 - 90, WIN_H/2 + 10, "Player X Wins!", 1,0.8f,0.1f);
            else if(winner == 2) drawText(WIN_W/2 - 90, WIN_H/2 + 10, "Player O Wins!", 1,0.8f,0.1f);
            else drawText(WIN_W/2 - 60, WIN_H/2 + 10, "Match Draw!", 1,0.6f,0.2f);
            drawText(WIN_W/2 - 130, WIN_H/2 - 20, "Click Restart button or Back to Menu", 0.2f,0.2f,0.2f);
        }

        updateAnimation();
    }

    // Manual restart (keep scores)
    void manualRestart() {
        for (int i=0;i<n;i++) for(int j=0;j<n;j++){
            board[i][j] = 0;
            anim[i][j] = 0.0f;
        }
        currentPlayer = 1;
        gameOver = false;
        winner = 0;
    }
};

// ---------- Global game pointer ----------
TicTacToe* game = nullptr;

// ---------- Utility: Buttons ----------
struct Button {
    float x,y,w,h;
    string label;
};
vector<Button> menuButtonsMode;
vector<Button> menuButtonsSize;
Button btnBackToMenu;
Button btnRestart;
Button btnQuit;

// ---------- Compute board viewport ----------
void computeBoardViewport(float &startX, float &startY, float &cellSize) {
    float boardW = WIN_W - 2*MARGIN;
    float boardH = WIN_H - 2*MARGIN;
    int n = game->getN();
    float rawCellW = boardW / (float)n;
    float rawCellH = boardH / (float)n;
    cellSize = min(rawCellW, rawCellH);
    float total = cellSize * n;
    startX = (WIN_W - total) / 2.0f;
    startY = (WIN_H - total) / 2.0f;
}

// ---------- Drawing background ----------
void drawBackground() {
    glBegin(GL_QUADS);
        glColor3f(0.85f,0.93f,0.98f); glVertex2f(0,0);
        glColor3f(0.75f,0.88f,0.95f); glVertex2f(WIN_W,0);
        glColor3f(0.65f,0.84f,0.92f); glVertex2f(WIN_W,WIN_H);
        glColor3f(0.75f,0.88f,0.96f); glVertex2f(0,WIN_H);
    glEnd();
}

// helper draw simple button
void drawButton(const Button &b, bool highlight=false) {
    if(highlight) glColor3f(0.25f,0.6f,0.95f);
    else glColor3f(0.18f,0.45f,0.8f);
    glBegin(GL_QUADS);
        glVertex2f(b.x, b.y);
        glVertex2f(b.x + b.w, b.y);
        glVertex2f(b.x + b.w, b.y + b.h);
        glVertex2f(b.x, b.y + b.h);
    glEnd();
    glColor3f(1,1,1);
    glRasterPos2f(b.x + 10, b.y + b.h/2 - 7);
    for (const char* c = b.label.c_str(); *c; ++c) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
}

// ---------- Display callbacks ----------
void displayMenu() {
    // draw mode selection or size selection based on menuStep
    glClear(GL_COLOR_BUFFER_BIT);
    drawBackground();

    // title
    glColor3f(0.08f,0.2f,0.4f);
    glRasterPos2f(WIN_W/2 - 120, WIN_H - 80);
    const char* title = "Tic Tac Toe - Choose Mode and Size";
    for (const char* c = title; *c; ++c) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);

    if(menuStep == MODE_SELECT) {
        // draw buttons for modes
        for (auto &b: menuButtonsMode) drawButton(b);
        // small hint
        glColor3f(0,0,0);
        const char* hint = "Choose Mode -> then click Next (Select size)";
        glRasterPos2f(WIN_W/2 - 140, WIN_H/2 - 80);
        for(const char* c=hint; *c; ++c) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    } else {
        // size select
        for (auto &b: menuButtonsSize) drawButton(b);
        // back button
        drawButton(btnBackToMenu);
    }

    // quit button bottom-right
    drawButton(btnQuit);

    glutSwapBuffers();
}

void displayGame() {
    glClear(GL_COLOR_BUFFER_BIT);
    drawBackground();

    float sx, sy, cs;
    computeBoardViewport(sx, sy, cs);

    if(game) {
        game->renderFull(sx, sy, cs);
    }

    // draw Restart and Back buttons
    drawButton(btnRestart);
    drawButton(btnBackToMenu);

    glutSwapBuffers();
}

// ---------- Mouse helpers ----------
bool pointInButton(int mx, int my, const Button &b) {
    return mx >= (int)b.x && mx <= (int)(b.x + b.w) && my >= (int)b.y && my <= (int)(b.y + b.h);
}

// ---------- Timer for computer move ----------
void timerComputer(int value) {
    if(game && !game->isGameOver() && selectedMode == HUMAN_VS_COMPUTER && game->getCurrentPlayer() == 2){
        game->computerPlay();
        glutPostRedisplay();
    }
}

// ---------- Mouse callback ----------
void mouseFunc(int button, int state, int mx, int my) {
    if(state != GLUT_DOWN) return;
    // convert to our coordinate system (OpenGL origin at bottom-left)
    int y = WIN_H - my;

    if(appState == STATE_MENU) {
        if(menuStep == MODE_SELECT) {
            // buttons for mode
            for(size_t i=0;i<menuButtonsMode.size();++i){
                if(pointInButton(mx, y, menuButtonsMode[i])){
                    if(i==0) selectedMode = HUMAN_VS_COMPUTER;
                    else selectedMode = HUMAN_VS_HUMAN;
                    // go to size selection
                    menuStep = SIZE_SELECT;
                    glutPostRedisplay();
                    return;
                }
            }
            if(pointInButton(mx,y,btnQuit)){
                // exit
                exit(0);
            }
        } else { // SIZE_SELECT
            for(size_t i=0;i<menuButtonsSize.size();++i){
                if(pointInButton(mx,y,menuButtonsSize[i])){
                    selectedSize = (i==0?3:4);
                    // start game
                    appState = STATE_PLAY;
                    if(game) delete game;
                    game = new TicTacToe(selectedSize);
                    // initial redraw
                    glutPostRedisplay();
                    return;
                }
            }
            if(pointInButton(mx,y,btnBackToMenu)){
                menuStep = MODE_SELECT;
                selectedMode = MODE_NONE;
                selectedSize = 0;
                glutPostRedisplay();
                return;
            }
            if(pointInButton(mx,y,btnQuit)){
                exit(0);
            }
        }
    } else if(appState == STATE_PLAY) {
        // Back to menu
        if(pointInButton(mx,y,btnBackToMenu)){
            // free game and return to menu
            if(game) { delete game; game = nullptr; }
            appState = STATE_MENU;
            menuStep = MODE_SELECT;
            selectedMode = MODE_NONE;
            selectedSize = 0;
            glutPostRedisplay();
            return;
        }
        // Restart button
        if(pointInButton(mx,y,btnRestart)){
            if(game){
                game->manualRestart();
                glutPostRedisplay();
            }
            return;
        }

        // If game over and clicked anywhere else do nothing (user should press restart/back)
        if(game && game->isGameOver()){
            return;
        }

        // Else handle board click if it's player's turn
        if(game){
            float sx, sy, cs;
            computeBoardViewport(sx, sy, cs);
            // only allow click when it's human's turn
            bool humanTurnAllowed = false;
            if(selectedMode == HUMAN_VS_HUMAN) humanTurnAllowed = true;
            else if(selectedMode == HUMAN_VS_COMPUTER) humanTurnAllowed = (game->getCurrentPlayer() == 1);

            if(humanTurnAllowed){
                bool placed = game->placeAtWindowCoord(mx, y, sx, sy, cs);
                if(placed){
                    glutPostRedisplay();
                    // if now computer's turn, schedule timer
                    if(selectedMode == HUMAN_VS_COMPUTER && game->getCurrentPlayer() == 2 && !game->isGameOver()){
                        glutTimerFunc(300, timerComputer, 0); // 300ms delay
                    }
                }
            }
        }
    }
}

// ---------- Reshape ----------
void reshape(int w, int h){
    WIN_W = w; WIN_H = h;
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluOrtho2D(0, w, 0, h);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
}

// ---------- Idle / display router ----------
void displayRouter(){
    if(appState == STATE_MENU) displayMenu();
    else displayGame();
}

// ---------- Setup buttons layout ----------
void setupButtons(){
    menuButtonsMode.clear();
    menuButtonsSize.clear();

    // Mode buttons (centered)
    Button b1; b1.w = 260; b1.h = 60; b1.x = (WIN_W - b1.w)/2.0f; b1.y = WIN_H/2 + 40; b1.label = "Player1  VS  Computer";
    Button b2 = b1; b2.y = WIN_H/2 - 40; b2.label = "Player1  VS  Player2";
    menuButtonsMode.push_back(b1);
    menuButtonsMode.push_back(b2);

    // Size buttons
    Button s1; s1.w = 200; s1.h = 60; s1.x = (WIN_W - s1.w)/2.0f; s1.y = WIN_H/2 + 10; s1.label = "3 x 3";
    Button s2 = s1; s2.y = WIN_H/2 - 90; s2.label = "4 x 4";
    menuButtonsSize.push_back(s1);
    menuButtonsSize.push_back(s2);

    // Back, Restart, Quit
    btnBackToMenu.w = 160; btnBackToMenu.h = 45; btnBackToMenu.x = 20; btnBackToMenu.y = 20; btnBackToMenu.label = "Back to Menu";
    btnRestart.w = 140; btnRestart.h = 45; btnRestart.x = WIN_W - 160; btnRestart.y = 20; btnRestart.label = "Restart";
    btnQuit.w = 110; btnQuit.h = 40; btnQuit.x = WIN_W - 130; btnQuit.y = WIN_H - 70; btnQuit.label = "Quit";
}

// ---------- Main ----------
int main(int argc, char** argv){
    srand((unsigned int)time(nullptr));
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIN_W, WIN_H);
    glutCreateWindow("Tic Tac Toe - Strong Computer (Minimax) - Orthe");

    glClearColor(1,1,1,1);

    setupButtons();

    glutDisplayFunc(displayRouter);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouseFunc);

    // Note: when window size changes, update button layout
    // We'll re-setup on reshape by installing a small lambda via idle - simpler: wrap reshape to call setupButtons
    // But glutReshapeFunc can't be a lambda capturing setupButtons - we'll call setupButtons at start and also rely on user not resizing heavily.
    // For robustness, set an idle to recompute button positions when size changes:
    glutIdleFunc([](){
        static int lastW = WIN_W, lastH = WIN_H;
        if(lastW != WIN_W || lastH != WIN_H){
            setupButtons();
            lastW = WIN_W; lastH = WIN_H;
        }
        glutPostRedisplay();
    });

    glutMainLoop();

    // cleanup
    if(game) { delete game; game = nullptr; }

    return 0;
}
