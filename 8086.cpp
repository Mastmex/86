#include "8086.hpp"
#include <iostream>
#include <fstream>
#include <bitset>
#include <cstdio>

// comment this for disable command output
//---------------------------------------

#if DEBUG > 1
#define PRINT_COM(now, name) printf("%d - %s\n", now, name);
#else
#define PRINT_COM(now, name) return 0;
#endif

void cp8086::init()
{
    for (int i = 0; i < 1048576; i++)
        memory[i] = 0;
    ax.reg = bx.reg = cx.reg = dx.reg = 0;
    si.reg = di.reg = bp.reg = cs.reg = ds.reg = es.reg = 0;
    ss.reg = 0XF000;
    sp.reg = 0XFFFE;
    ip.reg = 0;
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
    ip.reg = input_addr & 0X0FFFF;
    cs.reg = (input_addr & 0XF0000) >> 4;
    std::cout << std::hex << cs.reg << '\n';
    std::cout << std::hex << ip.reg << '\n';
    return 1;
}

void cp8086::printStack()
{
    printf("FFFF:%02X\tFFFF:%02X\nFFFD:%02X\tFFFC:%02X\nFFFB:%02X\tFFFA:%02X\nFFF9:%02X\tFFF8:%02X\nFFF7:%02X\tFFF6:%02X\nFFF5:%02X\tFFF4:%02X\nFFF3:%02X\tFFF2:%02X\nFFF1:%02X\tFFF0:%02X\n",
           memory[local_convert_cs_ip_to_real(ss, {0XFFFF})],
           memory[local_convert_cs_ip_to_real(ss, {0XFFFE})],
           memory[local_convert_cs_ip_to_real(ss, {0XFFFD})],
           memory[local_convert_cs_ip_to_real(ss, {0XFFFC})],
           memory[local_convert_cs_ip_to_real(ss, {0XFFFB})],
           memory[local_convert_cs_ip_to_real(ss, {0XFFFA})],
           memory[local_convert_cs_ip_to_real(ss, {0XFFF9})],
           memory[local_convert_cs_ip_to_real(ss, {0XFFF8})],
           memory[local_convert_cs_ip_to_real(ss, {0XFFF7})],
           memory[local_convert_cs_ip_to_real(ss, {0XFFF6})],
           memory[local_convert_cs_ip_to_real(ss, {0XFFF5})],
           memory[local_convert_cs_ip_to_real(ss, {0XFFF4})],
           memory[local_convert_cs_ip_to_real(ss, {0XFFF3})],
           memory[local_convert_cs_ip_to_real(ss, {0XFFF2})],
           memory[local_convert_cs_ip_to_real(ss, {0XFFF1})],
           memory[local_convert_cs_ip_to_real(ss, {0XFFF0})]);
}

void cp8086::convert_cs_ip_to_real()
{
    real_addr = cs.reg * 16 + ip.reg;
    if (real_addr > 0XFFFFF)
        real_addr &= 0X0FFFFF;
}
REAL_ADDR_SIZE cp8086::local_convert_cs_ip_to_real(REGISTER c, REGISTER i)
{
    REAL_ADDR_SIZE r;
    r = c.reg * 16 + i.reg;
    if (r > 0XFFFFF)
        r &= 0X0FFFFF;
    return r;
}

REAL_ADDR_SIZE cp8086::local_convert_cs_ip_to_real(REGISTER c, REG_SIZE i)
{
    REAL_ADDR_SIZE r;
    r = c.reg * 16 + i;
    if (r > 0XFFFFF)
        r &= 0X0FFFFF;
    return r;
}

REAL_ADDR_SIZE cp8086::local_convert_cs_ip_to_real(REG_SIZE c, REG_SIZE i)
{
    REAL_ADDR_SIZE r;
    r = c * 16 + i;
    if (r > 0XFFFFF)
        r &= 0X0FFFFF;
    return r;
}

