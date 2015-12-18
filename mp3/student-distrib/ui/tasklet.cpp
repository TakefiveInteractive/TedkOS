#include <inc/ui/tasklet.h>
#include <inc/proc/sched.h>
#include <inc/klibs/AutoSpinLock.h>

using namespace scheduler;

void __attribute__((fastcall)) ui_tasklet_entry(void* arg)
{
    ui::Tasklet::getInstance()->entry(arg);
}

namespace ui 
{

Tasklet::Tasklet()
{
    // Tasklet must be singleton
    thread = makeKThread(ui_tasklet_entry);
    attachThread(thread, Waiting);
}

Tasklet* Tasklet::getInstance()
{
    if(singleton)
        return singleton;

    singleton = new Tasklet;
    return singleton;
}

void Tasklet::entry(void* arg)
{
    spin_lock(&lock);
    while(true)
    {
        // wait for queue to be not empty
        if(queue.empty())
        {
            uint32_t flags;
            cli_and_save(flags);
            blockNotDuringInterrupt(thread);

            spin_unlock(&lock);
            restore_flags(flags);

            // Wait till queue is not empty
            // We have to wait because scheduler may not block immediately
            while(queue.empty()) ;
            spin_lock(&lock);
        }

        auto back = *(queue.back());

        // TODO: should differentiate different elements. Each element draw once.
        queue.clear();

        spin_unlock(&lock);
        back();
        spin_lock(&lock);
    }
}

void Tasklet::add(const function<void (void)>& task)
{
    AutoSpinLock l(&lock);
    if(!thread)
        return;
    queue.push_back(task);
    unblock(thread);
}

Tasklet* Tasklet::singleton = NULL;

}
