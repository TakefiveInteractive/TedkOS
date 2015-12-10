#include "tictactoe.h"
#include <stdio.h>
#include <stdlib.h>

tictactoe::tictactoe() {
    drawBoardWindow();
    loadImageResources();
    drawButtons();
}

extern "C" void _Unwind_Resume() {}
void *__gxx_personality_v0;

extern "C" int main() {
    tictactoe a;
    a.runGame();
    return 0;
}

auto loadImageHelper(char* &ptr, char *filename) -> void {
    FILE *fp = fopen (filename, "rb");
    fseek(fp, 0L, SEEK_END);
    auto sz = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    ptr = (char *) malloc(sizeof(char) * sz);
    fread(ptr, sz, 1, fp);
    fclose(fp);
}

auto tictactoe::loadImageResources() -> void {
    loadImageHelper(backgroundImageBuffer, (char *)"tictactoe_bg_img");
    loadImageHelper(ODropImageBuffer, (char *)"tictactoe_o_img");
    loadImageHelper(XDropImageBuffer, (char *)"tictactoe_x_img");
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

auto tictactoe::drawButtons() -> void {

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            void *dropButton = TedCreateButton(SingleSize, SingleSize, SingleSize * j, SingleSize * i);
            TedAttachMessageHandler(dropButton, (void *)(i * 3 + j));
            TedElemShow(dropButton);
        }
    }
}

auto tictactoe::checkWin(int row, int col) -> bool {
    if (
        (board[row][col] != Space) &&
        (
            (
                board[row][0] == board[row][1] &&
                board[row][0] == board[row][2]
            ) || (
                board[0][col] == board[1][col] &&
                board[0][col] == board[2][col]
            )
        )
    ) return true;
    else if (
        (board[1][1] != Space) &&
        (
            (
                (board[1][1] == board[0][0]) &&
                (board[1][1] == board[2][2])
            ) || (
                (board[1][1] == board[0][2]) &&
                (board[1][1] == board[2][0])
            )
        )
    ) return true;
    else return false;
}

auto tictactoe::drawDrop(int row, int col) -> void {
    if (board[row][col] == Space) return;
    void *drop = TedCreateImage(SingleSize, SingleSize, SingleSize * col, SingleSize * row);
    char *buf = (isOTerm == true) ? ODropImageBuffer : XDropImageBuffer;
    TedSetImageData(drop, (void *)buf);
    TedElemShow(drop);
}

auto tictactoe::runGame() -> void {
    drawBoardWindow();
    bool isDraw = false;
    for (int i = 0; i < 9; i++) {
        void *message;
        TedGetMessage(message);
        int buttonTag = (int)(message);
        int buttonRow = buttonTag / 3;
        int buttonCol = buttonTag % 3;
        board[buttonRow][buttonCol] = (isOTerm == true) ? ODrop : XDrop;
        drawDrop(buttonRow, buttonCol);
        if (checkWin(buttonRow, buttonCol)) {
            isDraw = false;
            break;
        }
        isOTerm = !isOTerm;
    }
    if (isDraw)
        gameEndWithDraw();
    else gameEndWithWinner();
}

auto tictactoe::gameEndWithDraw() -> void {

}

auto tictactoe::gameEndWithWinner() -> void {

}

tictactoe::~tictactoe() {
    free(backgroundImageBuffer);
    free(ODropImageBuffer);
    free(XDropImageBuffer);
}