void cp8086::load_mem_hex(std::string name)
{
    std::ifstream mem(name, std::ios::binary);
    BYTE c;
    for (int i = 0;; i++)
    {
        mem.read((char *)&c, 1);
        memory[i] = c;
        if (mem.eof())
            break;
    }
    mem.close();
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

void cp8086::memory_dump()
{
    std::ofstream mem("mem.dump", std::ios::binary | std::ios::trunc);
    for (BYTE c : memory)
    {
        mem.write((char *)&c, 1);
    }
    mem.close();
}

void cp8086::printFlags()
{
    printf("\
    \tNT\tOF\tDF\tIF\tTF\tSF\tZF\tAF\tPF\tCF\n\
    \t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
           (flags & 0X4000) == 0X4000 ? 1 : 0,
           (flags & 0X0800) == 0X0800 ? 1 : 0,
           (flags & 0X0400) == 0X0400 ? 1 : 0,
           (flags & 0X0200) == 0X0200 ? 1 : 0,
           (flags & 0X0100) == 0X0100 ? 1 : 0,
           (flags & 0X0080) == 0X0080 ? 1 : 0,
           (flags & 0X0040) == 0X0040 ? 1 : 0,
           (flags & 0X0010) == 0X0010 ? 1 : 0,
           (flags & 0X0004) == 0X0004 ? 1 : 0,
           (flags & 0X0001) == 0X0001 ? 1 : 0);
}

void cp8086::printRegs()
{
    printf("\
            User-accessible registers\t\tSegment registers\n\
            AX:%04X\tSP:%04X\t\t\tES:%04X\n\
            BX:%04X\tBP:%04X\t\t\tCS:%04X\n\
            CX:%04X\tSI:%04X\t\t\tSS:%04X\n\
            DX:%04X\tDI:%04X\t\t\tDS:%04X\n",
           ax.reg, sp.reg, es.reg, bx.reg, bp.reg, cs.reg, cx.reg, si.reg, ss.reg, dx.reg, di.reg, ds.reg);
}

int cp8086::run()
{
    // std::cout<<real_addr<<"-----"<<std::bitset<8>{memory[real_addr+1]}<<'\n';
    while (1)
    {
        if (error == nullptr)
        {
            std::cout << "No error monitor set\n";
            return 1;
        }

        if (memory[real_addr] == 0b11110100)
        { /// HLT
            PRINT_COM(real_addr, "HLT");
            return 0;
        }

        switch (memory[real_addr])
        {
        case 0b10001000:
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
            case 0b10010000:
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
            case 0b00000111:
                PRINT_COM(real_addr, "POP");
                pop(memory[real_addr]);
                printStack();
                break;
            case 0b00001111:
                PRINT_COM(real_addr, "POP");
                pop(memory[real_addr]);
                printStack();
                break;
            case 0b00010111:
                PRINT_COM(real_addr, "POP");
                pop(memory[real_addr]);
                printStack();
                break;
            case 0b00011111:
                PRINT_COM(real_addr, "POP");
                pop(memory[real_addr]);
                printStack();
                break;
            }
            {
            case 0b10000110:
                PRINT_COM(real_addr, "XCHG");
                xchg(memory[real_addr]);
                break;
            case 0b10000111:
                PRINT_COM(real_addr, "XCHG");
                xchg(memory[real_addr]);
                break;
            case 0b10010001:
                PRINT_COM(real_addr, "XCHG");
                xchg(memory[real_addr]);
                break;
            case 0b10010010:
                PRINT_COM(real_addr, "XCHG");
                xchg(memory[real_addr]);
                break;
            case 0b10010011:
                PRINT_COM(real_addr, "XCHG");
                xchg(memory[real_addr]);
                break;
            case 0b10010100:
                PRINT_COM(real_addr, "XCHG");
                xchg(memory[real_addr]);
                break;
            case 0b10010101:
                PRINT_COM(real_addr, "XCHG");
                xchg(memory[real_addr]);
                break;
            case 0b10010110:
                PRINT_COM(real_addr, "XCHG");
                xchg(memory[real_addr]);
                break;
            case 0b10010111:
                PRINT_COM(real_addr, "XCHG");
                xchg(memory[real_addr]);
                break;
            }
            {
            case 0b10001101:
                PRINT_COM(real_addr, "LEA");
                lea(memory[real_addr]);
                break;
            case 0b11000101:
                PRINT_COM(real_addr, "LDS");
                lds(memory[real_addr]);
                break;
            case 0b11000100:
                PRINT_COM(real_addr, "LES");
                les(memory[real_addr]);
                break;
            case 0b10011111:
                PRINT_COM(real_addr, "LAHF");
                lahf(memory[real_addr]);
                break;
            case 0b10011110:
                PRINT_COM(real_addr, "SAHF");
                sahf(memory[real_addr]);
                break;
            case 0b10011100:
                PRINT_COM(real_addr, "PUSHF");
                pushf(memory[real_addr]);
                break;
            case 0b10011101:
                PRINT_COM(real_addr, "POPF");
                popf(memory[real_addr]);
                break;
            }
            {
            case 0b00000000:
                PRINT_COM(real_addr, "ADD");
                add(memory[real_addr]);
                printFlags();
                break;
            case 0b00000001:
                PRINT_COM(real_addr, "ADD");
                add(memory[real_addr]);
                printFlags();
                break;
            case 0b00000010:
                PRINT_COM(real_addr, "ADD");
                add(memory[real_addr]);
                printFlags();
                break;
            case 0b00000011:
                PRINT_COM(real_addr, "ADD");
                add(memory[real_addr]);
                printFlags();
                break;
            case 0b10000000:
                if ((memory[real_addr + 1] & 0X38) == 0b00000000) // maybe adc,sub, etc.
                {
                    PRINT_COM(real_addr, "ADD");
                    add(memory[real_addr]);
                    printFlags();
                    break;
                }
            case 0b10000001:
                if ((memory[real_addr + 1] & 0X38) == 0b00000000) // maybe adc,sub, etc.
                {
                    PRINT_COM(real_addr, "ADD");
                    add(memory[real_addr]);
                    printFlags();
                    break;
                }
            case 0b10000011:
                if ((memory[real_addr + 1] & 0X38) == 0b00000000) // maybe adc,sub, etc.
                {
                    PRINT_COM(real_addr, "ADD");
                    add(memory[real_addr]);
                    printFlags();
                    break;
                }
            }
            {
            case 0b00010000:
                PRINT_COM(real_addr, "ADC");
                adc(memory[real_addr]);
                printFlags();
                break;
            case 0b00010001:
                PRINT_COM(real_addr, "ADC");
                adc(memory[real_addr]);
                printFlags();
                break;
            case 0b00010010:
                PRINT_COM(real_addr, "ADC");
                adc(memory[real_addr]);
                printFlags();
                break;
            case 0b00010011:
                PRINT_COM(real_addr, "ADC");
                adc(memory[real_addr]);
                printFlags();
                break;
            }
            {
            case 0b11110110:
                if ((memory[real_addr + 1] & 0X38) == 0b00100000) // maybe imul/div/idiv
                {
                    PRINT_COM(real_addr, "MUL");
                    mul(memory[real_addr]);
                    printFlags();
                }
                if ((memory[real_addr + 1] & 0X38) == 0b00101000)
                {
                    PRINT_COM(real_addr, "IMUL");
                    imul(memory[real_addr]);
                    printFlags();
                }
                break;
            case 0b11110111:
                if ((memory[real_addr + 1] & 0X38) == 0b00100000) // maybe imul/div/idiv
                {
                    PRINT_COM(real_addr, "MUL");
                    mul(memory[real_addr]);
                    printFlags();
                }
                if ((memory[real_addr + 1] & 0X38) == 0b00101000)
                {
                    PRINT_COM(real_addr, "IMUL");
                    imul(memory[real_addr]);
                    printFlags();
                }
                break;
            }
            {
            case 0b00111000:
                PRINT_COM(real_addr, "CMP");
                cmp(memory[real_addr]);
                printFlags();
                break;
            case 0b00111001:
                PRINT_COM(real_addr, "CMP");
                cmp(memory[real_addr]);
                printFlags();
                break;
            case 0b00111010:
                PRINT_COM(real_addr, "CMP");
                cmp(memory[real_addr]);
                printFlags();
                break;
            case 0b00111011:
                PRINT_COM(real_addr, "CMP");
                cmp(memory[real_addr]);
                printFlags();
                break;
            }
            { // jmp
            case 0b01111111:
                PRINT_COM(real_addr, "JG");
                jmp(memory[real_addr]);
                printFlags();
                break;
            case 0b11100010:
                PRINT_COM(real_addr, "LOOP");
                jmp(memory[real_addr]);
                printFlags();
                break;
            case 0b11101011:
                PRINT_COM(real_addr, "JMP"); // jmp short
                jmp(memory[real_addr]);
                printFlags();
                break;
            case 0b11101000:
                PRINT_COM(real_addr, "CALL"); // jmp short
                call(memory[real_addr]);
                break;
            case 0b11000011:
                PRINT_COM(real_addr, "RET"); // jmp short
                ret(memory[real_addr]);
                break;
            }
        default:
            return 1;
            break;
        }
        if (ip.reg == 0XFFFF)
        {
            cs.reg = cs.reg + 1;
            ip.reg = 0;
        }
        ip.reg = ip.reg + 1;
        convert_cs_ip_to_real();
        // printFlags();
        printRegs();
        printf("%04X:%04X\n", cs.reg, ip.reg);
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

void cp8086::flagPF(int s)
{
    if (s == 0)
        flags &= 0XFFFB;
    if (s == 1)
        flags |= 0X0004;
    if (s == 2)
        flags = flags ^ 0X0004;
}

void cp8086::flagAF(int s)
{
    if (s == 0)
        flags &= 0XFFEF;
    if (s == 1)
        flags |= 0X0010;
    if (s == 2)
        flags ^= 0X0010;
}

void cp8086::flagZF(int s)
{
    if (s == 0)
        flags &= 0XFFBF;
    if (s == 1)
        flags |= 0X0040;
    if (s == 2)
        flags ^= 0X0040;
}

void cp8086::flagSF(int s)
{
    if (s == 0)
        flags &= 0XFF7F;
    if (s == 1)
        flags |= 0X0080;
    if (s == 2)
        flags ^= 0X0080;
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

void cp8086::flagOF(int s)
{
    if (s == 0)
        flags &= 0XF7FF;
    if (s == 1)
        flags |= 0X0800;
    if (s == 2)
        flags = flags ^ 0X0800;
}

void cp8086::mov(BYTE com)
{
    if ((com & 0XFC) == 0b10001000) // reg to mem, mem to reg, reg to reg
    {
        BYTE w = com & 0X01;
        BYTE scnd = memory[real_addr + 1];
        if ((scnd & 0XC0) == 0XC0) // reg to reg
        {
            ip.reg++;
            if (w == 1)
            {
                REGISTER *f, *s;
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
                int h1, h2;
                REGISTER *f, *s;
                switch ((scnd & 0X38))
                {
                case 0b00000000:
                    h1 = 0;
                    f = &ax;
                    break;

                case 0b00001000:
                    h1 = 0;
                    f = &cx;
                    break;

                case 0b00010000:
                    h1 = 0;
                    f = &dx;
                    break;

                case 0b00011000:
                    h1 = 0;
                    f = &bx;
                    break;

                case 0b00100000:
                    h1 = 1;
                    f = &ax;
                    break;

                case 0b00101000:
                    h1 = 1;
                    f = &cx;
                    break;

                case 0b00110000:
                    h1 = 1;
                    f = &dx;
                    break;

                case 0b00111000:
                    h1 = 1;
                    f = &bx;
                    break;
                }
                switch ((scnd & 0X07))
                {
                case 0b00000000:
                    h2 = 0;
                    s = &ax;
                    break;

                case 0b00000001:
                    h2 = 0;
                    s = &cx;
                    break;

                case 0b00000010:
                    h2 = 0;
                    s = &dx;
                    break;

                case 0b00000011:
                    h2 = 0;
                    s = &bx;
                    break;

                case 0b00000100:
                    h2 = 1;
                    s = &ax;
                    break;

                case 0b00000101:
                    h2 = 1;
                    s = &cx;
                    break;

                case 0b00000110:
                    h2 = 1;
                    s = &dx;
                    break;

                case 0b00000111:
                    h2 = 1;
                    s = &bx;
                    break;
                }
                if ((com & 0X02) == 0X02)
                {
                    f->reg_half[h1] = s->reg_half[h2];
                }
                else
                {
                    s->reg_half[h2] = f->reg_half[h1];
                }
                return;
            }
        }
        else // reg to mem, mem to reg
        {
            REGISTER *reg;
            REG_SIZE addr_sum;
            switch ((scnd & 0XC7))
            {
            case 0b00000000:
                addr_sum = bx.reg + si.reg;
                ip.reg++;
                break;

            case 0b00000001:
                addr_sum = bx.reg + di.reg;
                ip.reg++;
                break;

            case 0b00000010:
                addr_sum = bp.reg + si.reg;
                ip.reg++;
                break;

            case 0b00000011:
                addr_sum = bp.reg + di.reg;
                ip.reg++;
                break;

            case 0b00000100:
                addr_sum = si.reg;
                ip.reg++;
                break;

            case 0b00000101:
                addr_sum = di.reg;
                ip.reg++;
                break;

            case 0b00000110:
            {
                REG_SIZE l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = l;
                ip.reg += 3;
                break;
            }

            case 0b00000111:
                addr_sum = bx.reg;
                ip.reg++;
                break;

            case 0b01000000:
                addr_sum = bx.reg + si.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000001:
                addr_sum = bx.reg + di.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000010:
                addr_sum = bp.reg + si.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000011:
                addr_sum = bp.reg + di.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000100:
                addr_sum = si.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000101:
                addr_sum = di.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000110:
                addr_sum = bp.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000111:
                addr_sum = bx.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b10000000:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = bx.reg + si.reg + l;
                ip.reg += 3;
            }
            break;

            case 0b10000001:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = bx.reg + di.reg + l;
                ip.reg += 3;
            }
            break;

            case 0b10000010:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = bp.reg + si.reg + l;
                ip.reg += 3;
            }
            break;

            case 0b10000011:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = bp.reg + di.reg + l;
                ip.reg += 3;
            }
            break;

            case 0b10000100:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = si.reg + l;
                ip.reg += 3;
            }
            break;

            case 0b10000101:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = di.reg + l;
                ip.reg += 3;
            }
            break;
            case 0b10000110:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = bp.reg + l;
                ip.reg += 3;
            }
            break;
            case 0b10000111:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = bx.reg + l;
                ip.reg += 3;
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
                    memory[local_convert_cs_ip_to_real(ds, addr_sum) + 1] = reg->reg_half[1];
                    memory[local_convert_cs_ip_to_real(ds, addr_sum)] = reg->reg_half[0];
                }
                else
                {
                    reg->reg_half[0] = memory[local_convert_cs_ip_to_real(ds, addr_sum)];
                    reg->reg_half[1] = memory[local_convert_cs_ip_to_real(ds, addr_sum + 1)];
                }
            }
            else
            {
                int h = 0;
                switch ((scnd & 0X38))
                {
                case 0b00000000:
                    reg = &ax;
                    h = 0;
                    break;

                case 0b00001000:
                    reg = &cx;
                    h = 0;
                    break;

                case 0b00010000:
                    h = 0;
                    reg = &dx;
                    break;

                case 0b00011000:
                    h = 0;
                    reg = &bx;
                    break;

                case 0b00100000:
                    h = 1;
                    reg = &ax;
                    break;

                case 0b00101000:
                    h = 1;
                    reg = &cx;
                    break;

                case 0b00110000:
                    h = 1;
                    reg = &dx;
                    break;

                case 0b00111000:
                    h = 1;
                    reg = &bx;
                    break;
                }

                if ((com & 0X02) == 0X00)
                {
                    memory[local_convert_cs_ip_to_real(ds, addr_sum)] = reg->reg_half[h];
                }
                else
                {
                    reg->reg_half[h] = memory[local_convert_cs_ip_to_real(ds, addr_sum)];
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
            addr_sum = bx.reg + si.reg;
            ip.reg++;
            break;

        case 0b00000001:
            addr_sum = bx.reg + di.reg;
            ip.reg++;
            break;

        case 0b00000010:
            addr_sum = bp.reg + si.reg;
            ip.reg++;
            break;

        case 0b00000011:
            addr_sum = bp.reg + di.reg;
            ip.reg++;
            break;

        case 0b00000100:
            addr_sum = si.reg;
            ip.reg++;
            break;

        case 0b00000101:
            addr_sum = di.reg;
            ip.reg++;
            break;

        case 0b00000110:
        {
            REG_SIZE l = memory[real_addr + 3];
            l = l << 8;

            l = l | memory[real_addr + 2];
            addr_sum = l;
            ip.reg += 3;
            break;
        }

        case 0b00000111:
            addr_sum = bx.reg;
            ip.reg++;
            break;

        case 0b01000000:
            addr_sum = bx.reg + si.reg + memory[real_addr + 2];
            ip.reg += 2;
            break;

        case 0b01000001:
            addr_sum = bx.reg + di.reg + memory[real_addr + 2];
            ip.reg += 2;
            break;

        case 0b01000010:
            addr_sum = bp.reg + si.reg + memory[real_addr + 2];
            ip.reg += 2;
            break;

        case 0b01000011:
            addr_sum = bp.reg + di.reg + memory[real_addr + 2];
            ip.reg += 2;
            break;

        case 0b01000100:
            addr_sum = si.reg + memory[real_addr + 2];
            ip.reg += 2;
            break;

        case 0b01000101:
            addr_sum = di.reg + memory[real_addr + 2];
            ip.reg += 2;
            break;

        case 0b01000110:
            addr_sum = bp.reg + memory[real_addr + 2];
            ip.reg += 2;
            break;

        case 0b01000111:
            addr_sum = bx.reg + memory[real_addr + 2];
            ip.reg += 2;
            break;

        case 0b10000000:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;

            l = l | memory[real_addr + 2];
            addr_sum = bx.reg + si.reg + l;
            ip.reg += 3;
        }
        break;

        case 0b10000001:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;

            l = l | memory[real_addr + 2];
            addr_sum = bx.reg + di.reg + l;
            ip.reg += 3;
        }
        break;

        case 0b10000010:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;

            l = l | memory[real_addr + 2];
            addr_sum = bp.reg + si.reg + l;
            ip.reg += 3;
        }
        break;

        case 0b10000011:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;

            l = l | memory[real_addr + 2];
            addr_sum = bp.reg + di.reg + l;
            ip.reg += 3;
        }
        break;

        case 0b10000100:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;

            l = l | memory[real_addr + 2];
            addr_sum = si.reg + l;
            ip.reg += 3;
        }
        break;

        case 0b10000101:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;

            l = l | memory[real_addr + 2];
            addr_sum = di.reg + l;
            ip.reg += 3;
        }
        break;
        case 0b10000110:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;

            l = l | memory[real_addr + 2];
            addr_sum = bp.reg + l;
            ip.reg += 3;
        }
        break;
        case 0b10000111:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;

            l = l | memory[real_addr + 2];
            addr_sum = bx.reg + l;
            ip.reg += 3;
        }
        break;
        default:
            break;
        }
        if (w == 0)
        {
            memory[local_convert_cs_ip_to_real(ds, addr_sum)] = memory[local_convert_cs_ip_to_real(cs, ip) + 1];
            ip.reg++;
            return;
        }
        else
        {
            memory[local_convert_cs_ip_to_real(ds, addr_sum) + 1] = memory[local_convert_cs_ip_to_real(cs, ip) + 2];
            memory[local_convert_cs_ip_to_real(ds, addr_sum)] = memory[local_convert_cs_ip_to_real(cs, ip) + 1];
            ip.reg += 2;
            return;
        }
    }

    if ((com & 0XF0) == 0b10110000) // data to reg
    {
        BYTE w = (com & 0X08) >> 3;
        if (w == 1)
        {
            REGISTER *r;
            switch ((com & 0X07))
            {
            case 0b00000000:
                r = &ax;
                break;
            case 0b00000001:
                r = &cx;
                break;
            case 0b00000010:
                r = &dx;
                break;
            case 0b00000011:
                r = &bx;
                break;
            case 0b00000100:
                r = &sp;
                break;
            case 0b00000101:
                r = &bp;
                break;
            case 0b00000110:
                r = &si;
                break;
            case 0b00000111:
                r = &di;
                break;
            default:
                break;
            }
            r->reg_half[1] = memory[real_addr + 2];
            r->reg_half[0] = memory[real_addr + 1];
            ip.reg += 2;
            return;
        }
        else
        {
            REGISTER *r;
            int h = 0;
            switch ((com & 0X07))
            {
            case 0b00000000:
                h = 0;
                r = &ax;
                break;
            case 0b00000001:
                h = 0;
                r = &cx;
                break;
            case 0b00000010:
                h = 0;
                r = &dx;
                break;
            case 0b00000011:
                h = 0;
                r = &bx;
                break;
            case 0b00000100:
                h = 1;
                r = &ax;
                break;
            case 0b00000101:
                h = 1;
                r = &cx;
                break;
            case 0b00000110:
                h = 1;
                r = &dx;
                break;
            case 0b00000111:
                h = 1;
                r = &bx;
                break;
            default:
                break;
            }
            r->reg_half[h] = memory[real_addr + 1];
            ip.reg += 1;
            return;
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
                ax.reg_half[0] = memory[local_convert_cs_ip_to_real(ds, memory[real_addr + 1])];
                ip.reg++;
                return;
            }
            else
            {
                memory[local_convert_cs_ip_to_real(ds, memory[real_addr + 1])] = ax.reg_half[0];
                ip.reg++;
                return;
            }
        }
        else
        {
            REGISTER addr_to_write;
            addr_to_write.reg_half[0] = memory[real_addr + 1];
            addr_to_write.reg_half[1] = memory[real_addr + 2];
            if (d == 0)
            {
                ax.reg_half[1] = memory[local_convert_cs_ip_to_real(ds, addr_to_write)];
                ax.reg_half[0] = memory[local_convert_cs_ip_to_real(ds, addr_to_write.reg + 1)];
                ip.reg += 2;
                return;
            }
            else
            {
                memory[local_convert_cs_ip_to_real(ds, addr_to_write)] = ax.reg_half[0];
                memory[local_convert_cs_ip_to_real(ds, addr_to_write.reg + 1)] = ax.reg_half[1];
                ip.reg += 2;
                return;
            }
        }
    }

    if ((com & 0XFD) == 0b10001100) // mem/reg to seg, seg to mem/reg
    {
        BYTE scnd = memory[real_addr + 1];
        REGISTER *f;
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
            REGISTER *s;
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
                ip.reg++;
                return;
            }
            else
            {
                *f = *s;
                ip.reg++;
                return;
            }
        }
        else
        {
            REG_SIZE addr_sum;
            switch ((scnd & 0XC7))
            {
            case 0b00000000:
                addr_sum = bx.reg + si.reg;
                ip.reg++;
                break;

            case 0b00000001:
                addr_sum = bx.reg + di.reg;
                ip.reg++;
                break;

            case 0b00000010:
                addr_sum = bp.reg + si.reg;
                ip.reg++;
                break;

            case 0b00000011:
                addr_sum = bp.reg + di.reg;
                ip.reg++;
                break;

            case 0b00000100:
                addr_sum = si.reg;
                ip.reg++;
                break;

            case 0b00000101:
                addr_sum = di.reg;
                ip.reg++;
                break;

            case 0b00000110:
            {
                REG_SIZE l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = l;
                ip.reg += 3;
                break;
            }

            case 0b00000111:
                addr_sum = bx.reg;
                ip.reg++;
                break;

            case 0b01000000:
                addr_sum = bx.reg + si.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000001:
                addr_sum = bx.reg + di.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000010:
                addr_sum = bp.reg + si.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000011:
                addr_sum = bp.reg + di.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000100:
                addr_sum = si.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000101:
                addr_sum = di.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000110:
                addr_sum = bp.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000111:
                addr_sum = bx.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b10000000:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = bx.reg + si.reg + l;
                ip.reg += 3;
            }
            break;

            case 0b10000001:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = bx.reg + di.reg + l;
                ip.reg += 3;
            }
            break;

            case 0b10000010:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = bp.reg + si.reg + l;
                ip.reg += 3;
            }
            break;

            case 0b10000011:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = bp.reg + di.reg + l;
                ip.reg += 3;
            }
            break;

            case 0b10000100:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = si.reg + l;
                ip.reg += 3;
            }
            break;

            case 0b10000101:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = di.reg + l;
                ip.reg += 3;
            }
            break;
            case 0b10000110:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = bp.reg + l;
                ip.reg += 3;
            }
            break;
            case 0b10000111:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = bx.reg + l;
                ip.reg += 3;
            }
            break;
            }
            if ((com & 0X02) == 0X02)
            {
                REG_SIZE ds_s = ds.reg;
                f->reg_half[1] = memory[local_convert_cs_ip_to_real(ds_s, addr_sum) + 1];
                f->reg_half[0] = memory[local_convert_cs_ip_to_real(ds_s, addr_sum)];

                return;
            }
            else
            {
                memory[local_convert_cs_ip_to_real(ds, addr_sum)] = f->reg_half[0];
                memory[local_convert_cs_ip_to_real(ds, addr_sum) + 1] = f->reg_half[1];
                return;
            }
        }
    }
}

