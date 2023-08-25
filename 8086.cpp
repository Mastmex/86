#include "8086.hpp"
#include <iostream>
#include <fstream>
#include <bitset>
#include <cstdio>

//comment this for disable command output
#define DEBUG
//---------------------------------------

#ifdef DEBUG
#define PRINT_COM(now,name) printf("%d - %s\n",now,name);
#else
#define PRINT_COM (now,name) return;
#endif

void cp8086::init()
{
    for(int i=0;i<1048576;i++)
        memory[i]=0;
    ax=bx=cx=dx=0;
    si=di=bp=cs=ds=es=0;
    ss=0XF000;
    sp=0XFFFE;
    ip=0;
    real_addr=0;
    flags=0;
}

int cp8086::set_initial_addr(REAL_ADDR_SIZE input_addr)
{
    if(input_addr>=1048576){
        return 0;
    }
    real_addr=input_addr;
    ip=input_addr&0X0FFFF;
    cs=(input_addr&0XF0000)>>4;
    std::cout<<std::hex<<cs<<'\n';
    std::cout<<std::hex<<ip<<'\n';
    return 1;
}

void cp8086::convert_cs_ip_to_real()
{
    real_addr=cs*16+ip;
    if(real_addr>0XFFFFF)
        real_addr&=0X0FFFFF;
}

void cp8086::load_mem(std::string name)
{
    std::ifstream mem(name);
    char buf[9];
    for(int i=0;;i++)
    {
        mem.getline(buf,9);
        int tmp;
        tmp=0;
        for(int c=0;c<8;c++)
        {
            tmp+=buf[c]-'0';
            if(c!=7)
                tmp*=2;
        }
        memory[i]=(BYTE)tmp;
        if(mem.eof())
            break;
    }
    mem.close();
}

void cp8086::printFlags()
{
    printf("%04X\n",flags);
}

int cp8086::run()
{
    //std::cout<<real_addr<<"-----"<<std::bitset<8>{memory[real_addr+1]}<<'\n';
    while(1){

        if(memory[real_addr]==0b11110100){  ///HLT
            PRINT_COM(real_addr,"HLT");
            return 0;
        }

    switch (memory[real_addr])
    {
    case 0b10010000:
        PRINT_COM(real_addr,"NOP");
        break;
    
    case 0b11111001:
        PRINT_COM(real_addr,"STC");
        flagCF(1);
        break;

    case 0b11110101:
        PRINT_COM(real_addr,"SMC");
        flagCF(2);
        break;

    case 0b11111000:
        PRINT_COM(real_addr,"CLC");
        flagCF(0);
        break;
    
    case 0b11111101:
        PRINT_COM(real_addr,"STD");
        flagDF(1);
        break;

    case 0b11111100:
        PRINT_COM(real_addr,"CLD");
        flagDF(0);
        break;

    case 0b11111011:
        PRINT_COM(real_addr,"STI");
        flagIF(1);
        break;

    case 0b11111010:
        PRINT_COM(real_addr,"CLI");
        flagIF(0);
        break;
    
    case 0b10001000:
        PRINT_COM(real_addr,"MOV");
        mov(memory[real_addr]);
        break;
    
    default:
        break;
    }
    if(ip==0XFFFF)
    {
        cs=cs+1;
        ip=0;
    }
    ip=ip+1;
    convert_cs_ip_to_real();
    printFlags();
    //printf("%04X:%04X\n",cs,ip);
    }
}


void cp8086::flagCF(int s)
{
    if(s==0)
        flags&=0XFFFE;
    if(s==1)
        flags|=0X0001;
    if(s==2)
        flags=flags xor 0X0001;
}

void cp8086::flagDF(int s)
{
    if(s==0)
        flags&=0XFBFF;
    if(s==1)
        flags|=0X0400;
    if(s==2)
        flags=flags xor 0X0400;
}

void cp8086::flagIF(int s)
{
    if(s==0)
        flags&=0XFDFF;
    if(s==1)
        flags|=0X0200;
    if(s==2)
        flags=flags xor 0X0200;
}

