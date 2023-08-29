#include "8086.hpp"
#include <iostream>
#include <fstream>
#include <bitset>
#include <cstdio>

// comment this for disable command output
#define DEBUG
//---------------------------------------

#ifdef DEBUG
#define PRINT_COM(now, name) printf("%d - %s\n", now, name);
#else
#define PRINT_COM (now, name) return;
#endif

void cp8086::init()
{
    for (int i = 0; i < 1048576; i++)
        memory[i] = 0;
    ax = bx = cx = dx = 0;
    si = di = bp = cs = ds = es = 0;
    ss = 0XF000;
    sp = 0XFFFE;
    ip = 0;
    real_addr = 0;
    flags = 0;
}

int cp8086::set_initial_addr(REAL_ADDR_SIZE input_addr)
{
    if (input_addr >= 1048576)
    {
        return 0;
    }
    real_addr = input_addr;
    ip = input_addr & 0X0FFFF;
    cs = (input_addr & 0XF0000) >> 4;
    std::cout << std::hex << cs << '\n';
    std::cout << std::hex << ip << '\n';
    return 1;
}

void cp8086::printStack()
{
    printf("FFFF:%02X\tFFFF:%02X\nFFFD:%02X\tFFFC:%02X\nFFFB:%02X\tFFFA:%02X\nFFF9:%02X\tFFF8:%02X\nFFF7:%02X\tFFF6:%02X\nFFF5:%02X\tFFF4:%02X\nFFF3:%02X\tFFF2:%02X\nFFF1:%02X\tFFF0:%02X\n",
           memory[local_convert_cs_ip_to_real(ss, 0XFFFF)],
           memory[local_convert_cs_ip_to_real(ss, 0XFFFE)],
           memory[local_convert_cs_ip_to_real(ss, 0XFFFD)],
           memory[local_convert_cs_ip_to_real(ss, 0XFFFC)],
           memory[local_convert_cs_ip_to_real(ss, 0XFFFB)],
           memory[local_convert_cs_ip_to_real(ss, 0XFFFA)],
           memory[local_convert_cs_ip_to_real(ss, 0XFFF9)],
           memory[local_convert_cs_ip_to_real(ss, 0XFFF8)],
           memory[local_convert_cs_ip_to_real(ss, 0XFFF7)],
           memory[local_convert_cs_ip_to_real(ss, 0XFFF6)],
           memory[local_convert_cs_ip_to_real(ss, 0XFFF5)],
           memory[local_convert_cs_ip_to_real(ss, 0XFFF4)],
           memory[local_convert_cs_ip_to_real(ss, 0XFFF3)],
           memory[local_convert_cs_ip_to_real(ss, 0XFFF2)],
           memory[local_convert_cs_ip_to_real(ss, 0XFFF1)],
           memory[local_convert_cs_ip_to_real(ss, 0XFFF0)]);
}

void cp8086::convert_cs_ip_to_real()
{
    real_addr = cs * 16 + ip;
    if (real_addr > 0XFFFFF)
        real_addr &= 0X0FFFFF;
}
REAL_ADDR_SIZE cp8086::local_convert_cs_ip_to_real(REG_SIZE c, REG_SIZE i)
{
    REAL_ADDR_SIZE r;
    r = c * 16 + i;
    if (r > 0XFFFFF)
        r &= 0X0FFFFF;
    return r;
}

void cp8086::load_mem(std::string name)
{
    std::ifstream mem(name);
    char buf[9];
    for (int i = 0;; i++)
    {
        mem.getline(buf, 9);
        // std::cout<<buf<<'\n';
        int tmp;
        tmp = 0;
        for (int c = 0; c < 8; c++)
        {
            tmp += buf[c] - '0';
            if (c != 7)
                tmp *= 2;
        }
        memory[i] = (BYTE)tmp;
        if (mem.eof())
            break;
    }
    mem.close();
}

void cp8086::printFlags()
{
    printf("%04X\n", flags);
}

void cp8086::printRegs()
{
    printf("\
            User-accessible registers\t\tSegment registers\n\
            AX:%04X\tSP:%04X\t\t\tES:%04X\n\
            BX:%04X\tBP:%04X\t\t\tCS:%04X\n\
            CX:%04X\tSI:%04X\t\t\tSS:%04X\n\
            DX:%04X\tDI:%04X\t\t\tDS:%04X\n",
           ax, sp, es, bx, bp, cs, cx, si, ss, dx, di, ds);
}