void cp8086::push(BYTE com)
{
    sp.reg -= 2;
    if (com == 0XFF) // mem/reg
    {
        BYTE scnd = memory[real_addr + 1];
        if ((scnd & 0XC0) == 0XC0) // reg to seg, seg to reg
        {
            ip.reg++;
            switch (scnd & 0X07)
            {
            case 0X00:
                memory[local_convert_cs_ip_to_real(ss, sp) + 1] = ax.reg_half[1];
                memory[local_convert_cs_ip_to_real(ss, sp)] = ax.reg_half[0];
                break;
            case 0X01:
                memory[local_convert_cs_ip_to_real(ss, sp) + 1] = cx.reg_half[1];
                memory[local_convert_cs_ip_to_real(ss, sp)] = cx.reg_half[0];
                break;
            case 0X02:
                memory[local_convert_cs_ip_to_real(ss, sp) + 1] = dx.reg_half[1];
                memory[local_convert_cs_ip_to_real(ss, sp)] = dx.reg_half[0];
                break;
            case 0X03:
                memory[local_convert_cs_ip_to_real(ss, sp) + 1] = bx.reg_half[1];
                memory[local_convert_cs_ip_to_real(ss, sp)] = bx.reg_half[0];
                break;
            case 0X04:
                memory[local_convert_cs_ip_to_real(ss, sp) + 1] = sp.reg_half[1];
                memory[local_convert_cs_ip_to_real(ss, sp)] = sp.reg_half[0];
                break;
            case 0X05:
                memory[local_convert_cs_ip_to_real(ss, sp) + 1] = bp.reg_half[1];
                memory[local_convert_cs_ip_to_real(ss, sp)] = bp.reg_half[0];
                break;
            case 0X06:
                memory[local_convert_cs_ip_to_real(ss, sp) + 1] = si.reg_half[1];
                memory[local_convert_cs_ip_to_real(ss, sp)] = si.reg_half[0];
                break;
            case 0X07:
                memory[local_convert_cs_ip_to_real(ss, sp) + 1] = di.reg_half[1];
                memory[local_convert_cs_ip_to_real(ss, sp)] = di.reg_half[0];
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
                addr_sum = bx.reg + si.reg;
                ip.reg++;
                break;

            case 0b00000001:
                addr_sum = bx.reg + di.reg;
                ip.reg++;
                break;

            case 0b00000010:
                addr_sum = bp.reg + si.reg;
                ip.reg++;
                break;

            case 0b00000011:
                addr_sum = bp.reg + di.reg;
                ip.reg++;
                break;

            case 0b00000100:
                addr_sum = si.reg;
                ip.reg++;
                break;

            case 0b00000101:
                addr_sum = di.reg;
                ip.reg++;
                break;

            case 0b00000110:
            {

                REG_SIZE l = memory[real_addr + 3];
                l = l << 8;
                l = l | memory[real_addr + 2];
                addr_sum = l;
                ip.reg += 3;
                break;
            }

            case 0b00000111:
                addr_sum = bx.reg;
                ip.reg++;
                break;

            case 0b01000000:
                addr_sum = bx.reg + si.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000001:
                addr_sum = bx.reg + di.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000010:
                addr_sum = bp.reg + si.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000011:
                addr_sum = bp.reg + di.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000100:
                addr_sum = si.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000101:
                addr_sum = di.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000110:
                addr_sum = bp.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000111:
                addr_sum = bx.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b10000000:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l = l | memory[real_addr + 2];
                addr_sum = bx.reg + si.reg + l;
                ip.reg += 3;
            }
            break;

            case 0b10000001:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l = l | memory[real_addr + 2];
                addr_sum = bx.reg + di.reg + l;
                ip.reg += 3;
            }
            break;

            case 0b10000010:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l = l | memory[real_addr + 2];
                addr_sum = bp.reg + si.reg + l;
                ip.reg += 3;
            }
            break;

            case 0b10000011:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l = l | memory[real_addr + 2];
                addr_sum = bp.reg + di.reg + l;
                ip.reg += 3;
            }
            break;

            case 0b10000100:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l = l | memory[real_addr + 2];
                addr_sum = si.reg + l;
                ip.reg += 3;
            }
            break;

            case 0b10000101:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l = l | memory[real_addr + 2];
                addr_sum = di.reg + l;
                ip.reg += 3;
            }
            break;
            case 0b10000110:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l = l | memory[real_addr + 2];
                addr_sum = bp.reg + l;
                ip.reg += 3;
            }
            break;
            case 0b10000111:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l = l | memory[real_addr + 2];
                addr_sum = bx.reg + l;
                ip.reg += 3;
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
            memory[local_convert_cs_ip_to_real(ss, sp) + 1] = ax.reg_half[1];
            memory[local_convert_cs_ip_to_real(ss, sp)] = ax.reg_half[0];
            break;
        case 0X01:
            memory[local_convert_cs_ip_to_real(ss, sp) + 1] = cx.reg_half[1];
            memory[local_convert_cs_ip_to_real(ss, sp)] = cx.reg_half[0];
            break;
        case 0X02:
            memory[local_convert_cs_ip_to_real(ss, sp) + 1] = dx.reg_half[1];
            memory[local_convert_cs_ip_to_real(ss, sp)] = dx.reg_half[0];
            break;
        case 0X03:
            memory[local_convert_cs_ip_to_real(ss, sp) + 1] = bx.reg_half[1];
            memory[local_convert_cs_ip_to_real(ss, sp)] = bx.reg_half[0];
            break;
        case 0X04:
            memory[local_convert_cs_ip_to_real(ss, sp) + 1] = sp.reg_half[1];
            memory[local_convert_cs_ip_to_real(ss, sp)] = sp.reg_half[0];
            break;
        case 0X05:
            memory[local_convert_cs_ip_to_real(ss, sp) + 1] = bp.reg_half[1];
            memory[local_convert_cs_ip_to_real(ss, sp)] = bp.reg_half[0];
            break;
        case 0X06:
            memory[local_convert_cs_ip_to_real(ss, sp) + 1] = si.reg_half[1];
            memory[local_convert_cs_ip_to_real(ss, sp)] = si.reg_half[0];
            break;
        case 0X07:
            memory[local_convert_cs_ip_to_real(ss, sp) + 1] = di.reg_half[1];
            memory[local_convert_cs_ip_to_real(ss, sp)] = di.reg_half[0];
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
            memory[local_convert_cs_ip_to_real(ss, sp) + 1] = es.reg_half[1];
            memory[local_convert_cs_ip_to_real(ss, sp)] = es.reg_half[0];
            break;
        case 0b00001000:
            memory[local_convert_cs_ip_to_real(ss, sp) + 1] = cs.reg_half[1];
            memory[local_convert_cs_ip_to_real(ss, sp)] = cs.reg_half[0];
            break;
        case 0b00010000:
            memory[local_convert_cs_ip_to_real(ss, sp) + 1] = ss.reg_half[1];
            memory[local_convert_cs_ip_to_real(ss, sp)] = ss.reg_half[0];
            break;
        case 0b00011000:
            memory[local_convert_cs_ip_to_real(ss, sp) + 1] = ds.reg_half[1];
            memory[local_convert_cs_ip_to_real(ss, sp)] = ds.reg_half[0];
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
            ip.reg++;
            switch (scnd & 0X07)
            {
            case 0X00:
                ax.reg_half[1] = memory[local_convert_cs_ip_to_real(ss, sp) + 1];
                ax.reg_half[0] = memory[local_convert_cs_ip_to_real(ss, sp)];
                break;
            case 0X01:
                cx.reg_half[1] = memory[local_convert_cs_ip_to_real(ss, sp) + 1];
                cx.reg_half[0] = memory[local_convert_cs_ip_to_real(ss, sp)];
                break;
            case 0X02:
                dx.reg_half[1] = memory[local_convert_cs_ip_to_real(ss, sp) + 1];
                dx.reg_half[0] = memory[local_convert_cs_ip_to_real(ss, sp)];
                break;
            case 0X03:
                bx.reg_half[1] = memory[local_convert_cs_ip_to_real(ss, sp) + 1];
                bx.reg_half[0] = memory[local_convert_cs_ip_to_real(ss, sp)];
                break;
            case 0X04:
            {
                REG_SIZE tmp = sp.reg;
                sp.reg_half[1] = memory[local_convert_cs_ip_to_real(ss, tmp) + 1];
                sp.reg_half[0] = memory[local_convert_cs_ip_to_real(ss, tmp)];
            }
            break;
            case 0X05:
                bp.reg_half[1] = memory[local_convert_cs_ip_to_real(ss, sp) + 1];
                bp.reg_half[0] = memory[local_convert_cs_ip_to_real(ss, sp)];
                break;
            case 0X06:
                si.reg_half[1] = memory[local_convert_cs_ip_to_real(ss, sp) + 1];
                si.reg_half[0] = memory[local_convert_cs_ip_to_real(ss, sp)];
                break;
            case 0X07:
                di.reg_half[1] = memory[local_convert_cs_ip_to_real(ss, sp) + 1];
                di.reg_half[0] = memory[local_convert_cs_ip_to_real(ss, sp)];
                break;
            default:
                break;
            }
            sp.reg += 2;
            return;
        }
        else
        {
            REG_SIZE addr_sum;
            switch ((scnd & 0XC7))
            {
            case 0b00000000:
                addr_sum = bx.reg + si.reg;
                ip.reg++;
                break;

            case 0b00000001:
                addr_sum = bx.reg + di.reg;
                ip.reg++;
                break;

            case 0b00000010:
                addr_sum = bp.reg + si.reg;
                ip.reg++;
                break;

            case 0b00000011:
                addr_sum = bp.reg + di.reg;
                ip.reg++;
                break;

            case 0b00000100:
                addr_sum = si.reg;
                ip.reg++;
                break;

            case 0b00000101:
                addr_sum = di.reg;
                ip.reg++;
                break;

            case 0b00000110:
            {
                REG_SIZE l = memory[real_addr + 3];
                l = l << 8;
                l = l | memory[real_addr + 2];
                addr_sum = l;
                ip.reg += 3;
                break;
            }

            case 0b00000111:
                addr_sum = bx.reg;
                ip.reg++;
                break;

            case 0b01000000:
                addr_sum = bx.reg + si.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000001:
                addr_sum = bx.reg + di.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000010:
                addr_sum = bp.reg + si.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000011:
                addr_sum = bp.reg + di.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000100:
                addr_sum = si.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000101:
                addr_sum = di.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000110:
                addr_sum = bp.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000111:
                addr_sum = bx.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b10000000:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l = l | memory[real_addr + 2];
                addr_sum = bx.reg + si.reg + l;
                ip.reg += 3;
            }
            break;

            case 0b10000001:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l = l | memory[real_addr + 2];
                addr_sum = bx.reg + di.reg + l;
                ip.reg += 3;
            }
            break;

            case 0b10000010:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l = l | memory[real_addr + 2];
                addr_sum = bp.reg + si.reg + l;
                ip.reg += 3;
            }
            break;

            case 0b10000011:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l = l | memory[real_addr + 2];
                addr_sum = bp.reg + di.reg + l;
                ip.reg += 3;
            }
            break;

            case 0b10000100:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l = l | memory[real_addr + 2];
                addr_sum = si.reg + l;
                ip.reg += 3;
            }
            break;

            case 0b10000101:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l = l | memory[real_addr + 2];
                addr_sum = di.reg + l;
                ip.reg += 3;
            }
            break;
            case 0b10000110:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l = l | memory[real_addr + 2];
                addr_sum = bp.reg + l;
                ip.reg += 3;
            }
            break;
            case 0b10000111:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;
                l = l | memory[real_addr + 2];
                addr_sum = bx.reg + l;
                ip.reg += 3;
            }
            break;
            default:
                break;
            }
            memory[local_convert_cs_ip_to_real(ds, addr_sum)] = memory[local_convert_cs_ip_to_real(ss, sp)];
            memory[local_convert_cs_ip_to_real(ds, addr_sum) + 1] = memory[local_convert_cs_ip_to_real(ss, sp) + 1];
            sp.reg += 2;
            return;
        }
    }

    if ((com & 0XF8) == 0b01011000) // reg
    {
        switch (com & 0X07)
        {
        case 0X00:
            ax.reg_half[1] = memory[local_convert_cs_ip_to_real(ss, sp) + 1];
            ax.reg_half[0] = memory[local_convert_cs_ip_to_real(ss, sp)];
            break;
        case 0X01:
            cx.reg_half[1] = memory[local_convert_cs_ip_to_real(ss, sp) + 1];
            cx.reg_half[0] = memory[local_convert_cs_ip_to_real(ss, sp)];
            break;
        case 0X02:
            dx.reg_half[1] = memory[local_convert_cs_ip_to_real(ss, sp) + 1];
            dx.reg_half[0] = memory[local_convert_cs_ip_to_real(ss, sp)];
            break;
        case 0X03:
            bx.reg_half[1] = memory[local_convert_cs_ip_to_real(ss, sp) + 1];
            bx.reg_half[0] = memory[local_convert_cs_ip_to_real(ss, sp)];
            break;
        case 0X04:
        {
            REG_SIZE tmp = sp.reg;
            sp.reg_half[1] = memory[local_convert_cs_ip_to_real(ss, tmp) + 1];
            sp.reg_half[0] = memory[local_convert_cs_ip_to_real(ss, tmp)];
        }
        break;
        case 0X05:
            bp.reg_half[1] = memory[local_convert_cs_ip_to_real(ss, sp) + 1];
            bp.reg_half[0] = memory[local_convert_cs_ip_to_real(ss, sp)];
            break;
        case 0X06:
            si.reg_half[1] = memory[local_convert_cs_ip_to_real(ss, sp) + 1];
            si.reg_half[0] = memory[local_convert_cs_ip_to_real(ss, sp)];
            break;
        case 0X07:
            di.reg_half[1] = memory[local_convert_cs_ip_to_real(ss, sp) + 1];
            di.reg_half[0] = memory[local_convert_cs_ip_to_real(ss, sp)];
            break;
        default:
            break;
        }
        sp.reg += 2;
        return;
    }

    if ((com & 0XE7) == 0b00000111) // seg
    {
        switch (com & 0X18)
        {
        case 0b00000000:
            es.reg_half[1] = memory[local_convert_cs_ip_to_real(ss, sp) + 1];
            es.reg_half[0] = memory[local_convert_cs_ip_to_real(ss, sp)];
            break;
        case 0b00001000:
            cs.reg_half[1] = memory[local_convert_cs_ip_to_real(ss, sp) + 1];
            es.reg_half[0] = memory[local_convert_cs_ip_to_real(ss, sp)];
            break;
        case 0b00010000:
        {
            REGISTER tmp = ss;
            ss.reg_half[1] = memory[local_convert_cs_ip_to_real(tmp, sp) + 1];
            ss.reg_half[0] = memory[local_convert_cs_ip_to_real(tmp, sp)];
            break;
        }
        case 0b00011000:
            ds.reg_half[1] = memory[local_convert_cs_ip_to_real(ss, sp) + 1];
            ds.reg_half[0] = memory[local_convert_cs_ip_to_real(ss, sp)];
            break;
        default:
            break;
        }
    }
}

