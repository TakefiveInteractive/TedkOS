#ifndef _UI_SYSCALL_H_
#define _UI_SYSCALL_H_

#include <inc/ui/compositor.h>

namespace ui {

Container* createWindow(int32_t width, int32_t height);
int32_t setText(Container *elem, char *text);
int32_t getText(Container *elem, char *buffer);
int32_t showElement(Container *elem);
int32_t hideElement(Container *elem);
Container* createButton(int32_t width, int32_t height, int32_t pos_x, int32_t pos_y);
int32_t getMessage(void *msg);
int32_t attacheMessageHandler(Container *elem, void *args);
Container* createImage(int32_t width, int32_t height, int32_t pos_x, int32_t pos_y);
int32_t setImageData(Container *img, uint8_t *data);

}

#endif
