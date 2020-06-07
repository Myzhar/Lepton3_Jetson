#ifndef STOPWATCH_HPP
#define STOPWATCH_HPP

#include <stdlib.h>
#include <chrono>

class StopWatch
{
public:
    StopWatch(){};

    void tic(); ///< Start timer
    double toc(); ///< Returns microsec elapsed from last tic

private:
    std::chrono::high_resolution_clock::time_point mStart;
};

#endif
