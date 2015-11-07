.global getCurrentThreadInfo
.text

getCurrentThreadInfo:
    movl %eax, %esp
    andl $THREAD_ESP_TO_PCB, %eax
    ret
