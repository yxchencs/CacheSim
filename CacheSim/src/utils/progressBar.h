#ifndef _PROGRESS_BAR_H_INCLUDED_
#define _PROGRESS_BAR_H_INCLUDED_

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>

void showProgressBar(int progress, int total)
{
    int bar_width = 70;

    std::cout << "[";
    int pos = bar_width * progress / total;
    for (int i = 0; i < bar_width; ++i)
    {
        if (i < pos)
            std::cout << "=";
        else if (i == pos)
            std::cout << ">";
        else
            std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0 / total) << " %\r";
    std::cout.flush();
}

#endif /*_PROGRESS_BAR_INCLUDED_*/
