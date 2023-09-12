#define REG_SIZE unsigned short
#define BYTE unsigned char
#define REAL_ADDR_SIZE unsigned int 
#include <string>

class cp8086
{
private:
    BYTE memory[1048576];
    REG_SIZE ax,bx,cx,dx;
    REG_SIZE si,di,bp,sp,dp,cs,ds,es,ss;
    REG_SIZE ip;
    REG_SIZE flags;
    REAL_ADDR_SIZE real_addr;
    void convert_cs_ip_to_real();
    REAL_ADDR_SIZE local_convert_cs_ip_to_real(REG_SIZE c,REG_SIZE i);
    void flagCF(int s);
    void flagDF(int s);
    void flagIF(int s);
    void printFlags();
    void printRegs();
    void printStack();
    void mov(BYTE com);
    void push(BYTE com);
    void pop(BYTE com);
public:
    void init();
    int set_initial_addr(REAL_ADDR_SIZE initial_addr);
    void load_mem(std::string name);
    int run();
};



