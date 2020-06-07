#include "stopwatch.hpp"

using namespace std;
using namespace chrono;

void StopWatch::tic()
{
    mStart = high_resolution_clock::now();
}

double StopWatch::toc()
{
    auto end = high_resolution_clock::now();
    auto dur = duration_cast<microseconds>(end - mStart);

    double elapsed = static_cast<double>(dur.count());

    return elapsed;
}
