#ifndef _KLIBS_HOLDS_PROCESS_REFERENCE_HPP_
#define _KLIBS_HOLDS_PROCESS_REFERENCE_HPP_

#include <inc/klibs/maybe.h>

class HoldsProcessReference {
    private:
        Maybe<ProcessDesc *> pd = Nothing;

    public:
        void setProcess(ProcessDesc *_pd)
        {
            pd = _pd;
        }

        Maybe<ProcessDesc *> getProcess()
        {
            return pd;
        }
};

#endif