int cp8086::run()
{
    // std::cout<<real_addr<<"-----"<<std::bitset<8>{memory[real_addr+1]}<<'\n';
    while (1)
    {

        if (memory[real_addr] == 0b11110100)
        { /// HLT
            PRINT_COM(real_addr, "HLT");
            return 0;
        }

        switch (memory[real_addr])
        {
        case 0b10010000:
            PRINT_COM(real_addr, "NOP");
            break;

        case 0b11111001:
            PRINT_COM(real_addr, "STC");
            flagCF(1);
            break;

        case 0b11110101:
            PRINT_COM(real_addr, "SMC");
            flagCF(2);
            break;

        case 0b11111000:
            PRINT_COM(real_addr, "CLC");
            flagCF(0);
            break;

        case 0b11111101:
            PRINT_COM(real_addr, "STD");
            flagDF(1);
            break;

        case 0b11111100:
            PRINT_COM(real_addr, "CLD");
            flagDF(0);
            break;

        case 0b11111011:
            PRINT_COM(real_addr, "STI");
            flagIF(1);
            break;

        case 0b11111010:
            PRINT_COM(real_addr, "CLI");
            flagIF(0);
            break;
            { // MOV commands
            case 0b10001000:
                PRINT_COM(real_addr, "MOV");
                mov(memory[real_addr]);
                break;
            case 0b10001001:
                PRINT_COM(real_addr, "MOV");
                mov(memory[real_addr]);
                break;
            case 0b10001010:
                PRINT_COM(real_addr, "MOV");
                mov(memory[real_addr]);
                break;
            case 0b10001011:
                PRINT_COM(real_addr, "MOV");
                mov(memory[real_addr]);
                break;
            case 0b11000110:
                PRINT_COM(real_addr, "MOV");
                mov(memory[real_addr]);
                break;
            case 0b11000111:
                PRINT_COM(real_addr, "MOV");
                mov(memory[real_addr]);
                break;
            case 0b10110000:
                PRINT_COM(real_addr, "MOV");
                mov(memory[real_addr]);
                break;
            case 0b10110001:
                PRINT_COM(real_addr, "MOV");
                mov(memory[real_addr]);
                break;
            case 0b10110010:
                PRINT_COM(real_addr, "MOV");
                mov(memory[real_addr]);
                break;
            case 0b10110011:
                PRINT_COM(real_addr, "MOV");
                mov(memory[real_addr]);
                break;
            case 0b10110100:
                PRINT_COM(real_addr, "MOV");
                mov(memory[real_addr]);
                break;
            case 0b10110101:
                PRINT_COM(real_addr, "MOV");
                mov(memory[real_addr]);
                break;
            case 0b10110110:
                PRINT_COM(real_addr, "MOV");
                mov(memory[real_addr]);
                break;
            case 0b10110111:
                PRINT_COM(real_addr, "MOV");
                mov(memory[real_addr]);
                break;
            case 0b10111000:
                PRINT_COM(real_addr, "MOV");
                mov(memory[real_addr]);
                break;
            case 0b10111001:
                PRINT_COM(real_addr, "MOV");
                mov(memory[real_addr]);
                break;
            case 0b10111010:
                PRINT_COM(real_addr, "MOV");
                mov(memory[real_addr]);
                break;
            case 0b10111011:
                PRINT_COM(real_addr, "MOV");
                mov(memory[real_addr]);
                break;
            case 0b10111100:
                PRINT_COM(real_addr, "MOV");
                mov(memory[real_addr]);
                break;
            case 0b10111101:
                PRINT_COM(real_addr, "MOV");
                mov(memory[real_addr]);
                break;
            case 0b10111110:
                PRINT_COM(real_addr, "MOV");
                mov(memory[real_addr]);
                break;
            case 0b10111111:
                PRINT_COM(real_addr, "MOV");
                mov(memory[real_addr]);
                break;
            case 0b10100000:
                PRINT_COM(real_addr, "MOV");
                mov(memory[real_addr]);
                break;
            case 0b10100001:
                PRINT_COM(real_addr, "MOV");
                mov(memory[real_addr]);
                break;
            case 0b10100010:
                PRINT_COM(real_addr, "MOV");
                mov(memory[real_addr]);
                break;
            case 0b10100011:
                PRINT_COM(real_addr, "MOV");
                mov(memory[real_addr]);
                break;
            case 0b10001110:
                PRINT_COM(real_addr, "MOV");
                mov(memory[real_addr]);
                break;
            case 0b10001100:
                PRINT_COM(real_addr, "MOV");
                mov(memory[real_addr]);
                break;
            }

            {                // PUSH commands
            case 0b11111111: // maybe push or jmp or call
                if ((memory[real_addr + 1] & 0X38) == 0b00110000)
                {
                    PRINT_COM(real_addr, "PUSH");
                    push(memory[real_addr]);
                    printStack();
                }
                break;
            case 0b01010000:
                PRINT_COM(real_addr, "PUSH");
                push(memory[real_addr]);
                printStack();
                break;
            case 0b01010001:
                PRINT_COM(real_addr, "PUSH");
                push(memory[real_addr]);
                printStack();
                break;
            case 0b01010010:
                PRINT_COM(real_addr, "PUSH");
                push(memory[real_addr]);
                printStack();
                break;
            case 0b01010011:
                PRINT_COM(real_addr, "PUSH");
                push(memory[real_addr]);
                printStack();
                break;
            case 0b01010100:
                PRINT_COM(real_addr, "PUSH");
                push(memory[real_addr]);
                printStack();
                break;
            case 0b01010101:
                PRINT_COM(real_addr, "PUSH");
                push(memory[real_addr]);
                printStack();
                break;
            case 0b01010110:
                PRINT_COM(real_addr, "PUSH");
                push(memory[real_addr]);
                printStack();
                break;
            case 0b01010111:
                PRINT_COM(real_addr, "PUSH");
                push(memory[real_addr]);
                printStack();
                break;
            case 0b00000110:
                PRINT_COM(real_addr, "PUSH");
                push(memory[real_addr]);
                printStack();
                break;
            case 0b00001110:
                PRINT_COM(real_addr, "PUSH");
                push(memory[real_addr]);
                printStack();
                break;
            case 0b00010110:
                PRINT_COM(real_addr, "PUSH");
                push(memory[real_addr]);
                printStack();
                break;
            case 0b00011110:
                PRINT_COM(real_addr, "PUSH");
                push(memory[real_addr]);
                printStack();
                break;
            }

            { // pop commands
            case 0b10001111:
                PRINT_COM(real_addr, "POP");
                pop(memory[real_addr]);
                printStack();
                break;
            case 0b01011000:
                PRINT_COM(real_addr, "POP");
                pop(memory[real_addr]);
                printStack();
                break;
            case 0b01011001:
                PRINT_COM(real_addr, "POP");
                pop(memory[real_addr]);
                printStack();
                break;
            case 0b01011010:
                PRINT_COM(real_addr, "POP");
                pop(memory[real_addr]);
                printStack();
                break;
            case 0b01011011:
                PRINT_COM(real_addr, "POP");
                pop(memory[real_addr]);
                printStack();
                break;
            case 0b01011100:
                PRINT_COM(real_addr, "POP");
                pop(memory[real_addr]);
                printStack();
                break;
            case 0b01011101:
                PRINT_COM(real_addr, "POP");
                pop(memory[real_addr]);
                printStack();
                break;
            case 0b01011110:
                PRINT_COM(real_addr, "POP");
                pop(memory[real_addr]);
                printStack();
                break;
            case 0b01011111:
                PRINT_COM(real_addr, "POP");
                pop(memory[real_addr]);
                printStack();
                break;
            }
        default:
            return 1;
            break;
        }
        if (ip == 0XFFFF)
        {
            cs = cs + 1;
            ip = 0;
        }
        ip = ip + 1;
        convert_cs_ip_to_real();
        // printFlags();
        printRegs();
        printf("%04X:%04X\n", cs, ip);
    }
}

