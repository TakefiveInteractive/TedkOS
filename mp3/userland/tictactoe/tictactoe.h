#ifndef _TICTACTOE_H_
#define _TICTACTOE_H_

#include "../../shared/ece391syscall.h"
// GUI functions
extern void * TedCreateWindow(int32_t width, int32_t height);
extern int32_t TedSetText(void *elem, char *text);
extern int32_t TedGetText(void *elem, char *buffer);
extern int32_t TedElemShow(void *elem);
extern int32_t TedElemHide(void *elem);

extern void * TedCreateButton(int32_t width, int32_t height, int32_t pos_x, int32_t pos_y);

extern int32_t TedGetMessage(void *message);
extern int32_t TedAttachMessageHandler(void *elem, void *fn);

extern void * TedCreateImage(int32_t width, int32_t height, int32_t pos_x, int32_t pos_y);
extern int32_t TedSetImageData(void *elem, void *data);

class tictactoe {
private:
    constexpr static char Space = ' ';
    constexpr static char ODrop = 'O';
    constexpr static char XDrop = 'X';
    constexpr static int WindowSize = 600;

    char board[3][3] = { {Space}, {Space}, {Space} };
    bool isOTerm = true;

    char* backgroundImageBuffer;
    char* ODropImageBuffer;
    char* XDropImageBuffer;

    auto checkWin(int row, int col) -> bool;
    auto drawBoardWindow() -> void;
    auto drawButtons() -> void;
    auto gameEndWithDraw() -> void;
    auto gameEndWithWinner() -> void;

public:
    tictactoe();
    ~tictactoe();
    auto runGame() -> void;
};

#endif