void cp8086::xchg(BYTE com)
{
    BYTE w = com & 0X01;
    BYTE scnd = memory[real_addr + 1];
    if ((com & 0XFE) == 0b10000110) // reg/mem
    {
        if ((scnd & 0XC0) == 0XC0) // reg to reg
        {
            ip.reg++;
            if (w == 1)
            {
                REGISTER *f, *s;
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
                REGISTER tmp;
                tmp.reg = f->reg;
                f->reg = s->reg;
                s->reg = tmp.reg;
                return;
            }
            else
            {
                int h1 = 0, h2 = 0;
                REGISTER *f, *s;
                switch ((scnd & 0X38))
                {
                case 0b00000000:
                    h1 = 0;
                    f = &ax;
                    break;

                case 0b00001000:
                    h1 = 0;
                    f = &cx;
                    break;

                case 0b00010000:
                    h1 = 0;
                    f = &dx;
                    break;

                case 0b00011000:
                    h1 = 0;
                    f = &bx;
                    break;

                case 0b00100000:
                    h1 = 1;
                    f = &ax;
                    break;

                case 0b00101000:
                    h1 = 1;
                    f = &cx;
                    break;

                case 0b00110000:
                    h1 = 1;
                    f = &dx;
                    break;

                case 0b00111000:
                    h1 = 1;
                    f = &bx;
                    break;
                }
                switch ((scnd & 0X07))
                {
                case 0b00000000:
                    h2 = 0;
                    s = &ax;
                    break;

                case 0b00000001:
                    h2 = 0;
                    s = &cx;
                    break;

                case 0b00000010:
                    h2 = 0;
                    s = &dx;
                    break;

                case 0b00000011:
                    h2 = 0;
                    s = &bx;
                    break;

                case 0b00000100:
                    h2 = 1;
                    s = &ax;
                    break;

                case 0b00000101:
                    h2 = 1;
                    s = &cx;
                    break;

                case 0b00000110:
                    h2 = 1;
                    s = &dx;
                    break;

                case 0b00000111:
                    h2 = 1;
                    s = &bx;
                    break;
                }
                BYTE tmp;
                tmp = f->reg_half[h1];
                f->reg_half[h1] = s->reg_half[h2];
                s->reg_half[h2] = tmp;
                return;
            }
        }
        else // reg/mem
        {
            REGISTER *reg;
            REG_SIZE addr_sum;
            switch ((scnd & 0XC7))
            {
            case 0b00000000:
                addr_sum = bx.reg + si.reg;
                ip.reg++;
                break;

            case 0b00000001:
                addr_sum = bx.reg + di.reg;
                ip.reg++;
                break;

            case 0b00000010:
                addr_sum = bp.reg + si.reg;
                ip.reg++;
                break;

            case 0b00000011:
                addr_sum = bp.reg + di.reg;
                ip.reg++;
                break;

            case 0b00000100:
                addr_sum = si.reg;
                ip.reg++;
                break;

            case 0b00000101:
                addr_sum = di.reg;
                ip.reg++;
                break;

            case 0b00000110:
            {
                REG_SIZE l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = l;
                ip.reg += 3;
                break;
            }

            case 0b00000111:
                addr_sum = bx.reg;
                ip.reg++;
                break;

            case 0b01000000:
                addr_sum = bx.reg + si.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000001:
                addr_sum = bx.reg + di.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000010:
                addr_sum = bp.reg + si.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000011:
                addr_sum = bp.reg + di.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000100:
                addr_sum = si.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000101:
                addr_sum = di.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000110:
                addr_sum = bp.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000111:
                addr_sum = bx.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b10000000:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = bx.reg + si.reg + l;
                ip.reg += 3;
            }
            break;

            case 0b10000001:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = bx.reg + di.reg + l;
                ip.reg += 3;
            }
            break;

            case 0b10000010:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = bp.reg + si.reg + l;
                ip.reg += 3;
            }
            break;

            case 0b10000011:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = bp.reg + di.reg + l;
                ip.reg += 3;
            }
            break;

            case 0b10000100:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = si.reg + l;
                ip.reg += 3;
            }
            break;

            case 0b10000101:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = di.reg + l;
                ip.reg += 3;
            }
            break;
            case 0b10000110:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = bp.reg + l;
                ip.reg += 3;
            }
            break;
            case 0b10000111:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = bx.reg + l;
                ip.reg += 3;
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
                REGISTER tmp;
                tmp.reg = reg->reg;
                reg->reg_half[0] = memory[addr_sum];
                reg->reg_half[1] = memory[addr_sum + 1];
                memory[addr_sum] = tmp.reg_half[0];
                memory[addr_sum + 1] = tmp.reg_half[1];
                return;
            }
            else
            {
                int h = 0;
                switch ((scnd & 0X38))
                {
                case 0b00000000:
                    h = 0;
                    reg = &ax;
                    break;

                case 0b00001000:
                    h = 0;
                    reg = &cx;
                    break;

                case 0b00010000:
                    h = 0;
                    reg = &dx;
                    break;

                case 0b00011000:
                    h = 0;
                    reg = &bx;
                    break;

                case 0b00100000:
                    h = 1;
                    reg = &ax;
                    break;

                case 0b00101000:
                    h = 1;
                    reg = &cx;
                    break;

                case 0b00110000:
                    h = 1;
                    reg = &dx;
                    break;

                case 0b00111000:
                    h = 1;
                    reg = &bx;
                    break;
                }
                BYTE tmp = reg->reg_half[h];
                reg->reg_half[h] = memory[addr_sum];
                memory[addr_sum] = tmp;
                return;
            }
        }
    }

    if ((com & 0XF8) == 0b10010000) // reg/ax
    {
        REGISTER *reg;
        switch ((com & 0X07))
        {
        case 0X01:
            reg = &cx;
            break;
        case 0X02:
            reg = &dx;
            break;
        case 0X03:
            reg = &bx;
            break;
        case 0X04:
            reg = &sp;
            break;
        case 0X05:
            reg = &bp;
            break;
        case 0X06:
            reg = &si;
            break;
        case 0X07:
            reg = &di;
            break;
        default:
            break;
        }
        REG_SIZE tmp;
        tmp = ax.reg;
        ax.reg = reg->reg;
        reg->reg = tmp;
        return;
    }
}