void cp8086::flagCF(int s)
{
    if (s == 0)
        flags &= 0XFFFE;
    if (s == 1)
        flags |= 0X0001;
    if (s == 2)
        flags = flags ^ 0X0001;
}

void cp8086::flagDF(int s)
{
    if (s == 0)
        flags &= 0XFBFF;
    if (s == 1)
        flags |= 0X0400;
    if (s == 2)
        flags = flags ^ 0X0400;
}

void cp8086::flagIF(int s)
{
    if (s == 0)
        flags &= 0XFDFF;
    if (s == 1)
        flags |= 0X0200;
    if (s == 2)
        flags = flags ^ 0X0200;
}

void cp8086::mov(BYTE com)
{
    if ((com & 0XFC) == 0b10001000) // reg to mem, mem to reg, reg to reg
    {
        BYTE w = com & 0X01;
        BYTE scnd = memory[real_addr + 1];
        if ((scnd & 0XC0) == 0XC0) // reg to reg
        {
            ip++;
            if (w == 1)
            {
                REG_SIZE *f, *s;
                switch ((scnd & 0X38))
                {
                case 0b00000000:
                    f = &ax;
                    break;

                case 0b00001000:
                    f = &cx;
                    break;

                case 0b00010000:
                    f = &dx;
                    break;

                case 0b00011000:
                    f = &bx;
                    break;

                case 0b00100000:
                    f = &sp;
                    break;

                case 0b00101000:
                    f = &bp;
                    break;

                case 0b00110000:
                    f = &si;
                    break;

                case 0b00111000:
                    f = &di;
                    break;
                }
                switch ((scnd & 0X07))
                {
                case 0b00000000:
                    s = &ax;
                    break;

                case 0b00000001:
                    s = &cx;
                    break;

                case 0b00000010:
                    s = &dx;
                    break;

                case 0b00000011:
                    s = &bx;
                    break;

                case 0b00000100:
                    s = &sp;
                    break;

                case 0b00000101:
                    s = &bp;
                    break;

                case 0b00000110:
                    s = &si;
                    break;

                case 0b00000111:
                    s = &di;
                    break;
                }
                if ((com & 0X02) == 0X00)
                {
                    *s = *f;
                }
                else
                {
                    *f = *s;
                }
                return;
            }
            if (w == 0)
            {
                bool h1, h2;
                REG_SIZE *f, *s;
                switch ((scnd & 0X38))
                {
                case 0b00000000:
                    h1 = false;
                    f = &ax;
                    break;

                case 0b00001000:
                    h1 = false;
                    f = &cx;
                    break;

                case 0b00010000:
                    h1 = false;
                    f = &dx;
                    break;

                case 0b00011000:
                    h1 = false;
                    f = &bx;
                    break;

                case 0b00100000:
                    h1 = true;
                    f = &ax;
                    break;

                case 0b00101000:
                    h1 = true;
                    f = &cx;
                    break;

                case 0b00110000:
                    h1 = true;
                    f = &dx;
                    break;

                case 0b00111000:
                    h1 = true;
                    f = &bx;
                    break;
                }
                switch ((scnd & 0X07))
                {
                case 0b00000000:
                    h2 = false;
                    s = &ax;
                    break;

                case 0b00000001:
                    h2 = false;
                    s = &cx;
                    break;

                case 0b00000010:
                    h2 = false;
                    s = &dx;
                    break;

                case 0b00000011:
                    h2 = false;
                    s = &bx;
                    break;

                case 0b00000100:
                    h2 = true;
                    s = &ax;
                    break;

                case 0b00000101:
                    h2 = true;
                    s = &cx;
                    break;

                case 0b00000110:
                    h2 = true;
                    s = &dx;
                    break;

                case 0b00000111:
                    h2 = true;
                    s = &bx;
                    break;
                }
                if ((com & 0X02) == 0X02)
                {
                    REG_SIZE sc;
                    if (!h2)
                        sc = (0 | (*s)) & 0X00FF;
                    else
                        sc = ((0 | (*s)) << 8) & 0XFF00;
                    if (!h1)
                    {
                        *f = *f & 0XFF00;
                        *f = *f | sc;
                    }
                    else
                    {
                        *f = *f & 0X00FF;
                        *f = *f | sc;
                    }
                }
                else
                {
                    REG_SIZE fs;
                    if (!h1)
                        fs = (0 | (*f)) & 0X00FF;
                    else
                        fs = ((0 | (*f)) << 8) & 0XFF00;
                    if (!h2)
                    {
                        *s = *s & 0XFF00;
                        *s = *s | fs;
                    }
                    else
                    {
                        *s = *s & 0X00FF;
                        *s = *s | fs;
                    }
                }
                return;
            }
        }
        else // reg to mem, mem to reg
        {
            REG_SIZE *reg;
            REG_SIZE addr_sum;
            switch ((scnd & 0XC7))
            {
            case 0b00000000:
                addr_sum = bx + si;
                ip++;
                break;

            case 0b00000001:
                addr_sum = bx + di;
                ip++;
                break;

            case 0b00000010:
                addr_sum = bp + si;
                ip++;
                break;

            case 0b00000011:
                addr_sum = bp + di;
                ip++;
                break;

            case 0b00000100:
                addr_sum = si;
                ip++;
                break;

            case 0b00000101:
                addr_sum = di;
                ip++;
                break;

            case 0b00000110:
            {
                REG_SIZE l = memory[real_addr + 3];
                l = l << 8;
                l & 0XFF00;
                l = l | memory[real_addr + 2];
                addr_sum = l;
                ip += 3;
                break;
            }

            case 0b00000111:
                addr_sum = bx;
                ip++;
                break;

            case 0b01000000:
                addr_sum = bx + si + memory[real_addr + 2];
                ip += 2;
                break;

            case 0b01000001:
                addr_sum = bx + di + memory[real_addr + 2];
                ip += 2;
                break;

            case 0b01000010:
                addr_sum = bp + si + memory[real_addr + 2];
                ip += 2;
                break;

            case 0b01000011:
                addr_sum = bp + di + memory[real_addr + 2];
                ip += 2;
                break;

            case 0b01000100:
                addr_sum = si + memory[real_addr + 2];
                ip += 2;
                break;

            case 0b01000101:
                addr_sum = di + memory[real_addr + 2];
                ip += 2;
                break;

            case 0b01000110:
                addr_sum = bp + memory[real_addr + 2];
                ip += 2;
                break;

            case 0b01000111:
                addr_sum = bx + memory[real_addr + 2];
                ip += 2;
                break;

            case 0b10000000:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l & 0XFF00;
                l = l | memory[real_addr + 2];
                addr_sum = bx + si + l;
                ip += 2;
            }
            break;

            case 0b10000001:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l & 0XFF00;
                l = l | memory[real_addr + 2];
                addr_sum = bx + di + l;
                ip += 2;
            }
            break;

            case 0b10000010:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l & 0XFF00;
                l = l | memory[real_addr + 2];
                addr_sum = bp + si + l;
                ip += 2;
            }
            break;

            case 0b10000011:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l & 0XFF00;
                l = l | memory[real_addr + 2];
                addr_sum = bp + di + l;
                ip += 2;
            }
            break;

            case 0b10000100:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l & 0XFF00;
                l = l | memory[real_addr + 2];
                addr_sum = si + l;
                ip += 2;
            }
            break;

            case 0b10000101:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l & 0XFF00;
                l = l | memory[real_addr + 2];
                addr_sum = di + l;
                ip += 2;
            }
            break;
            case 0b10000110:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l & 0XFF00;
                l = l | memory[real_addr + 2];
                addr_sum = bp + l;
                ip += 2;
            }
            break;
            case 0b10000111:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l & 0XFF00;
                l = l | memory[real_addr + 2];
                addr_sum = bx + l;
                ip += 2;
            }
            break;
            default:
                break;
            }
            if (w == 1)
            {
                switch ((scnd & 0X38))
                {
                case 0b00000000:
                    reg = &ax;
                    break;

                case 0b00001000:
                    reg = &cx;
                    break;

                case 0b00010000:
                    reg = &dx;
                    break;

                case 0b00011000:
                    reg = &bx;
                    break;

                case 0b00100000:
                    reg = &sp;
                    break;

                case 0b00101000:
                    reg = &bp;
                    break;

                case 0b00110000:
                    reg = &si;
                    break;

                case 0b00111000:
                    reg = &di;
                    break;
                }
                if ((com & 0X02) == 0X00)
                {
                    memory[local_convert_cs_ip_to_real(ds, addr_sum) + 1] = ((*reg) & 0XFF00) >> 8;
                    memory[local_convert_cs_ip_to_real(ds, addr_sum)] = (*reg) & 0X00FF;
                }
                else
                {
                    *reg = 0;
                    *reg = memory[local_convert_cs_ip_to_real(ds, addr_sum) + 1];
                    *reg = *reg << 8;
                    *reg &= 0XFF00;
                    *reg |= memory[local_convert_cs_ip_to_real(ds, addr_sum)];
                }
            }
            else
            {
                bool h = false;
                switch ((scnd & 0X38))
                {
                case 0b00000000:
                    reg = &ax;
                    h = false;
                    break;

                case 0b00001000:
                    reg = &cx;
                    h = false;
                    break;

                case 0b00010000:
                    h = false;
                    reg = &dx;
                    break;

                case 0b00011000:
                    h = false;
                    reg = &bx;
                    break;

                case 0b00100000:
                    h = true;
                    reg = &ax;
                    break;

                case 0b00101000:
                    h = true;
                    reg = &cx;
                    break;

                case 0b00110000:
                    h = true;
                    reg = &dx;
                    break;

                case 0b00111000:
                    h = true;
                    reg = &bx;
                    break;
                }

                if ((com & 0X02) == 0X00)
                {
                    BYTE dat;
                    if (h == false)
                        dat = *reg & 0X00FF;
                    else
                        dat = ((*reg) & 0XFF00) >> 8;
                    memory[local_convert_cs_ip_to_real(ds, addr_sum)] = dat;
                }
                else
                {
                    if (h == false)
                    {
                        *reg &= 0XFF00;
                        REG_SIZE tmp = 0;
                        tmp = memory[local_convert_cs_ip_to_real(ds, addr_sum)];
                        *reg |= tmp;
                    }
                    else
                    {
                        *reg &= 0X00FF;
                        REG_SIZE tmp = 0;
                        tmp = memory[local_convert_cs_ip_to_real(ds, addr_sum)];
                        tmp = tmp << 8;
                        tmp &= 0XFF00;
                        *reg |= tmp;
                    }
                }
            }
        }
    }

    if ((com & 0XFE) == 0b11000110) // data to mem
    {
        BYTE w = com & 0X01;
        BYTE scnd = memory[real_addr + 1];
        REG_SIZE addr_sum;
        switch ((scnd & 0XC7))
        {
        case 0b00000000:
            addr_sum = bx + si;
            ip++;
            break;

        case 0b00000001:
            addr_sum = bx + di;
            ip++;
            break;

        case 0b00000010:
            addr_sum = bp + si;
            ip++;
            break;

        case 0b00000011:
            addr_sum = bp + di;
            ip++;
            break;

        case 0b00000100:
            addr_sum = si;
            ip++;
            break;

        case 0b00000101:
            addr_sum = di;
            ip++;
            break;

        case 0b00000110:
        {
            REG_SIZE l = memory[real_addr + 3];
            l = l << 8;
            l & 0XFF00;
            l = l | memory[real_addr + 2];
            addr_sum = l;
            ip += 3;
            break;
        }

        case 0b00000111:
            addr_sum = bx;
            ip++;
            break;

        case 0b01000000:
            addr_sum = bx + si + memory[real_addr + 2];
            ip += 2;
            break;

        case 0b01000001:
            addr_sum = bx + di + memory[real_addr + 2];
            ip += 2;
            break;

        case 0b01000010:
            addr_sum = bp + si + memory[real_addr + 2];
            ip += 2;
            break;

        case 0b01000011:
            addr_sum = bp + di + memory[real_addr + 2];
            ip += 2;
            break;

        case 0b01000100:
            addr_sum = si + memory[real_addr + 2];
            ip += 2;
            break;

        case 0b01000101:
            addr_sum = di + memory[real_addr + 2];
            ip += 2;
            break;

        case 0b01000110:
            addr_sum = bp + memory[real_addr + 2];
            ip += 2;
            break;

        case 0b01000111:
            addr_sum = bx + memory[real_addr + 2];
            ip += 2;
            break;

        case 0b10000000:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;
            l & 0XFF00;
            l = l | memory[real_addr + 2];
            addr_sum = bx + si + l;
            ip += 2;
        }
        break;

        case 0b10000001:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;
            l & 0XFF00;
            l = l | memory[real_addr + 2];
            addr_sum = bx + di + l;
            ip += 2;
        }
        break;

        case 0b10000010:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;
            l & 0XFF00;
            l = l | memory[real_addr + 2];
            addr_sum = bp + si + l;
            ip += 2;
        }
        break;

        case 0b10000011:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;
            l & 0XFF00;
            l = l | memory[real_addr + 2];
            addr_sum = bp + di + l;
            ip += 2;
        }
        break;

        case 0b10000100:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;
            l & 0XFF00;
            l = l | memory[real_addr + 2];
            addr_sum = si + l;
            ip += 2;
        }
        break;

        case 0b10000101:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;
            l & 0XFF00;
            l = l | memory[real_addr + 2];
            addr_sum = di + l;
            ip += 2;
        }
        break;
        case 0b10000110:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;
            l & 0XFF00;
            l = l | memory[real_addr + 2];
            addr_sum = bp + l;
            ip += 2;
        }
        break;
        case 0b10000111:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;
            l & 0XFF00;
            l = l | memory[real_addr + 2];
            addr_sum = bx + l;
            ip += 2;
        }
        break;
        default:
            break;
        }
        if (w == 0)
        {
            memory[local_convert_cs_ip_to_real(ds, addr_sum)] = memory[local_convert_cs_ip_to_real(ds, ip) + 1];
            ip++;
            return;
        }
        else
        {
            memory[local_convert_cs_ip_to_real(ds, addr_sum) + 1] = memory[local_convert_cs_ip_to_real(ds, ip) + 2];
            memory[local_convert_cs_ip_to_real(ds, addr_sum)] = memory[local_convert_cs_ip_to_real(ds, ip) + 1];
            ip++;
            ip++;
            return;
        }
    }

    if ((com & 0XF0) == 0b10110000) // data to reg
    {
        BYTE w = com & 0X01;
        if (w == 1)
        {
            REG_SIZE *r;
            switch ((com & 0X0E))
            {
            case 0b00000000:
                r = &ax;
                break;
            case 0b00000010:
                r = &cx;
                break;
            case 0b00000100:
                r = &dx;
                break;
            case 0b00000110:
                r = &bx;
                break;
            case 0b00001000:
                r = &sp;
                break;
            case 0b00001010:
                r = &bp;
                break;
            case 0b00001100:
                r = &si;
                break;
            case 0b00001110:
                r = &di;
                break;
            default:
                break;
            }
            *r = memory[real_addr + 2];
            *r = *r << 8;
            *r |= memory[real_addr + 1];
            ip++;
            ip++;
            return;
        }
        else
        {
            REG_SIZE *r;
            bool h = false;
            switch ((com & 0X0E))
            {
            case 0b00000000:
                h = false;
                r = &ax;
                break;
            case 0b00000010:
                h = false;
                r = &cx;
                break;
            case 0b00000100:
                h = false;
                r = &dx;
                break;
            case 0b00000110:
                h = false;
                r = &bx;
                break;
            case 0b00001000:
                h = true;
                r = &ax;
                break;
            case 0b00001010:
                h = true;
                r = &cx;
                break;
            case 0b00001100:
                h = true;
                r = &dx;
                break;
            case 0b00001110:
                h = true;
                r = &bx;
                break;
            default:
                break;
            }
            if (h == false)
            {
                REG_SIZE t = 0;
                t = memory[real_addr + 1];
                *r = *r & 0XFF00;
                *r = *r | t;
                ip++;
                return;
            }
            else
            {
                REG_SIZE t = 0;
                t = memory[real_addr + 1];
                t = t << 8;
                t = t & 0XFF00;
                *r = *r & 0X00FF;
                *r = *r | t;
                ip++;
                return;
            }
        }
    }

    if ((com & 0XFC) == 0b10100000) // mem to ax, ax to mem
    {
        int w = com & 0X01;
        int d = com & 0X02;
        if (w == 0)
        {
            if (d == 0)
            {
                ax = memory[local_convert_cs_ip_to_real(ds, memory[real_addr + 1])];
                ip++;
                return;
            }
            else
            {
                memory[local_convert_cs_ip_to_real(ds, memory[real_addr + 1])] = ax & 0X00FF;
                ip++;
                return;
            }
        }
        else
        {
            if (d == 0)
            {
                ax = memory[local_convert_cs_ip_to_real(ds, memory[real_addr + 2])];
                ax = ax << 8;
                ax = ax & 0XFF00;
                ax = ax | memory[local_convert_cs_ip_to_real(ds, memory[real_addr + 1])];
                ip++;
                ip++;
                return;
            }
            else
            {
                BYTE h, l;
                l = ax & 0X00FF;
                h = (ax & 0XFF00) >> 8;
                memory[local_convert_cs_ip_to_real(ds, memory[real_addr + 1])] = l;
                memory[local_convert_cs_ip_to_real(ds, memory[real_addr + 2])] = h;
                ip++;
                ip++;
                return;
            }
        }
    }

    if ((com & 0XFD) == 0b10001100) // mem/reg to seg, seg to mem/reg
    {
        BYTE scnd = memory[real_addr + 1];
        REG_SIZE *f;
        switch (scnd & 0X18)
        {
        case 0X00:
            f = &es;
            break;
        case 0X08:
            f = &cs;
            break;
        case 0X10:
            f = &ss;
            break;
        case 0X18:
            f = &ds;
            break;
        default:
            break;
        }
        if ((scnd & 0XC0) == 0XC0) // reg to seg, seg to reg
        {
            REG_SIZE *s;
            switch (scnd & 0X07)
            {
            case 0X00:
                s = &ax;
                break;
            case 0X01:
                s = &cx;
                break;
            case 0X02:
                s = &dx;
                break;
            case 0X03:
                s = &bx;
                break;
            case 0X04:
                s = &sp;
                break;
            case 0X05:
                s = &bp;
                break;
            case 0X06:
                s = &si;
                break;
            case 0X07:
                s = &di;
                break;
            default:
                break;
            }
            if ((com & 0X02) == 0X00)
            {
                *s = *f;
                ip++;
                return;
            }
            else
            {
                *f = *s;
                ip++;
                return;
            }
        }
        else
        {
            REG_SIZE addr_sum;
            switch ((scnd & 0XC7))
            {
            case 0b00000000:
                addr_sum = bx + si;
                ip++;
                break;

            case 0b00000001:
                addr_sum = bx + di;
                ip++;
                break;

            case 0b00000010:
                addr_sum = bp + si;
                ip++;
                break;

            case 0b00000011:
                addr_sum = bp + di;
                ip++;
                break;

            case 0b00000100:
                addr_sum = si;
                ip++;
                break;

            case 0b00000101:
                addr_sum = di;
                ip++;
                break;

            case 0b00000110:
            {
                REG_SIZE l = memory[real_addr + 3];
                l = l << 8;
                l & 0XFF00;
                l = l | memory[real_addr + 2];
                addr_sum = l;
                ip += 3;
                break;
            }

            case 0b00000111:
                addr_sum = bx;
                ip++;
                break;

            case 0b01000000:
                addr_sum = bx + si + memory[real_addr + 2];
                ip += 2;
                break;

            case 0b01000001:
                addr_sum = bx + di + memory[real_addr + 2];
                ip += 2;
                break;

            case 0b01000010:
                addr_sum = bp + si + memory[real_addr + 2];
                ip += 2;
                break;

            case 0b01000011:
                addr_sum = bp + di + memory[real_addr + 2];
                ip += 2;
                break;

            case 0b01000100:
                addr_sum = si + memory[real_addr + 2];
                ip += 2;
                break;

            case 0b01000101:
                addr_sum = di + memory[real_addr + 2];
                ip += 2;
                break;

            case 0b01000110:
                addr_sum = bp + memory[real_addr + 2];
                ip += 2;
                break;

            case 0b01000111:
                addr_sum = bx + memory[real_addr + 2];
                ip += 2;
                break;

            case 0b10000000:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l & 0XFF00;
                l = l | memory[real_addr + 2];
                addr_sum = bx + si + l;
                ip += 2;
            }
            break;

            case 0b10000001:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l & 0XFF00;
                l = l | memory[real_addr + 2];
                addr_sum = bx + di + l;
                ip += 2;
            }
            break;

            case 0b10000010:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l & 0XFF00;
                l = l | memory[real_addr + 2];
                addr_sum = bp + si + l;
                ip += 2;
            }
            break;

            case 0b10000011:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l & 0XFF00;
                l = l | memory[real_addr + 2];
                addr_sum = bp + di + l;
                ip += 2;
            }
            break;

            case 0b10000100:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l & 0XFF00;
                l = l | memory[real_addr + 2];
                addr_sum = si + l;
                ip += 2;
            }
            break;

            case 0b10000101:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l & 0XFF00;
                l = l | memory[real_addr + 2];
                addr_sum = di + l;
                ip += 2;
            }
            break;
            case 0b10000110:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l & 0XFF00;
                l = l | memory[real_addr + 2];
                addr_sum = bp + l;
                ip += 2;
            }
            break;
            case 0b10000111:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l & 0XFF00;
                l = l | memory[real_addr + 2];
                addr_sum = bx + l;
                ip += 2;
            }
            break;
            default:
                break;
            }
            if ((com & 0X02) == 0X02)
            {
                REG_SIZE ds_s = ds;
                *f = 0;
                *f = memory[local_convert_cs_ip_to_real(ds_s, addr_sum) + 1];
                *f = (*f & 0X00FF) << 8;
                *f &= 0XFF00;
                *f |= memory[local_convert_cs_ip_to_real(ds_s, addr_sum)];
                return;
            }
            else
            {
                memory[local_convert_cs_ip_to_real(ds, addr_sum)] = *f & 0X00FF;
                memory[local_convert_cs_ip_to_real(ds, addr_sum) + 1] = (*f >> 8) & 0X00FF;
                return;
            }
        }
    }
}

