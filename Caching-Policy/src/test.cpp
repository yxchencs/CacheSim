#include "utils/progress_bar.h"

int main()
{
    int total_steps = 100; // 定义总步骤数为100

    for (int step = 0; step <= total_steps; ++step)
    {
        show_progress_bar(step, total_steps);                        // 调用进度条显示函数
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 模拟每步耗时100毫秒
    }

    std::cout << std::endl
              << "进度完成!" << std::endl;
    return 0;
}