void cp8086::lea(BYTE com)
{
    BYTE scnd = memory[real_addr + 1];
    if ((scnd & 0XC0) == 0XC0)
    {
        std::cout << "There is error in lea\n";
        *error = 1;
        delete this;
    }
    else
    {
        REGISTER *reg;
        REG_SIZE addr_sum;
        switch ((scnd & 0XC7))
        {
        case 0b00000000:
            addr_sum = bx.reg + si.reg;
            ip.reg++;
            break;

        case 0b00000001:
            addr_sum = bx.reg + di.reg;
            ip.reg++;
            break;

        case 0b00000010:
            addr_sum = bp.reg + si.reg;
            ip.reg++;
            break;

        case 0b00000011:
            addr_sum = bp.reg + di.reg;
            ip.reg++;
            break;

        case 0b00000100:
            addr_sum = si.reg;
            ip.reg++;
            break;

        case 0b00000101:
            addr_sum = di.reg;
            ip.reg++;
            break;

        case 0b00000110:
        {
            REG_SIZE l = memory[real_addr + 3];
            l = l << 8;

            l = l | memory[real_addr + 2];
            addr_sum = l;
            ip.reg += 3;
            break;
        }

        case 0b00000111:
            addr_sum = bx.reg;
            ip.reg++;
            break;

        case 0b01000000:
            addr_sum = bx.reg + si.reg + memory[real_addr + 2];
            ip.reg += 2;
            break;

        case 0b01000001:
            addr_sum = bx.reg + di.reg + memory[real_addr + 2];
            ip.reg += 2;
            break;

        case 0b01000010:
            addr_sum = bp.reg + si.reg + memory[real_addr + 2];
            ip.reg += 2;
            break;

        case 0b01000011:
            addr_sum = bp.reg + di.reg + memory[real_addr + 2];
            ip.reg += 2;
            break;

        case 0b01000100:
            addr_sum = si.reg + memory[real_addr + 2];
            ip.reg += 2;
            break;

        case 0b01000101:
            addr_sum = di.reg + memory[real_addr + 2];
            ip.reg += 2;
            break;

        case 0b01000110:
            addr_sum = bp.reg + memory[real_addr + 2];
            ip.reg += 2;
            break;

        case 0b01000111:
            addr_sum = bx.reg + memory[real_addr + 2];
            ip.reg += 2;
            break;

        case 0b10000000:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;

            l = l | memory[real_addr + 2];
            addr_sum = bx.reg + si.reg + l;
            ip.reg += 3;
        }
        break;

        case 0b10000001:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;

            l = l | memory[real_addr + 2];
            addr_sum = bx.reg + di.reg + l;
            ip.reg += 3;
        }
        break;

        case 0b10000010:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;

            l = l | memory[real_addr + 2];
            addr_sum = bp.reg + si.reg + l;
            ip.reg += 3;
        }
        break;

        case 0b10000011:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;

            l = l | memory[real_addr + 2];
            addr_sum = bp.reg + di.reg + l;
            ip.reg += 3;
        }
        break;

        case 0b10000100:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;

            l = l | memory[real_addr + 2];
            addr_sum = si.reg + l;
            ip.reg += 3;
        }
        break;

        case 0b10000101:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;

            l = l | memory[real_addr + 2];
            addr_sum = di.reg + l;
            ip.reg += 3;
        }
        break;
        case 0b10000110:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;

            l = l | memory[real_addr + 2];
            addr_sum = bp.reg + l;
            ip.reg += 3;
        }
        break;
        case 0b10000111:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;

            l = l | memory[real_addr + 2];
            addr_sum = bx.reg + l;
            ip.reg += 3;
        }
        break;
        default:
            break;
        }
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
        reg->reg = addr_sum;
    }
}

void cp8086::lds(BYTE com)
{
    BYTE scnd = memory[real_addr + 1];
    if ((scnd & 0XC0) == 0XC0)
    {
        std::cout << "There is error in lds\n";
        *error = 2;
        delete this;
    }
    else
    {
        REGISTER *reg;
        REG_SIZE addr_sum;
        switch ((scnd & 0XC7))
        {
        case 0b00000000:
            addr_sum = bx.reg + si.reg;
            ip.reg++;
            break;

        case 0b00000001:
            addr_sum = bx.reg + di.reg;
            ip.reg++;
            break;

        case 0b00000010:
            addr_sum = bp.reg + si.reg;
            ip.reg++;
            break;

        case 0b00000011:
            addr_sum = bp.reg + di.reg;
            ip.reg++;
            break;

        case 0b00000100:
            addr_sum = si.reg;
            ip.reg++;
            break;

        case 0b00000101:
            addr_sum = di.reg;
            ip.reg++;
            break;

        case 0b00000110:
        {
            REG_SIZE l = memory[real_addr + 3];
            l = l << 8;

            l = l | memory[real_addr + 2];
            addr_sum = l;
            ip.reg += 3;
            break;
        }

        case 0b00000111:
            addr_sum = bx.reg;
            ip.reg++;
            break;

        case 0b01000000:
            addr_sum = bx.reg + si.reg + memory[real_addr + 2];
            ip.reg += 2;
            break;

        case 0b01000001:
            addr_sum = bx.reg + di.reg + memory[real_addr + 2];
            ip.reg += 2;
            break;

        case 0b01000010:
            addr_sum = bp.reg + si.reg + memory[real_addr + 2];
            ip.reg += 2;
            break;

        case 0b01000011:
            addr_sum = bp.reg + di.reg + memory[real_addr + 2];
            ip.reg += 2;
            break;

        case 0b01000100:
            addr_sum = si.reg + memory[real_addr + 2];
            ip.reg += 2;
            break;

        case 0b01000101:
            addr_sum = di.reg + memory[real_addr + 2];
            ip.reg += 2;
            break;

        case 0b01000110:
            addr_sum = bp.reg + memory[real_addr + 2];
            ip.reg += 2;
            break;

        case 0b01000111:
            addr_sum = bx.reg + memory[real_addr + 2];
            ip.reg += 2;
            break;

        case 0b10000000:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;

            l = l | memory[real_addr + 2];
            addr_sum = bx.reg + si.reg + l;
            ip.reg += 3;
        }
        break;

        case 0b10000001:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;

            l = l | memory[real_addr + 2];
            addr_sum = bx.reg + di.reg + l;
            ip.reg += 3;
        }
        break;

        case 0b10000010:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;

            l = l | memory[real_addr + 2];
            addr_sum = bp.reg + si.reg + l;
            ip.reg += 3;
        }
        break;

        case 0b10000011:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;

            l = l | memory[real_addr + 2];
            addr_sum = bp.reg + di.reg + l;
            ip.reg += 3;
        }
        break;

        case 0b10000100:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;

            l = l | memory[real_addr + 2];
            addr_sum = si.reg + l;
            ip.reg += 3;
        }
        break;

        case 0b10000101:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;

            l = l | memory[real_addr + 2];
            addr_sum = di.reg + l;
            ip.reg += 3;
        }
        break;
        case 0b10000110:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;

            l = l | memory[real_addr + 2];
            addr_sum = bp.reg + l;
            ip.reg += 3;
        }
        break;
        case 0b10000111:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;

            l = l | memory[real_addr + 2];
            addr_sum = bx.reg + l;
            ip.reg += 3;
        }
        break;
        default:
            break;
        }
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
        REG_SIZE tmp = ds.reg;
        reg->reg_half[0] = memory[local_convert_cs_ip_to_real(tmp, addr_sum)];
        reg->reg_half[1] = memory[local_convert_cs_ip_to_real(tmp, addr_sum) + 1];
        ds.reg_half[0] = memory[local_convert_cs_ip_to_real(tmp, addr_sum) + 2];
        ds.reg_half[1] = memory[local_convert_cs_ip_to_real(tmp, addr_sum) + 3];
    }
}

void cp8086::les(BYTE com)
{
    BYTE scnd = memory[real_addr + 1];
    if ((scnd & 0XC0) == 0XC0)
    {
        std::cout << "There is error in les\n";
        *error = 3;
        delete this;
    }
    else
    {
        REGISTER *reg;
        REG_SIZE addr_sum;
        switch ((scnd & 0XC7))
        {
        case 0b00000000:
            addr_sum = bx.reg + si.reg;
            ip.reg++;
            break;

        case 0b00000001:
            addr_sum = bx.reg + di.reg;
            ip.reg++;
            break;

        case 0b00000010:
            addr_sum = bp.reg + si.reg;
            ip.reg++;
            break;

        case 0b00000011:
            addr_sum = bp.reg + di.reg;
            ip.reg++;
            break;

        case 0b00000100:
            addr_sum = si.reg;
            ip.reg++;
            break;

        case 0b00000101:
            addr_sum = di.reg;
            ip.reg++;
            break;

        case 0b00000110:
        {
            REG_SIZE l = memory[real_addr + 3];
            l = l << 8;

            l = l | memory[real_addr + 2];
            addr_sum = l;
            ip.reg += 3;
            break;
        }

        case 0b00000111:
            addr_sum = bx.reg;
            ip.reg++;
            break;

        case 0b01000000:
            addr_sum = bx.reg + si.reg + memory[real_addr + 2];
            ip.reg += 2;
            break;

        case 0b01000001:
            addr_sum = bx.reg + di.reg + memory[real_addr + 2];
            ip.reg += 2;
            break;

        case 0b01000010:
            addr_sum = bp.reg + si.reg + memory[real_addr + 2];
            ip.reg += 2;
            break;

        case 0b01000011:
            addr_sum = bp.reg + di.reg + memory[real_addr + 2];
            ip.reg += 2;
            break;

        case 0b01000100:
            addr_sum = si.reg + memory[real_addr + 2];
            ip.reg += 2;
            break;

        case 0b01000101:
            addr_sum = di.reg + memory[real_addr + 2];
            ip.reg += 2;
            break;

        case 0b01000110:
            addr_sum = bp.reg + memory[real_addr + 2];
            ip.reg += 2;
            break;

        case 0b01000111:
            addr_sum = bx.reg + memory[real_addr + 2];
            ip.reg += 2;
            break;

        case 0b10000000:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;

            l = l | memory[real_addr + 2];
            addr_sum = bx.reg + si.reg + l;
            ip.reg += 3;
        }
        break;

        case 0b10000001:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;

            l = l | memory[real_addr + 2];
            addr_sum = bx.reg + di.reg + l;
            ip.reg += 3;
        }
        break;

        case 0b10000010:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;

            l = l | memory[real_addr + 2];
            addr_sum = bp.reg + si.reg + l;
            ip.reg += 3;
        }
        break;

        case 0b10000011:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;

            l = l | memory[real_addr + 2];
            addr_sum = bp.reg + di.reg + l;
            ip.reg += 3;
        }
        break;

        case 0b10000100:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;

            l = l | memory[real_addr + 2];
            addr_sum = si.reg + l;
            ip.reg += 3;
        }
        break;

        case 0b10000101:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;

            l = l | memory[real_addr + 2];
            addr_sum = di.reg + l;
            ip.reg += 3;
        }
        break;
        case 0b10000110:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;

            l = l | memory[real_addr + 2];
            addr_sum = bp.reg + l;
            ip.reg += 3;
        }
        break;
        case 0b10000111:
        {
            REG_SIZE l = 0;
            l = memory[real_addr + 3];
            l = l << 8;

            l = l | memory[real_addr + 2];
            addr_sum = bx.reg + l;
            ip.reg += 3;
        }
        break;
        default:
            break;
        }
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
        reg->reg_half[0] = memory[local_convert_cs_ip_to_real(ds, addr_sum)];
        reg->reg_half[1] = memory[local_convert_cs_ip_to_real(ds, addr_sum) + 1];
        es.reg_half[0] = memory[local_convert_cs_ip_to_real(ds, addr_sum) + 2];
        es.reg_half[1] = memory[local_convert_cs_ip_to_real(ds, addr_sum) + 3];
    }
}