void cp8086::push(BYTE com)
{
    sp -= 2;
    if (com == 0XFF) // mem/reg
    {
        BYTE scnd = memory[real_addr + 1];
        if ((scnd & 0XC0) == 0XC0) // reg to seg, seg to reg
        {
            ip++;
            switch (scnd & 0X07)
            {
            case 0X00:
                memory[local_convert_cs_ip_to_real(ss, sp) + 1] = ax >> 8;
                memory[local_convert_cs_ip_to_real(ss, sp)] = ax;
                break;
            case 0X01:
                memory[local_convert_cs_ip_to_real(ss, sp) + 1] = cx >> 8;
                memory[local_convert_cs_ip_to_real(ss, sp)] = cx;
                break;
            case 0X02:
                memory[local_convert_cs_ip_to_real(ss, sp) + 1] = dx >> 8;
                memory[local_convert_cs_ip_to_real(ss, sp)] = dx;
                break;
            case 0X03:
                memory[local_convert_cs_ip_to_real(ss, sp) + 1] = bx >> 8;
                memory[local_convert_cs_ip_to_real(ss, sp)] = bx;
                break;
            case 0X04:
                memory[local_convert_cs_ip_to_real(ss, sp) + 1] = sp >> 8;
                memory[local_convert_cs_ip_to_real(ss, sp)] = sp;
                break;
            case 0X05:
                memory[local_convert_cs_ip_to_real(ss, sp) + 1] = bp >> 8;
                memory[local_convert_cs_ip_to_real(ss, sp)] = bp;
                break;
            case 0X06:
                memory[local_convert_cs_ip_to_real(ss, sp) + 1] = si >> 8;
                memory[local_convert_cs_ip_to_real(ss, sp)] = si;
                break;
            case 0X07:
                memory[local_convert_cs_ip_to_real(ss, sp) + 1] = di >> 8;
                memory[local_convert_cs_ip_to_real(ss, sp)] = di;
                break;
            default:
                break;
            }
            return;
        }
        else
        {
            REG_SIZE addr_sum;
            switch ((scnd & 0XC7))
            {
            case 0b00000000:
                addr_sum = bx + si;
                ip++;
                break;

            case 0b00000001:
                addr_sum = bx + di;
                ip++;
                break;

            case 0b00000010:
                addr_sum = bp + si;
                ip++;
                break;

            case 0b00000011:
                addr_sum = bp + di;
                ip++;
                break;

            case 0b00000100:
                addr_sum = si;
                ip++;
                break;

            case 0b00000101:
                addr_sum = di;
                ip++;
                break;

            case 0b00000110:
            {
                REG_SIZE l = memory[real_addr + 3];
                l = l << 8;
                l & 0XFF00;
                l = l | memory[real_addr + 2];
                addr_sum = l;
                ip += 3;
                break;
            }

            case 0b00000111:
                addr_sum = bx;
                ip++;
                break;

            case 0b01000000:
                addr_sum = bx + si + memory[real_addr + 2];
                ip += 2;
                break;

            case 0b01000001:
                addr_sum = bx + di + memory[real_addr + 2];
                ip += 2;
                break;

            case 0b01000010:
                addr_sum = bp + si + memory[real_addr + 2];
                ip += 2;
                break;

            case 0b01000011:
                addr_sum = bp + di + memory[real_addr + 2];
                ip += 2;
                break;

            case 0b01000100:
                addr_sum = si + memory[real_addr + 2];
                ip += 2;
                break;

            case 0b01000101:
                addr_sum = di + memory[real_addr + 2];
                ip += 2;
                break;

            case 0b01000110:
                addr_sum = bp + memory[real_addr + 2];
                ip += 2;
                break;

            case 0b01000111:
                addr_sum = bx + memory[real_addr + 2];
                ip += 2;
                break;

            case 0b10000000:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l & 0XFF00;
                l = l | memory[real_addr + 2];
                addr_sum = bx + si + l;
                ip += 2;
            }
            break;

            case 0b10000001:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l & 0XFF00;
                l = l | memory[real_addr + 2];
                addr_sum = bx + di + l;
                ip += 2;
            }
            break;

            case 0b10000010:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l & 0XFF00;
                l = l | memory[real_addr + 2];
                addr_sum = bp + si + l;
                ip += 2;
            }
            break;

            case 0b10000011:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l & 0XFF00;
                l = l | memory[real_addr + 2];
                addr_sum = bp + di + l;
                ip += 2;
            }
            break;

            case 0b10000100:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l & 0XFF00;
                l = l | memory[real_addr + 2];
                addr_sum = si + l;
                ip += 2;
            }
            break;

            case 0b10000101:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l & 0XFF00;
                l = l | memory[real_addr + 2];
                addr_sum = di + l;
                ip += 2;
            }
            break;
            case 0b10000110:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l & 0XFF00;
                l = l | memory[real_addr + 2];
                addr_sum = bp + l;
                ip += 2;
            }
            break;
            case 0b10000111:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l & 0XFF00;
                l = l | memory[real_addr + 2];
                addr_sum = bx + l;
                ip += 2;
            }
            break;
            default:
                break;
            }
            memory[local_convert_cs_ip_to_real(ss, sp)] = memory[local_convert_cs_ip_to_real(ds, addr_sum)];
            memory[local_convert_cs_ip_to_real(ss, sp) + 1] = memory[local_convert_cs_ip_to_real(ds, addr_sum) + 1];
            return;
        }
    }

    if ((com & 0XF8) == 0b01010000) // reg
    {
        switch (com & 0X07)
        {
        case 0X00:
            memory[local_convert_cs_ip_to_real(ss, sp) + 1] = ax >> 8;
            memory[local_convert_cs_ip_to_real(ss, sp)] = ax;
            break;
        case 0X01:
            memory[local_convert_cs_ip_to_real(ss, sp) + 1] = cx >> 8;
            memory[local_convert_cs_ip_to_real(ss, sp)] = cx;
            break;
        case 0X02:
            memory[local_convert_cs_ip_to_real(ss, sp) + 1] = dx >> 8;
            memory[local_convert_cs_ip_to_real(ss, sp)] = dx;
            break;
        case 0X03:
            memory[local_convert_cs_ip_to_real(ss, sp) + 1] = bx >> 8;
            memory[local_convert_cs_ip_to_real(ss, sp)] = bx;
            break;
        case 0X04:
            memory[local_convert_cs_ip_to_real(ss, sp) + 1] = sp >> 8;
            memory[local_convert_cs_ip_to_real(ss, sp)] = sp;
            break;
        case 0X05:
            memory[local_convert_cs_ip_to_real(ss, sp) + 1] = bp >> 8;
            memory[local_convert_cs_ip_to_real(ss, sp)] = bp;
            break;
        case 0X06:
            memory[local_convert_cs_ip_to_real(ss, sp) + 1] = si >> 8;
            memory[local_convert_cs_ip_to_real(ss, sp)] = si;
            break;
        case 0X07:
            memory[local_convert_cs_ip_to_real(ss, sp) + 1] = di >> 8;
            memory[local_convert_cs_ip_to_real(ss, sp)] = di;
            break;
        default:
            break;
        }
        return;
    }

    if ((com & 0XE7) == 0b00000110) // seg
    {
        switch (com & 0X18)
        {
        case 0b00000000:
            memory[local_convert_cs_ip_to_real(ss, sp) + 1] = es >> 8;
            memory[local_convert_cs_ip_to_real(ss, sp)] = es;
            break;
        case 0b00001000:
            memory[local_convert_cs_ip_to_real(ss, sp) + 1] = cs >> 8;
            memory[local_convert_cs_ip_to_real(ss, sp)] = cs;
            break;
        case 0b00010000:
            memory[local_convert_cs_ip_to_real(ss, sp) + 1] = ss >> 8;
            memory[local_convert_cs_ip_to_real(ss, sp)] = ss;
            break;
        case 0b00011000:
            memory[local_convert_cs_ip_to_real(ss, sp) + 1] = ds >> 8;
            memory[local_convert_cs_ip_to_real(ss, sp)] = ds;
            break;
        default:
            break;
        }
    }
}

