#include <iostream>
#include <bitset>
#include <ctime>
#include <chrono>
#include "8086.hpp"
int main()
{
    std::cout << sizeof(int) << "\n";
    cp8086 a;
    auto start = std::chrono::system_clock::now();
    a.init();
    //a.load_mem("../mem.mem");
    a.load_mem_hex("../mem.hex");
    if (!a.set_initial_addr(0X00000))
        std::cout << "Error in address!\n";
    auto x = a.run();
    if (x == 0)
        std::cout << "Ended by hlt\n";
    else
        std::cout << "Ended by error";
    a.memory_dump();
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);

    std::cout << "finished computation at " << std::ctime(&end_time)
              << "elapsed time: " << elapsed_seconds.count() << "s"
              << std::endl;
    return 0;
}