void cp8086::lahf(BYTE com)
{
    ax.reg_half[1] = flags & 0X00FF;
    printFlags();
}

void cp8086::sahf(BYTE com)
{
    flags = ax.reg_half[1];
    printFlags();
}

void cp8086::pushf(BYTE com)
{
    sp.reg -= 2;
    memory[local_convert_cs_ip_to_real(ss, sp) + 1] = (flags >> 8);
    memory[local_convert_cs_ip_to_real(ss, sp)] = flags;
    printStack();
}

void cp8086::popf(BYTE com)
{
    flags = memory[local_convert_cs_ip_to_real(ss, sp) + 1];
    flags = flags << 8;
    flags |= memory[local_convert_cs_ip_to_real(ss, sp)];
    sp.reg += 2;
    printStack();
}

void cp8086::add(BYTE com)
{
    if ((com & 0XFC) == 0X00) // reg mem, reg reg, mem reg
    {
        BYTE scnd = memory[real_addr + 1];
        int w = com & 0X01;
        int d = com & 0X02;
        printf("%0X\n",scnd & 0XC0);
        if ((scnd & 0XC0) == 0XC0) // reg reg
        {
            ip.reg++;
            if (w == 1)
            {
                REGISTER *f, *s;
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
                    REAL_ADDR_SIZE c = 0;
                    REG_SIZE a = 0, b = 0;
                    signed short a_s = (signed short)f->reg, b_s = (signed short)s->reg, of;
                    of = a_s + b_s;
                    REGISTER sum;
                    b = f->reg + s->reg;
                    sum.reg = (REG_SIZE)of;
                    if ((a_s < 0 && b_s < 0 && of > 0) || (a_s > 0 && b_s > 0 && of < 0))
                        flagOF(1); // OF
                    else
                        flagOF(0);
                    if (of == 0)
                        flagZF(1); // ZF
                    else
                        flagZF(0);
                    c = s->reg + f->reg;
                    if (c & 0X00010000) // CF
                        flagCF(1);
                    else
                        flagCF(0);
                    if (a & 0X0100) // AF
                        flagAF(1);
                    else
                        flagAF(0);
                    int p = b >> 15;
                    flagSF(p); // SF
                    std::bitset<8> cc{sum.reg_half[0]};
                    int sm = cc[0] + cc[1] + cc[2] + cc[3] + cc[4] + cc[5] + cc[6] + cc[7];
                    if (sm == 0 || sm == 2 || sm == 4 || sm == 6 || sm == 8)
                        flagPF(1); // PF
                    else
                        flagPF(0);
                    s->reg = sum.reg;
                }
                else
                {
                    REAL_ADDR_SIZE c = 0;
                    REG_SIZE a = 0, b = 0;
                    signed short a_s = (signed short)f->reg, b_s = (signed short)s->reg, of;
                    of = a_s + b_s;
                    REGISTER sum;
                    b = f->reg + s->reg;
                    sum.reg = (REG_SIZE)of;
                    if ((a_s < 0 && b_s < 0 && of > 0) || (a_s > 0 && b_s > 0 && of < 0))
                        flagOF(1); // OF
                    else
                        flagOF(0);
                    if (of == 0)
                        flagZF(1); // ZF
                    else
                        flagZF(0);
                    c = s->reg + f->reg;
                    if (c & 0X00010000) // CF
                        flagCF(1);
                    else
                        flagCF(0);
                    if (a & 0X0100) // AF
                        flagAF(1);
                    else
                        flagAF(0);
                    int p = b >> 15;
                    flagSF(p); // SF
                    std::bitset<8> cc{sum.reg_half[0]};
                    int sm = cc[0] + cc[1] + cc[2] + cc[3] + cc[4] + cc[5] + cc[6] + cc[7];
                    if (sm == 0 || sm == 2 || sm == 4 || sm == 6 || sm == 8)
                        flagPF(1); // PF
                    else
                        flagPF(0);
                    f->reg = sum.reg;
                }
                return;
            }
            else
            {
                int h1, h2;
                REGISTER *f, *s;
                switch ((scnd & 0X38))
                {
                case 0b00000000:
                    h1 = 0;
                    f = &ax;
                    break;

                case 0b00001000:
                    h1 = 0;
                    f = &cx;
                    break;

                case 0b00010000:
                    h1 = 0;
                    f = &dx;
                    break;

                case 0b00011000:
                    h1 = 0;
                    f = &bx;
                    break;

                case 0b00100000:
                    h1 = 1;
                    f = &ax;
                    break;

                case 0b00101000:
                    h1 = 1;
                    f = &cx;
                    break;

                case 0b00110000:
                    h1 = 1;
                    f = &dx;
                    break;

                case 0b00111000:
                    h1 = 1;
                    f = &bx;
                    break;
                }
                switch ((scnd & 0X07))
                {
                case 0b00000000:
                    h2 = 0;
                    s = &ax;
                    break;

                case 0b00000001:
                    h2 = 0;
                    s = &cx;
                    break;

                case 0b00000010:
                    h2 = 0;
                    s = &dx;
                    break;

                case 0b00000011:
                    h2 = 0;
                    s = &bx;
                    break;

                case 0b00000100:
                    h2 = 1;
                    s = &ax;
                    break;

                case 0b00000101:
                    h2 = 1;
                    s = &cx;
                    break;

                case 0b00000110:
                    h2 = 1;
                    s = &dx;
                    break;

                case 0b00000111:
                    h2 = 1;
                    s = &bx;
                    break;
                }
                if ((com & 0X02) == 0X00)
                {
                    signed short a = 0;
                    BYTE j;
                    signed char d1, d2, d3, d4;
                    d1 = (signed char)f->reg_half[h1];
                    d2 = (signed char)s->reg_half[h2];
                    d3 = d1 & 0X07;
                    d4 = d2 & 0X07;
                    a = d1 + d2;
                    j = (BYTE)a;
                    flagOF((int)(a >= 128 || a <= -127));
                    flagCF((int)(a >= 128 || a <= -127));
                    std::bitset<8> cc{j};
                    int sm = cc[0] + cc[1] + cc[2] + cc[3] + cc[4] + cc[5] + cc[6] + cc[7];
                    flagPF((int)(sm == 0 || sm == 2 || sm == 4 || sm == 6 || sm == 8));
                    BYTE ms = d3 + d4;
                    flagAF((int)((ms & 0X08) == 0X08));
                    flagZF((int)(a == 0));
                    if ((a & 0X0080) == 0X0080)
                        flagSF(1);
                    else
                        flagSF(0);
                    s->reg_half[h2] = j;
                    return;
                }
                else
                {
                    signed short a = 0;
                    BYTE j;
                    signed char d1, d2, d3, d4;
                    d1 = (signed char)f->reg_half[h1];
                    d2 = (signed char)s->reg_half[h2];
                    d3 = d1 & 0X07;
                    d4 = d2 & 0X07;
                    a = d1 + d2;
                    j = (BYTE)a;
                    flagOF((int)(a >= 128 || a <= -127));
                    flagCF((int)(a >= 128 || a <= -127));
                    std::bitset<8> cc{j};
                    int sm = cc[0] + cc[1] + cc[2] + cc[3] + cc[4] + cc[5] + cc[6] + cc[7];
                    flagPF((int)(sm == 0 || sm == 2 || sm == 4 || sm == 6 || sm == 8));
                    BYTE ms = d3 + d4;
                    flagAF((int)((ms & 0X08) == 0X08));
                    flagZF((int)(a == 0));
                    if ((a & 0X0080) == 0X0080)
                        flagSF(1);
                    else
                        flagSF(0);
                    f->reg_half[h2] = j;
                    return;
                }
            }
        }
        else
        {
            REGISTER *reg;
            REG_SIZE addr_sum;
            switch ((scnd & 0XC7))
            {
            case 0b00000000:
                addr_sum = bx.reg + si.reg;
                ip.reg++;
                break;

            case 0b00000001:
                addr_sum = bx.reg + di.reg;
                ip.reg++;
                break;

            case 0b00000010:
                addr_sum = bp.reg + si.reg;
                ip.reg++;
                break;

            case 0b00000011:
                addr_sum = bp.reg + di.reg;
                ip.reg++;
                break;

            case 0b00000100:
                addr_sum = si.reg;
                ip.reg++;
                break;

            case 0b00000101:
                addr_sum = di.reg;
                ip.reg++;
                break;

            case 0b00000110:
            {
                REG_SIZE l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = l;
                ip.reg += 3;
                break;
            }

            case 0b00000111:
                addr_sum = bx.reg;
                ip.reg++;
                break;

            case 0b01000000:
                addr_sum = bx.reg + si.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000001:
                addr_sum = bx.reg + di.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000010:
                addr_sum = bp.reg + si.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000011:
                addr_sum = bp.reg + di.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000100:
                addr_sum = si.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000101:
                addr_sum = di.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000110:
                addr_sum = bp.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000111:
                addr_sum = bx.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b10000000:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = bx.reg + si.reg + l;
                ip.reg += 3;
            }
            break;

            case 0b10000001:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = bx.reg + di.reg + l;
                ip.reg += 3;
            }
            break;

            case 0b10000010:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = bp.reg + si.reg + l;
                ip.reg += 3;
            }
            break;

            case 0b10000011:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = bp.reg + di.reg + l;
                ip.reg += 3;
            }
            break;

            case 0b10000100:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = si.reg + l;
                ip.reg += 3;
            }
            break;

            case 0b10000101:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = di.reg + l;
                ip.reg += 3;
            }
            break;
            case 0b10000110:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = bp.reg + l;
                ip.reg += 3;
            }
            break;
            case 0b10000111:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = bx.reg + l;
                ip.reg += 3;
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
                if ((com & 0X02) == 0X02)
                {
                    REAL_ADDR_SIZE c = 0;
                    REG_SIZE a = 0, b = 0;
                    REGISTER m;
                    m.reg_half[1] = memory[local_convert_cs_ip_to_real(ds, addr_sum) + 1];
                    m.reg_half[0] = memory[local_convert_cs_ip_to_real(ds, addr_sum)];
                    signed short a_s = (signed short)reg->reg, b_s = (signed short)m.reg, of;
                    of = a_s + b_s;
                    REGISTER sum;
                    b = reg->reg + m.reg;
                    sum.reg = (REG_SIZE)of;
                    if ((a_s < 0 && b_s < 0 && of > 0) || (a_s > 0 && b_s > 0 && of < 0))
                        flagOF(1); // OF
                    else
                        flagOF(0);
                    if (of == 0)
                        flagZF(1); // ZF
                    else
                        flagZF(0);
                    c = reg->reg + m.reg;
                    if (c & 0X00010000) // CF
                        flagCF(1);
                    else
                        flagCF(0);
                    if (a & 0X0100) // AF
                        flagAF(1);
                    else
                        flagAF(0);
                    int p = b >> 15;
                    flagSF(p); // SF
                    std::bitset<8> cc{sum.reg_half[0]};
                    int sm = cc[0] + cc[1] + cc[2] + cc[3] + cc[4] + cc[5] + cc[6] + cc[7];
                    if (sm == 0 || sm == 2 || sm == 4 || sm == 6 || sm == 8)
                        flagPF(1); // PF
                    else
                        flagPF(0);
                    reg->reg = sum.reg;
                }
                else
                {
                    REAL_ADDR_SIZE c = 0;
                    REG_SIZE a = 0, b = 0;
                    REGISTER m;
                    m.reg_half[1] = memory[local_convert_cs_ip_to_real(ds, addr_sum) + 1];
                    m.reg_half[0] = memory[local_convert_cs_ip_to_real(ds, addr_sum)];
                    signed short a_s = (signed short)reg->reg, b_s = (signed short)m.reg, of;
                    of = a_s + b_s;
                    REGISTER sum;
                    b = reg->reg + m.reg;
                    sum.reg = (REG_SIZE)of;
                    if ((a_s < 0 && b_s < 0 && of > 0) || (a_s > 0 && b_s > 0 && of < 0))
                        flagOF(1); // OF
                    else
                        flagOF(0);
                    if (of == 0)
                        flagZF(1); // ZF+
                    else
                        flagZF(0);
                    c = reg->reg + m.reg;
                    if (c & 0X00010000) // CF
                        flagCF(1);
                    else
                        flagCF(0);
                    if (a & 0X0100) // AF
                        flagAF(1);
                    else
                        flagAF(0);
                    int p = b >> 15;
                    flagSF(p); // SF
                    std::bitset<8> cc{sum.reg_half[0]};
                    int sm = cc[0] + cc[1] + cc[2] + cc[3] + cc[4] + cc[5] + cc[6] + cc[7];
                    if (sm == 0 || sm == 2 || sm == 4 || sm == 6 || sm == 8)
                        flagPF(1); // PF
                    else
                        flagPF(0);
                    memory[local_convert_cs_ip_to_real(ds, addr_sum) + 1] = sum.reg_half[1];
                    memory[local_convert_cs_ip_to_real(ds, addr_sum)] = sum.reg_half[0];
                }
            }
            else
            {
                return;
            }
        }
    }

    if ((com & 0XFC) == 0X80) // reg/mem data
    {

        int sw = com & 0X03;
        BYTE scnd = memory[real_addr + 1];
        if ((scnd & 0XC0) == 0XC0) // reg
        {
            if (sw == 1)
            {
                ip.reg += 3;
                REGISTER *r;
                switch ((scnd & 0X07))
                {
                case 0b00000000:
                    r = &ax;
                    break;

                case 0b00000001:
                    r = &cx;
                    break;

                case 0b00000010:
                    r = &dx;
                    break;

                case 0b00000011:
                    r = &bx;
                    break;

                case 0b00000100:
                    r = &sp;
                    break;

                case 0b00000101:
                    r = &bp;
                    break;

                case 0b00000110:
                    r = &si;
                    break;

                case 0b00000111:
                    r = &di;
                    break;
                }
                REGISTER data;
                data.reg_half[0] = memory[real_addr + 2];
                data.reg_half[1] = memory[real_addr + 3];
                REAL_ADDR_SIZE c = 0;
                REG_SIZE a = 0, b = 0;
                signed short a_s = (signed short)r->reg, b_s = (signed short)data.reg, of;
                of = a_s + b_s;
                REGISTER sum;
                b = r->reg + data.reg;
                sum.reg = (REG_SIZE)of;
                if ((a_s < 0 && b_s < 0 && of > 0) || (a_s > 0 && b_s > 0 && of < 0))
                    flagOF(1); // OF
                else
                    flagOF(0);
                if (of == 0)
                    flagZF(1); // ZF
                else
                    flagZF(0);
                c = r->reg + data.reg;
                if (c & 0X00010000) // CF
                    flagCF(1);
                else
                    flagCF(0);
                if (a & 0X0100) // AF
                    flagAF(1);
                else
                    flagAF(0);
                int p = b >> 15;
                flagSF(p); // SF
                std::bitset<8> cc{sum.reg_half[0]};
                int sm = cc[0] + cc[1] + cc[2] + cc[3] + cc[4] + cc[5] + cc[6] + cc[7];
                if (sm == 0 || sm == 2 || sm == 4 || sm == 6 || sm == 8)
                    flagPF(1); // PF
                else
                    flagPF(0);
                r->reg = sum.reg;
                return;
            }
            else
            {
                ip.reg += 2;
                REGISTER *r;
                int h = 0;
                switch ((scnd & 0X07))
                {
                case 0b00000000:
                    h = 0;
                    r = &ax;
                    break;

                case 0b00000001:
                    h = 0;
                    r = &cx;
                    break;

                case 0b00000010:
                    h = 0;
                    r = &dx;
                    break;

                case 0b00000011:
                    h = 0;
                    r = &bx;
                    break;

                case 0b00000100:
                    h = 1;
                    r = &ax;
                    break;

                case 0b00000101:
                    h = 1;
                    r = &cx;
                    break;

                case 0b00000110:
                    h = 1;
                    r = &dx;
                    break;

                case 0b00000111:
                    h = 1;
                    r = &bx;
                    break;
                }
                BYTE data;
                data = memory[real_addr + 2];
                signed short a = 0;
                BYTE j;
                signed char d1, d2, d3, d4;
                d1 = (signed char)r->reg_half[h];
                d2 = (signed char)data;
                d3 = d1 & 0X07;
                d4 = d2 & 0X07;
                a = d1 + d2;
                j = (BYTE)a;
                flagOF((int)(a >= 128 || a <= -127));
                flagCF((int)(a >= 128 || a <= -127));
                std::bitset<8> cc{j};
                int sm = cc[0] + cc[1] + cc[2] + cc[3] + cc[4] + cc[5] + cc[6] + cc[7];
                flagPF((int)(sm == 0 || sm == 2 || sm == 4 || sm == 6 || sm == 8));
                BYTE ms = d3 + d4;
                flagAF((int)((ms & 0X08) == 0X08));
                flagZF((int)(a == 0));
                if ((a & 0X0080) == 0X0080)
                    flagSF(1);
                else
                    flagSF(0);
                r->reg_half[h] = j;
                return;
            }
        }
    }
}

