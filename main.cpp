#include <iostream>
#include <bitset>
#include "8086.hpp"
int main(){
    std::cout<<sizeof(int)<<"\n";
    cp8086 a;
    a.init();
    a.load_mem("../mem.mem");
    if(!a.set_initial_addr(0X00000))
        std::cout<<"Error in address!\n";
        auto x=a.run();
        if(x==0)
            std::cout<<"Ended by hlt\n";
        else
            std::cout<<"Ended by error";
    return 0;
}