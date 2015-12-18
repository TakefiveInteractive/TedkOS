#ifndef _UI_TASKLET_H
#define _UI_TASKLET_H

#include <inc/klibs/deque.h>
#include <inc/klibs/function.h>
#include <inc/proc/tasks.h>
#include <inc/klibs/spinlock.h>

extern "C"
{
    void __attribute__((fastcall)) ui_tasklet_entry(void* arg);
}

namespace ui
{
    // A simple tasklet to run tasks
    class Tasklet
    {
    private:
        Tasklet();
        Deque<function<void (void)> > queue;

        thread_kinfo* thread = NULL;
        static Tasklet* singleton;
        spinlock_t lock = SPINLOCK_UNLOCKED;
    public:
        static Tasklet* getInstance();
        void entry(void* arg);
        void add(const function<void (void)>& task);

        inline size_t size() const
        {
            return queue.size();
        }
    };
}


#endif//_UI_TASKLET_H