void cp8086::adc(BYTE com)
{
    if ((com & 0XFC) == 0X10) // reg mem, reg reg, mem reg
    {
        BYTE scnd = memory[real_addr + 1];
        int w = com & 0X01;
        int d = com & 0X02;
        int cf = (flags & 0X0001) == 0X0001;
        if ((scnd & 0XC0) == 0XC0) // reg reg
        {
            ip.reg++;
            if (w == 1)
            {
                REGISTER *f, *s;
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
                    REAL_ADDR_SIZE c = 0;
                    REG_SIZE a = 0, b = 0;
                    signed short a_s = (signed short)f->reg, b_s = (signed short)s->reg, of;
                    of = a_s + b_s + cf;
                    REGISTER sum;
                    b = f->reg + s->reg + cf;
                    sum.reg = (REG_SIZE)of;
                    if ((a_s < 0 && b_s < 0 && of > 0) || (a_s > 0 && b_s > 0 && of < 0))
                        flagOF(1); // OF
                    else
                        flagOF(0);
                    if (of == 0)
                        flagZF(1); // ZF
                    else
                        flagZF(0);
                    c = s->reg + f->reg + cf;
                    if (c & 0X00010000) // CF
                        flagCF(1);
                    else
                        flagCF(0);
                    if (a & 0X0100) // AF
                        flagAF(1);
                    else
                        flagAF(0);
                    int p = b >> 15;
                    flagSF(p); // SF
                    std::bitset<8> cc{sum.reg_half[0]};
                    int sm = cc[0] + cc[1] + cc[2] + cc[3] + cc[4] + cc[5] + cc[6] + cc[7];
                    if (sm == 0 || sm == 2 || sm == 4 || sm == 6 || sm == 8)
                        flagPF(1); // PF
                    else
                        flagPF(0);
                    s->reg = sum.reg;
                }
                else
                {
                    REAL_ADDR_SIZE c = 0;
                    REG_SIZE a = 0, b = 0;
                    signed short a_s = (signed short)f->reg, b_s = (signed short)s->reg, of;
                    of = a_s + b_s + cf;
                    REGISTER sum;
                    b = f->reg + s->reg + cf;
                    sum.reg = (REG_SIZE)of;
                    if ((a_s < 0 && b_s < 0 && of > 0) || (a_s > 0 && b_s > 0 && of < 0))
                        flagOF(1); // OF
                    else
                        flagOF(0);
                    if (of == 0)
                        flagZF(1); // ZF
                    else
                        flagZF(0);
                    c = s->reg + f->reg + cf;
                    if (c & 0X00010000) // CF
                        flagCF(1);
                    else
                        flagCF(0);
                    if (a & 0X0100) // AF
                        flagAF(1);
                    else
                        flagAF(0);
                    int p = b >> 15;
                    flagSF(p); // SF
                    std::bitset<8> cc{sum.reg_half[0]};
                    int sm = cc[0] + cc[1] + cc[2] + cc[3] + cc[4] + cc[5] + cc[6] + cc[7];
                    if (sm == 0 || sm == 2 || sm == 4 || sm == 6 || sm == 8)
                        flagPF(1); // PF
                    else
                        flagPF(0);
                    f->reg = sum.reg;
                }
                return;
            }
            else
            {
                int h1, h2;
                REGISTER *f, *s;
                switch ((scnd & 0X38))
                {
                case 0b00000000:
                    h1 = 0;
                    f = &ax;
                    break;

                case 0b00001000:
                    h1 = 0;
                    f = &cx;
                    break;

                case 0b00010000:
                    h1 = 0;
                    f = &dx;
                    break;

                case 0b00011000:
                    h1 = 0;
                    f = &bx;
                    break;

                case 0b00100000:
                    h1 = 1;
                    f = &ax;
                    break;

                case 0b00101000:
                    h1 = 1;
                    f = &cx;
                    break;

                case 0b00110000:
                    h1 = 1;
                    f = &dx;
                    break;

                case 0b00111000:
                    h1 = 1;
                    f = &bx;
                    break;
                }
                switch ((scnd & 0X07))
                {
                case 0b00000000:
                    h2 = 0;
                    s = &ax;
                    break;

                case 0b00000001:
                    h2 = 0;
                    s = &cx;
                    break;

                case 0b00000010:
                    h2 = 0;
                    s = &dx;
                    break;

                case 0b00000011:
                    h2 = 0;
                    s = &bx;
                    break;

                case 0b00000100:
                    h2 = 1;
                    s = &ax;
                    break;

                case 0b00000101:
                    h2 = 1;
                    s = &cx;
                    break;

                case 0b00000110:
                    h2 = 1;
                    s = &dx;
                    break;

                case 0b00000111:
                    h2 = 1;
                    s = &bx;
                    break;
                }
                if ((com & 0X02) == 0X00)
                {
                    signed short a = 0;
                    BYTE j;
                    signed char d1, d2, d3, d4;
                    d1 = (signed char)f->reg_half[h1];
                    d2 = (signed char)s->reg_half[h2];
                    d3 = d1 & 0X07;
                    d4 = d2 & 0X07;
                    a = d1 + d2 + cf;
                    j = (BYTE)a;
                    flagOF((int)(a >= 128 || a <= -127));
                    flagCF((int)(a >= 128 || a <= -127));
                    std::bitset<8> cc{j};
                    int sm = cc[0] + cc[1] + cc[2] + cc[3] + cc[4] + cc[5] + cc[6] + cc[7];
                    flagPF((int)(sm == 0 || sm == 2 || sm == 4 || sm == 6 || sm == 8));
                    BYTE ms = d3 + d4 + cf;
                    flagAF((int)((ms & 0X08) == 0X08));
                    flagZF((int)(a == 0));
                    if ((a & 0X0080) == 0X0080)
                        flagSF(1);
                    else
                        flagSF(0);
                    s->reg_half[h2] = j;
                    return;
                }
                else
                {
                    signed short a = 0;
                    BYTE j;
                    signed char d1, d2, d3, d4;
                    d1 = (signed char)f->reg_half[h1];
                    d2 = (signed char)s->reg_half[h2];
                    d3 = d1 & 0X07;
                    d4 = d2 & 0X07;
                    a = d1 + d2 + cf;
                    j = (BYTE)a;
                    flagOF((int)(a >= 128 || a <= -127));
                    flagCF((int)(a >= 128 || a <= -127));
                    std::bitset<8> cc{j};
                    int sm = cc[0] + cc[1] + cc[2] + cc[3] + cc[4] + cc[5] + cc[6] + cc[7];
                    flagPF((int)(sm == 0 || sm == 2 || sm == 4 || sm == 6 || sm == 8));
                    BYTE ms = d3 + d4 + cf;
                    flagAF((int)((ms & 0X08) == 0X08));
                    flagZF((int)(a == 0));
                    if ((a & 0X0080) == 0X0080)
                        flagSF(1);
                    else
                        flagSF(0);
                    f->reg_half[h2] = j;
                    return;
                }
            }
        }
        else
        {
            REGISTER *reg;
            REG_SIZE addr_sum;
            switch ((scnd & 0XC7))
            {
            case 0b00000000:
                addr_sum = bx.reg + si.reg;
                ip.reg++;
                break;

            case 0b00000001:
                addr_sum = bx.reg + di.reg;
                ip.reg++;
                break;

            case 0b00000010:
                addr_sum = bp.reg + si.reg;
                ip.reg++;
                break;

            case 0b00000011:
                addr_sum = bp.reg + di.reg;
                ip.reg++;
                break;

            case 0b00000100:
                addr_sum = si.reg;
                ip.reg++;
                break;

            case 0b00000101:
                addr_sum = di.reg;
                ip.reg++;
                break;

            case 0b00000110:
            {
                REG_SIZE l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = l;
                ip.reg += 3;
                break;
            }

            case 0b00000111:
                addr_sum = bx.reg;
                ip.reg++;
                break;

            case 0b01000000:
                addr_sum = bx.reg + si.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000001:
                addr_sum = bx.reg + di.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000010:
                addr_sum = bp.reg + si.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000011:
                addr_sum = bp.reg + di.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000100:
                addr_sum = si.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000101:
                addr_sum = di.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000110:
                addr_sum = bp.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b01000111:
                addr_sum = bx.reg + memory[real_addr + 2];
                ip.reg += 2;
                break;

            case 0b10000000:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = bx.reg + si.reg + l;
                ip.reg += 3;
            }
            break;

            case 0b10000001:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = bx.reg + di.reg + l;
                ip.reg += 3;
            }
            break;

            case 0b10000010:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = bp.reg + si.reg + l;
                ip.reg += 3;
            }
            break;

            case 0b10000011:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = bp.reg + di.reg + l;
                ip.reg += 3;
            }
            break;

            case 0b10000100:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = si.reg + l;
                ip.reg += 3;
            }
            break;

            case 0b10000101:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = di.reg + l;
                ip.reg += 3;
            }
            break;
            case 0b10000110:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = bp.reg + l;
                ip.reg += 3;
            }
            break;
            case 0b10000111:
            {
                REG_SIZE l = 0;
                l = memory[real_addr + 3];
                l = l << 8;

                l = l | memory[real_addr + 2];
                addr_sum = bx.reg + l;
                ip.reg += 3;
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
                if ((com & 0X02) == 0X02)
                {
                    REAL_ADDR_SIZE c = 0;
                    REG_SIZE a = 0, b = 0;
                    REGISTER m;
                    m.reg_half[1] = memory[local_convert_cs_ip_to_real(ds, addr_sum) + 1];
                    m.reg_half[0] = memory[local_convert_cs_ip_to_real(ds, addr_sum)];
                    signed short a_s = (signed short)reg->reg, b_s = (signed short)m.reg, of;
                    of = a_s + b_s + cf;
                    REGISTER sum;
                    b = reg->reg + m.reg + cf;
                    sum.reg = (REG_SIZE)of;
                    if ((a_s < 0 && b_s < 0 && of > 0) || (a_s > 0 && b_s > 0 && of < 0))
                        flagOF(1); // OF
                    else
                        flagOF(0);
                    if (of == 0)
                        flagZF(1); // ZF
                    else
                        flagZF(0);
                    c = reg->reg + m.reg + cf;
                    if (c & 0X00010000) // CF
                        flagCF(1);
                    else
                        flagCF(0);
                    if (a & 0X0100) // AF
                        flagAF(1);
                    else
                        flagAF(0);
                    int p = b >> 15;
                    flagSF(p); // SF
                    std::bitset<8> cc{sum.reg_half[0]};
                    int sm = cc[0] + cc[1] + cc[2] + cc[3] + cc[4] + cc[5] + cc[6] + cc[7];
                    if (sm == 0 || sm == 2 || sm == 4 || sm == 6 || sm == 8)
                        flagPF(1); // PF
                    else
                        flagPF(0);
                    reg->reg = sum.reg;
                }
                else
                {
                    REAL_ADDR_SIZE c = 0;
                    REG_SIZE a = 0, b = 0;
                    REGISTER m;
                    m.reg_half[1] = memory[local_convert_cs_ip_to_real(ds, addr_sum) + 1];
                    m.reg_half[0] = memory[local_convert_cs_ip_to_real(ds, addr_sum)];
                    signed short a_s = (signed short)reg->reg, b_s = (signed short)m.reg, of;
                    of = a_s + b_s + cf;
                    REGISTER sum;
                    b = reg->reg + m.reg + cf;
                    sum.reg = (REG_SIZE)of;
                    if ((a_s < 0 && b_s < 0 && of > 0) || (a_s > 0 && b_s > 0 && of < 0))
                        flagOF(1); // OF
                    else
                        flagOF(0);
                    if (of == 0)
                        flagZF(1); // ZF+
                    else
                        flagZF(0);
                    c = reg->reg + m.reg + cf;
                    if (c & 0X00010000) // CF
                        flagCF(1);
                    else
                        flagCF(0);
                    if (a & 0X0100) // AF
                        flagAF(1);
                    else
                        flagAF(0);
                    int p = b >> 15;
                    flagSF(p); // SF
                    std::bitset<8> cc{sum.reg_half[0]};
                    int sm = cc[0] + cc[1] + cc[2] + cc[3] + cc[4] + cc[5] + cc[6] + cc[7];
                    if (sm == 0 || sm == 2 || sm == 4 || sm == 6 || sm == 8)
                        flagPF(1); // PF
                    else
                        flagPF(0);
                    memory[local_convert_cs_ip_to_real(ds, addr_sum) + 1] = sum.reg_half[1];
                    memory[local_convert_cs_ip_to_real(ds, addr_sum)] = sum.reg_half[0];
                }
            }
            else
            {
                return;
            }
        }
    }
}

