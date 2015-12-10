#include "tictactoe.h"
#include <stdio.h>
#include <stdlib.h>

tictactoe::tictactoe() {
    drawBoardWindow();
}

auto tictactoe::drawBoardWindow() -> void {
    // Draw Window
    void *window = TedCreateWindow(WindowSize, WindowSize);
    TedSetText(window, (char *)"Tic Tac Toe");
    TedElemShow(window);

    // Draw background image on main window
    void *background = TedCreateImage(WindowSize, WindowSize, 0, 0);
    FILE *fp = fopen ("tictactoe_bg", "rb");
    fseek(fp, 0L, SEEK_END);
    auto sz = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    char *backgroundImageBuffer = new char[sz];
    fread(backgroundImageBuffer, sz, 1, fp);
    TedSetImageData(background, backgroundImageBuffer);
    TedElemShow(background);
}

auto tictactoe::checkWin(int row, int col) -> bool {
    if (
        (board[row][col] != Space) &&
        ((board[row][0] == board[row][1] && board[row][0] == board[row][2]) ||
        (board[0][col] == board[1][col] && board[0][col] == board[2][col]))
    ) return true;
    else return false;
}

auto tictactoe::runGame() -> void {
    drawBoardWindow();
    for (int i = 0; i < 9; i++) {

        isOTerm = !isOTerm;
    }
    gameEndWithDraw();
}

tictactoe::~tictactoe() {
    delete [] backgroundImageBuffer;
    delete [] ODropImageBuffer;
    delete [] XDropImageBuffer;
}