void cp8086::mov(BYTE com)
{
    if(com&0XFC==0b10001000)            //reg to mem, mem to reg, reg to reg
    {
        BYTE w = com&0XFE;
        BYTE scnd=memory[real_addr+1];
        if(scnd&0XC0==0XC0)             //reg to reg
        {
            if(w==1)
            {
                REG_SIZE *f,*s;
                switch (scnd&0X38)
                {
                case 0b00000000:
                    f=&ax;
                    break;
                
                case 0b00001000:
                    f=&cx;
                    break;

                case 0b00010000:
                    f=&dx;
                    break;

                case 0b00011000:
                    f=&bx;
                    break;

                case 0b00100000:
                    f=&sp;
                    break;

                case 0b00101000:
                    f=&bp;
                    break;

                case 0b00110000:
                    f=&si;
                    break;

                case 0b00111000:
                    f=&di;
                    break;
                }
                switch (scnd&0X07)
                {
                case 0b00000000:
                    s=&ax;
                    break;
                
                case 0b00000001:
                    s=&cx;
                    break;

                case 0b00000010:
                    s=&dx;
                    break;

                case 0b00000011:
                    s=&bx;
                    break;

                case 0b00000100:
                    s=&sp;
                    break;

                case 0b00000101:
                    s=&bp;
                    break;

                case 0b00000110:
                    s=&si;
                    break;

                case 0b00000111:
                    s=&di;
                    break;
                }
                if(com&0X02==0X00)
                    *s=*f;
                else
                    *f=*s;
                return;
            }
            if(w==0)
            {
                bool h1,h2;
                REG_SIZE *f,*s;
                switch (scnd&0X38)
                {
                case 0b00000000:
                    h1=false;
                    f=&ax;
                    break;
                
                case 0b00001000:
                    h1=false;
                    f=&cx;
                    break;

                case 0b00010000:
                    h1=false;
                    f=&dx;
                    break;

                case 0b00011000:
                    h1=false;
                    f=&bx;
                    break;

                case 0b00100000:
                    h1=true;
                    f=&ax;
                    break;

                case 0b00101000:
                    h1=true;
                    f=&cx;
                    break;

                case 0b00110000:
                    h1=true;
                    f=&dx;
                    break;

                case 0b00111000:
                    h1=true;
                    f=&bx;
                    break;
                }
                switch (scnd&0X07)
                {
                case 0b00000000:
                    h2=false;
                    s=&ax;
                    break;
                
                case 0b00000001:
                    h2=false;
                    s=&cx;
                    break;

                case 0b00000010:
                    h2=false;
                    s=&dx;
                    break;

                case 0b00000011:
                    h2=false;
                    s=&bx;
                    break;

                case 0b00000100:
                    h2=true;
                    s=&ax;
                    break;

                case 0b00000101:
                    h2=true;
                    s=&cx;
                    break;

                case 0b00000110:
                    h2=true;
                    s=&dx;
                    break;

                case 0b00000111:
                    h2=true;
                    s=&bx;
                    break;
                }
                if(com&0X02==0X02){
                    REG_SIZE sc;
                    if(!h2)
                        sc=(0|(*s))&0X00FF;
                    else
                        sc=((0|(*s))<<8)&0XFF00;
                    if(!h1)
                    {
                        *f=*f&0XFF00;
                        *f=*f|sc;
                    }
                    else
                    {
                        *f=*f&0X00FF;
                        *f=*f|sc;
                    }
                }
                else
                {
                    REG_SIZE fs;
                    if(!h1)
                        fs=(0|(*f))&0X00FF;
                    else
                        fs=((0|(*f))<<8)&0XFF00;
                    if(!h2)
                    {
                        *s=*s&0XFF00;
                        *s=*s|fs;
                    }
                    else
                    {
                        *s=*s&0X00FF;
                        *s=*s|fs;
                    }
                }
                return;
            }
        }
        
        // REG_SIZE operand;
        // REAL_ADDR_SIZE operand_addr;
        // switch (scnd&0XC7)
        // {
        //     case 0X00:
        //         operand_addr=bx+si;
        //         o
        //         break;
        // default:
        //     break;
        // }
    }
}