void cp8086::mul(BYTE com)
{
    int w = (com & 0X0001);
    BYTE scnd = memory[real_addr + 1];
    if ((scnd & 0XC0) == 0XC0) // reg
    {
        ip.reg++;
        if (w == 1)
        {
            REGISTER *reg;
            switch ((scnd & 0X07))
            {
            case 0b00000000:
                reg = &ax;
                break;

            case 0b00000001:
                reg = &cx;
                break;

            case 0b00000010:
                reg = &dx;
                break;

            case 0b00000011:
                reg = &bx;
                break;

            case 0b00000100:
                reg = &sp;
                break;

            case 0b00000101:
                reg = &bp;
                break;

            case 0b00000110:
                reg = &si;
                break;

            case 0b00000111:
                reg = &di;
                break;
            }
            REAL_ADDR_SIZE mux;
            mux = ax.reg * reg->reg;
            ax.reg = (mux & 0X0000FFFF);
            dx.reg = ((mux >> 16) & 0X0000FFFF);
            flagCF(dx.reg != 0);
            flagOF(dx.reg != 0);
        }
    }
}

void cp8086::imul(BYTE com)
{
    int w = (com & 0X0001);
    BYTE scnd = memory[real_addr + 1];
    if ((scnd & 0XC0) == 0XC0) // reg
    {
        ip.reg++;
        if (w == 1)
        {
            REGISTER *reg;
            switch ((scnd & 0X07))
            {
            case 0b00000000:
                reg = &ax;
                break;

            case 0b00000001:
                reg = &cx;
                break;

            case 0b00000010:
                reg = &dx;
                break;

            case 0b00000011:
                reg = &bx;
                break;

            case 0b00000100:
                reg = &sp;
                break;

            case 0b00000101:
                reg = &bp;
                break;

            case 0b00000110:
                reg = &si;
                break;

            case 0b00000111:
                reg = &di;
                break;
            }
            signed int mux;
            mux = (signed short)ax.reg * (signed short)reg->reg;
            ax.reg = (mux & 0X0000FFFF);
            dx.reg = ((mux >> 16) & 0X0000FFFF);
            flagCF(dx.reg != 0);
            flagOF(dx.reg != 0);
        }
    }
}

void cp8086::cmp(BYTE com)
{
    if ((com & 0XFC) == 0b00111000) // reg/mem
    {
        BYTE scnd = memory[real_addr + 1];
        int w = com & 0X01;
        int d = com & 0X02;
        if ((scnd & 0XC0) == 0XC0) // reg reg
        {
            ip.reg++;
            if (w == 1)
            {
                REGISTER *f, *s;
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
                    REAL_ADDR_SIZE c = 0;
                    REG_SIZE a = 0, b = 0;
                    signed short l1 = s->reg, l2 = f->reg, l = l1 - l2;
                    b = s->reg - f->reg;
                    a = s->reg_half[0] - f->reg_half[0];
                    if ((l1 & 0X0007) < (l2 & 0X0007)) // AF
                        flagAF(1);
                    else
                        flagAF(0);
                    if (l1 < l2) // CF
                        flagCF(1);
                    else
                        flagCF(0);
                    if ((l1 > 0 && l2 > 0 && l < 0) || (l1 > 0 && l2 > 0 && l < 0))
                        flagOF(1);
                    else
                        flagOF(0);
                    if (b) // ZF
                        flagZF(0);
                    else
                        flagZF(1);
                    int p = l >> 15;
                    flagSF(p); // SF
                    REGISTER tmp;
                    tmp.reg = s->reg - f->reg;
                    std::bitset<8> cc{tmp.reg_half[0]};
                    int sm = cc[0] + cc[1] + cc[2] + cc[3] + cc[4] + cc[5] + cc[6] + cc[7];
                    if (sm == 0 || sm == 2 || sm == 4 || sm == 6 || sm == 8)
                        flagPF(1);
                    else
                        flagPF(0);
                    return;
                }
                else
                {
                    REAL_ADDR_SIZE c = 0;
                    REG_SIZE a = 0, b = 0;
                    signed short l1 = s->reg, l2 = f->reg, l = l2 - l1;
                    b = f->reg - s->reg;
                    a = f->reg_half[0] - s->reg_half[0];
                    if ((l1 & 0X0007) > (l2 & 0X0007)) // AF
                        flagAF(1);
                    else
                        flagAF(0);
                    if (l1 > l2) // CF
                        flagCF(1);
                    else
                        flagCF(0);
                    if ((l1 > 0 && l2 > 0 && l < 0) || (l1 > 0 && l2 > 0 && l < 0))
                        flagOF(1);
                    else
                        flagOF(0);
                    if (b) // ZF
                        flagZF(0);
                    else
                        flagZF(1);
                    int p = l >> 15;
                    flagSF(p); // SF
                    REGISTER tmp;
                    tmp.reg = f->reg - s->reg;
                    std::bitset<8> cc{tmp.reg_half[0]};
                    int sm = cc[0] + cc[1] + cc[2] + cc[3] + cc[4] + cc[5] + cc[6] + cc[7];
                    if (sm == 0 || sm == 2 || sm == 4 || sm == 6 || sm == 8)
                        flagPF(1);
                    else
                        flagPF(0);
                    return;
                }
            }
        }
    }
}

void cp8086::jmp(BYTE com)
{
    if (com == 0b01111111) // JG
    {
        if ((flags & 0X0020) == 0X0000 && ((flags & 0X0080) >> 6) == ((flags & 0X0800) >> 10))
        {
            printf("jmp completed\n");
            signed char disp = memory[real_addr + 1];
            ip.reg++;
            ip.reg += disp;
            return;
        }
        printf("jmp incompleted\n");
        ip.reg++;
        return;
    }
    if (com == 0b11100010) // loop
    {
        cx.reg--;
        if (cx.reg != 0)
        {
            printf("loop completed\n");
            signed char disp = memory[real_addr + 1];
            ip.reg++;
            ip.reg += disp;
            return;
        }
        printf("loop incompleted\n");
        ip.reg++;
        return;
    }
    if (com == 0b11101011) // jmp short
    {
        printf("jmp completed\n");
        signed char disp = memory[real_addr + 1];
        ip.reg++;
        ip.reg += disp;
        return;
    }
}
void cp8086::call(BYTE com)
{
    if(com==0XE8)
    {
        printf("jmp completed\n");
            REGISTER disp;
            disp.reg_half[1] = memory[real_addr + 2];
            disp.reg_half[0] = memory[real_addr + 1];
            ip.reg+=2;
            sp.reg -= 2;
            memory[local_convert_cs_ip_to_real(ss, sp) + 1] = ip.reg_half[1];
            memory[local_convert_cs_ip_to_real(ss, sp)] = ip.reg_half[0];
            ip.reg = (signed short)ip.reg+(signed short)disp.reg;
            return;
    }
}
void cp8086::ret(BYTE com)
{
    if(com==0b11000011)
    {
        ip.reg_half[1] = memory[local_convert_cs_ip_to_real(ss, sp) + 1];
        ip.reg_half[0] = memory[local_convert_cs_ip_to_real(ss, sp)];
        sp.reg += 2;
        return;
    }
}
