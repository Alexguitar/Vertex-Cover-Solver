#pragma once
#include <chrono>

using namespace std;

// global variable
extern chrono::time_point<chrono::steady_clock> TIME_start;

// max heuristic runtime
#define TIMEOUT 160.0
