/* 2. Understanding Disassembled Functions */

int calculate(unsigned int arg1, unsigned int arg2) {
    switch (arg1) {
        case 0: return arg2 * arg2;
        case 1: return -arg2;
        case 2: return arg2 + 0x80;
        default: return arg2;
    }
}