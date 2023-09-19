#define REG_SIZE unsigned short
#define BYTE unsigned char
#define REAL_ADDR_SIZE unsigned int 
#include <string>

union REGISTER
{
    REG_SIZE reg;
    BYTE reg_half[2];
};

class cp8086
{
private:
    BYTE memory[1048576];
    REGISTER ax,bx,cx,dx;
    REGISTER si,di,bp,sp,dp,cs,ds,es,ss;
    REGISTER ip;
    REG_SIZE flags;
    REAL_ADDR_SIZE real_addr;
    void convert_cs_ip_to_real();
    REAL_ADDR_SIZE local_convert_cs_ip_to_real(REGISTER c,REGISTER i);
    REAL_ADDR_SIZE local_convert_cs_ip_to_real(REGISTER c,REG_SIZE i);
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
    void xchg(BYTE com);
public:
    void init();
    int set_initial_addr(REAL_ADDR_SIZE initial_addr);
    void load_mem(std::string name);
    void load_mem_hex(std::string name);
    void memory_dump();
    int run();
};



