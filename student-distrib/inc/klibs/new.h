#ifndef _INC_NEW_H_
#define _INC_NEW_H_

void* operator new(size_t s);
void* operator new[](size_t s);
void operator delete(void *p);
void operator delete[](void *p);

#endif