void cp8086::pop(BYTE com)
{
    if (com == 0X8F) // mem/reg
    {
        BYTE scnd = memory[real_addr + 1];
        if ((scnd & 0XC0) == 0XC0) // reg to seg, seg to reg
        {
            ip++;
            switch (scnd & 0X07)
            {
            case 0X00:
                ax = 0;
                ax = memory[local_convert_cs_ip_to_real(ss, sp) + 1];
                ax = (ax << 8) & 0XFF00;
                ax |= memory[local_convert_cs_ip_to_real(ss, sp)];
                break;
            case 0X01:
                cx = 0;
                cx = memory[local_convert_cs_ip_to_real(ss, sp) + 1];
                cx = (cx << 8) & 0XFF00;
                cx |= memory[local_convert_cs_ip_to_real(ss, sp)];
                break;
            case 0X02:
                dx = 0;
                dx = memory[local_convert_cs_ip_to_real(ss, sp) + 1];
                dx = (dx << 8) & 0XFF00;
                dx |= memory[local_convert_cs_ip_to_real(ss, sp)];
                break;
            case 0X03:
                bx = 0;
                bx = memory[local_convert_cs_ip_to_real(ss, sp) + 1];
                bx = (bx << 8) & 0XFF00;
                bx |= memory[local_convert_cs_ip_to_real(ss, sp)];
                break;
            case 0X04:
                sp = 0;
                sp = memory[local_convert_cs_ip_to_real(ss, sp) + 1];
                sp = (sp << 8) & 0XFF00;
                sp |= memory[local_convert_cs_ip_to_real(ss, sp)];
                break;
            case 0X05:
                bp = 0;
                bp = memory[local_convert_cs_ip_to_real(ss, sp) + 1];
                bp = (bp << 8) & 0XFF00;
                bp |= memory[local_convert_cs_ip_to_real(ss, sp)];
                break;
            case 0X06:
                si = 0;
                si = memory[local_convert_cs_ip_to_real(ss, sp) + 1];
                si = (si << 8) & 0XFF00;
                si |= memory[local_convert_cs_ip_to_real(ss, sp)];
                break;
            case 0X07:
                di = 0;
                di = memory[local_convert_cs_ip_to_real(ss, sp) + 1];
                di = (di << 8) & 0XFF00;
                di |= memory[local_convert_cs_ip_to_real(ss, sp)];
                break;
            default:
                break;
            }
            sp += 2;
            return;
        }
        else
        {
            REG_SIZE addr_sum;
            switch ((scnd & 0XC7))
            {
            case 0b00000000:
                addr_sum = bx + si;
                ip++;
                break;

            case 0b00000001:
                addr_sum = bx + di;
                ip++;
                break;

            case 0b00000010:
                addr_sum = bp + si;
                ip++;
                break;

            case 0b00000011:
                addr_sum = bp + di;
                ip++;
                break;

            case 0b00000100:
                addr_sum = si;
                ip++;
                break;

            case 0b00000101:
                addr_sum = di;
                ip++;
                break;

            case 0b00000110:
            {
                REG_SIZE l = memory[real_addr + 3];
                l = l << 8;
                l & 0XFF00;
                l = l | memory[real_addr + 2];
                addr_sum = l;
                ip += 3;
                break;
            }

            case 0b00000111:
                addr_sum = bx;
                ip++;
                break;

            case 0b01000000:
                addr_sum = bx + si + memory[real_addr + 2];
                ip += 2;
                break;

            case 0b01000001:
                addr_sum = bx + di + memory[real_addr + 2];
                ip += 2;
                break;

            case 0b01000010:
                addr_sum = bp + si + memory[real_addr + 2];
                ip += 2;
                break;

            case 0b01000011:
                addr_sum = bp + di + memory[real_addr + 2];
                ip += 2;
                break;

            case 0b01000100:
                addr_sum = si + memory[real_addr + 2];
                ip += 2;
                break;

            case 0b01000101:
                addr_sum = di + memory[real_addr + 2];
                ip += 2;
                break;

            case 0b01000110:
                addr_sum = bp + memory[real_addr + 2];
                ip += 2;
                break;

            case 0b01000111:
                addr_sum = bx + memory[real_addr + 2];
                ip += 2;
                break;

            case 0b10000000:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l & 0XFF00;
                l = l | memory[real_addr + 2];
                addr_sum = bx + si + l;
                ip += 2;
            }
            break;

            case 0b10000001:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l & 0XFF00;
                l = l | memory[real_addr + 2];
                addr_sum = bx + di + l;
                ip += 2;
            }
            break;

            case 0b10000010:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l & 0XFF00;
                l = l | memory[real_addr + 2];
                addr_sum = bp + si + l;
                ip += 2;
            }
            break;

            case 0b10000011:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l & 0XFF00;
                l = l | memory[real_addr + 2];
                addr_sum = bp + di + l;
                ip += 2;
            }
            break;

            case 0b10000100:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l & 0XFF00;
                l = l | memory[real_addr + 2];
                addr_sum = si + l;
                ip += 2;
            }
            break;

            case 0b10000101:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l & 0XFF00;
                l = l | memory[real_addr + 2];
                addr_sum = di + l;
                ip += 2;
            }
            break;
            case 0b10000110:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l & 0XFF00;
                l = l | memory[real_addr + 2];
                addr_sum = bp + l;
                ip += 2;
            }
            break;
            case 0b10000111:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l & 0XFF00;
                l = l | memory[real_addr + 2];
                addr_sum = bx + l;
                ip += 2;
            }
            break;
            default:
                break;
            }
            memory[local_convert_cs_ip_to_real(ds, addr_sum)] = memory[local_convert_cs_ip_to_real(ss, sp)];
            memory[local_convert_cs_ip_to_real(ds, addr_sum) + 1] = memory[local_convert_cs_ip_to_real(ss, sp) + 1];
            sp += 2;
            return;
        }
    }
}