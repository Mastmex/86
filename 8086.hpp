#define REG_SIZE signed short
#define BYTE signed char
#define REAL_ADDR_SIZE signed int 
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
    int* error=nullptr;
    REG_SIZE flags;
    REAL_ADDR_SIZE real_addr;
    void convert_cs_ip_to_real();
    REAL_ADDR_SIZE local_convert_cs_ip_to_real(REGISTER c,REGISTER i);
    REAL_ADDR_SIZE local_convert_cs_ip_to_real(REGISTER c,REG_SIZE i);
    REAL_ADDR_SIZE local_convert_cs_ip_to_real(REG_SIZE c,REG_SIZE i);
    void flagCF(int s);
    void flagPF(int s);
    void flagAF(int s);
    void flagZF(int s);
    void flagSF(int s);
    void flagDF(int s);
    void flagIF(int s);
    void flagOF(int s);
    void printFlags();
    void printRegs();
    void printStack();
    void mov(BYTE com);
    void push(BYTE com);
    void pop(BYTE com);
    void xchg(BYTE com);
    void lea(BYTE com);
    void lds(BYTE com);
    void les(BYTE com);
    void lahf(BYTE com);
    void sahf(BYTE com);
    void pushf(BYTE com);
    void popf(BYTE com);
    void add(BYTE com);
    void cmp(BYTE com);
    void jmp(BYTE com);
public:
    void init();
    int set_initial_addr(REAL_ADDR_SIZE initial_addr);
    void load_mem(std::string name);
    void load_mem_hex(std::string name);
    void set_error_data(int *a) {error=a;};
    void memory_dump();
    int run();
};



