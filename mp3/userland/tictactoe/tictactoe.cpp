#include "tictactoe.h"
#include <stdio.h>
#include <stdlib.h>

tictactoe::tictactoe() {
    drawBoardWindow();
    loadImageResources();
}

auto tictactoe::drawBoardWindow() -> void {
    // Draw Window
    void *window = TedCreateWindow(WindowSize, WindowSize);
    TedSetText(window, (char *)"Tic Tac Toe");
    TedElemShow(window);

    // Draw background image on main window
    void *background = TedCreateImage(WindowSize, WindowSize, 0, 0);

    TedSetImageData(background, backgroundImageBuffer);
    TedElemShow(background);
}

auto loadImageHelper(char *ptr, char *filename) -> void {
    FILE *fp = fopen (filename, "rb");
    fseek(fp, 0L, SEEK_END);
    auto sz = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    ptr = new char[sz];
    fread(ptr, sz, 1, fp);
    fclose(fp);
}

auto tictactoe::loadImageResources() -> void {
    loadImageHelper(backgroundImageBuffer, (char *)"tictactoe_bg_img");
    loadImageHelper(ODropImageBuffer, (char *)"tictactoe_o_img");
    loadImageHelper(XDropImageBuffer, (char *)"tictactoe_x_img");
}

auto tictactoe::drawButtons() -> void {
    constexpr int SingleSize = WindowSize / 3;

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            void *dropButton = TedCreateImage(SingleSize, SingleSize, SingleSize * j, SingleSize * i);
            TedAttachMessageHandler(dropButton, (void *)(i * 3 + j));
            TedElemShow(dropButton);
        }
    }
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
        void *message;
        TedGetMessage(message);
        int button_tag = (int)(message);
        if (isOTerm)
        isOTerm = !isOTerm;
    }
    gameEndWithDraw();
}

tictactoe::~tictactoe() {
    delete [] backgroundImageBuffer;
    delete [] ODropImageBuffer;
    delete [] XDropImageBuffer;
}
