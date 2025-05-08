#pragma once

#include <signal.h>

class Signal{
    private:
        int signal_;
    public:
        Signal(int signal);
        void signalHandler(int signum);
};