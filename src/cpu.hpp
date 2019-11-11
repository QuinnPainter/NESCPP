#pragma once

#include "memory.hpp"

enum class addrModes
{
    implied,
    accumulator,
    immediate,
    zeroPage,
    zeroPageX,
    zeroPageY,
    relative,
    absolute,
    absoluteX,
    absoluteY,
    indirect,
    indirectX, //indexed indirect
    indirectY  //indirect indexed
};

enum class instructions
{
    INVALID,
    ADC, AND, ASL, BCC, BCS, BEQ, BIT, BMI,
    BNE, BPL, BRK, BVC, BVS, CLC, CLD, CLI,
    CLV, CMP, CPX, CPY, DEC, DEX, DEY, EOR,
    INC, INX, INY, JMP, JSR, LDA, LDX, LDY,
    LSR, NOP, ORA, PHA, PHP, PLA, PLP, ROL,
    ROR, RTI, RTS, SBC, SEC, SED, SEI, STA,
    STX, STY, TAX, TAY, TSX, TXA, TXS, TYA,
};

struct opInfo
{
    instructions instr;
    addrModes addrMode;
    byte baseCycles; //amount of cycles used not counting extra ones for crossing pages or branching
};

struct cpuState
{
    //accumulator
    byte A;
    //index registers
    byte X;
    byte Y;
    //program counter
    ushort PC;
    //stack pointer
    byte SP;
    //status / flags register
    byte P;
};

enum class flags
{
    carry = 0,
    zero = 1,
    interrupt_disable = 2,
    decimal = 3,
    overflow = 6,
    negative = 7
};

class cpu
{
    public:
        cpu(memory* mem, bool* NMI, bool* IRQ);
        ~cpu();
        void reset();
        bool* NMILatch;
        bool* IRQLine;
        byte step();
    private:
        FILE* logFile;
        cpuState state;
        memory* Memory;
        void setFlag(flags flag, bool value);
        bool getFlag(flags flag);
        bool checkPageCrossed(ushort addr1, ushort addr2);
        void setZeroNegative(byte result);
        byte branch(bool cond, byte value);
        byte serviceInterrupt(bool brk = false);
        void stackPush(byte value);
        void stackPush16(ushort value);
        byte stackPull();
        ushort stackPull16();
        byte doOneOp();

        //hopefully this makes it a little cleaner
        #define INVALID_OPCODE(x) {instructions::INVALID, addrModes::implied, x}

        //doesn't include undocumented opcodes, may do this later
        const opInfo opcodes[256] = {
            {instructions::BRK, addrModes::implied, 0},         //00 saying it's 0 cycles as the interrupt code adds the 7 cycles it takes
            {instructions::ORA, addrModes::indirectX, 6},       //01
            INVALID_OPCODE(1),                                  //02
            INVALID_OPCODE(2),                                  //03
            INVALID_OPCODE(2),                                  //04
            {instructions::ORA, addrModes::zeroPage, 3},        //05
            {instructions::ASL, addrModes::zeroPage, 5},        //06
            INVALID_OPCODE(2),                                  //07
            {instructions::PHP, addrModes::implied, 3},         //08
            {instructions::ORA, addrModes::immediate, 2},       //09
            {instructions::ASL, addrModes::accumulator, 2},     //0a
            INVALID_OPCODE(2),                                  //0b
            INVALID_OPCODE(3),                                  //0c
            {instructions::ORA, addrModes::absolute, 4},        //0d
            {instructions::ASL, addrModes::absolute, 6},        //0e
            INVALID_OPCODE(3),                                  //0f
            {instructions::BPL, addrModes::relative, 2},        //10
            {instructions::ORA, addrModes::indirectY, 5},       //11
            INVALID_OPCODE(1),                                  //12
            INVALID_OPCODE(2),                                  //13
            INVALID_OPCODE(2),                                  //14
            {instructions::ORA, addrModes::zeroPageX, 4},       //15
            {instructions::ASL, addrModes::zeroPageX, 6},       //16
            INVALID_OPCODE(2),                                  //17
            {instructions::CLC, addrModes::implied, 2},         //18
            {instructions::ORA, addrModes::absoluteY, 4},       //19
            INVALID_OPCODE(1),                                  //1a
            INVALID_OPCODE(3),                                  //1b
            INVALID_OPCODE(3),                                  //1c
            {instructions::ORA, addrModes::absoluteX, 4},       //1d
            {instructions::ASL, addrModes::absoluteX, 7},       //1e
            INVALID_OPCODE(3),                                  //1f
            {instructions::JSR, addrModes::absolute, 6},        //20
            {instructions::AND, addrModes::indirectX, 6},       //21
            INVALID_OPCODE(1),                                  //22
            INVALID_OPCODE(2),                                  //23
            {instructions::BIT, addrModes::zeroPage, 3},        //24
            {instructions::AND, addrModes::zeroPage, 3},        //25
            {instructions::ROL, addrModes::zeroPage, 5},        //26
            INVALID_OPCODE(2),                                  //27
            {instructions::PLP, addrModes::implied, 4},         //28
            {instructions::AND, addrModes::immediate, 2},       //29
            {instructions::ROL, addrModes::accumulator, 2},     //2a
            INVALID_OPCODE(2),                                  //2b
            {instructions::BIT, addrModes::absolute, 4},        //2c
            {instructions::AND, addrModes::absolute, 4},        //2d
            {instructions::ROL, addrModes::absolute, 6},        //2e
            INVALID_OPCODE(3),                                  //2f
            {instructions::BMI, addrModes::relative, 2},        //30
            {instructions::AND, addrModes::indirectY, 5},       //31
            INVALID_OPCODE(1),                                  //32
            INVALID_OPCODE(2),                                  //33
            INVALID_OPCODE(2),                                  //34
            {instructions::AND, addrModes::zeroPageX, 4},       //35
            {instructions::ROL, addrModes::zeroPageX, 6},       //36
            INVALID_OPCODE(2),                                  //37
            {instructions::SEC, addrModes::implied, 2},         //38
            {instructions::AND, addrModes::absoluteY, 4},       //39
            INVALID_OPCODE(1),                                  //3a
            INVALID_OPCODE(3),                                  //3b
            INVALID_OPCODE(3),                                  //3c
            {instructions::AND, addrModes::absoluteX, 4},       //3d
            {instructions::ROL, addrModes::absoluteX, 7},       //3e
            INVALID_OPCODE(3),                                  //3f
            {instructions::RTI, addrModes::implied, 6},         //40
            {instructions::EOR, addrModes::indirectX, 6},       //41
            INVALID_OPCODE(1),                                  //42
            INVALID_OPCODE(2),                                  //43
            INVALID_OPCODE(2),                                  //44
            {instructions::EOR, addrModes::zeroPage, 3},        //45
            {instructions::LSR, addrModes::zeroPage, 5},        //46
            INVALID_OPCODE(2),                                  //47
            {instructions::PHA, addrModes::implied, 3},         //48
            {instructions::EOR, addrModes::immediate, 2},       //49
            {instructions::LSR, addrModes::accumulator, 2},     //4a
            INVALID_OPCODE(2),                                  //4b
            {instructions::JMP, addrModes::absolute, 3},        //4c
            {instructions::EOR, addrModes::absolute, 4},        //4d
            {instructions::LSR, addrModes::absolute, 6},        //4e
            INVALID_OPCODE(3),                                  //4f
            {instructions::BVC, addrModes::relative, 2},        //50
            {instructions::EOR, addrModes::indirectY, 5},       //51
            INVALID_OPCODE(1),                                  //52
            INVALID_OPCODE(2),                                  //53
            INVALID_OPCODE(2),                                  //54
            {instructions::EOR, addrModes::zeroPageX, 4},       //55
            {instructions::LSR, addrModes::zeroPageX, 6},       //56
            INVALID_OPCODE(2),                                  //57
            {instructions::CLI, addrModes::implied, 2},         //58
            {instructions::EOR, addrModes::absoluteY, 4},       //59
            INVALID_OPCODE(1),                                  //5a
            INVALID_OPCODE(3),                                  //5b
            INVALID_OPCODE(3),                                  //5c
            {instructions::EOR, addrModes::absoluteX, 4},       //5d
            {instructions::LSR, addrModes::absoluteX, 7},       //5e
            INVALID_OPCODE(3),                                  //5f
            {instructions::RTS, addrModes::implied, 6},         //60
            {instructions::ADC, addrModes::indirectX, 6},       //61
            INVALID_OPCODE(1),                                  //62
            INVALID_OPCODE(2),                                  //63
            INVALID_OPCODE(2),                                  //64
            {instructions::ADC, addrModes::zeroPage, 3},        //65
            {instructions::ROR, addrModes::zeroPage, 5},        //66
            INVALID_OPCODE(2),                                  //67
            {instructions::PLA, addrModes::implied, 4},         //68
            {instructions::ADC, addrModes::immediate, 2},       //69
            {instructions::ROR, addrModes::accumulator, 2},     //6a
            INVALID_OPCODE(2),                                  //6b
            {instructions::JMP, addrModes::indirect, 5},        //6c
            {instructions::ADC, addrModes::absolute, 4},        //6d
            {instructions::ROR, addrModes::absolute, 7},        //6e
            INVALID_OPCODE(3),                                  //6f
            {instructions::BVS, addrModes::relative, 2},        //70
            {instructions::ADC, addrModes::indirectY, 5},       //71
            INVALID_OPCODE(1),                                  //72
            INVALID_OPCODE(2),                                  //73
            INVALID_OPCODE(2),                                  //74
            {instructions::ADC, addrModes::zeroPageX, 4},       //75
            {instructions::ROR, addrModes::zeroPageX, 6},       //76
            INVALID_OPCODE(2),                                  //77
            {instructions::SEI, addrModes::implied, 2},         //78
            {instructions::ADC, addrModes::absoluteY, 4},       //79
            INVALID_OPCODE(1),                                  //7a
            INVALID_OPCODE(3),                                  //7b
            INVALID_OPCODE(3),                                  //7c
            {instructions::ADC, addrModes::absoluteX, 4},       //7d
            {instructions::ROR, addrModes::absoluteX, 6},       //7e
            INVALID_OPCODE(3),                                  //7f
            INVALID_OPCODE(2),                                  //80
            {instructions::STA, addrModes::indirectX, 6},       //81
            INVALID_OPCODE(2),                                  //82
            INVALID_OPCODE(2),                                  //83
            {instructions::STY, addrModes::zeroPage, 3},        //84
            {instructions::STA, addrModes::zeroPage, 3},        //85
            {instructions::STX, addrModes::zeroPage, 3},        //86
            INVALID_OPCODE(2),                                  //87
            {instructions::DEY, addrModes::implied, 2},         //88
            INVALID_OPCODE(2),                                  //89
            {instructions::TXA, addrModes::implied, 2},         //8a
            INVALID_OPCODE(2),                                  //8b
            {instructions::STY, addrModes::absolute, 4},        //8c
            {instructions::STA, addrModes::absolute, 4},        //8d
            {instructions::STX, addrModes::absolute, 4},        //8e
            INVALID_OPCODE(3),                                  //8f
            {instructions::BCC, addrModes::relative, 2},        //90
            {instructions::STA, addrModes::indirectY, 6},       //91
            INVALID_OPCODE(1),                                  //92
            INVALID_OPCODE(2),                                  //93
            {instructions::STY, addrModes::zeroPageX, 4},       //94
            {instructions::STA, addrModes::zeroPageX, 4},       //95
            {instructions::STX, addrModes::zeroPageY, 4},       //96
            INVALID_OPCODE(2),                                  //97
            {instructions::TYA, addrModes::implied, 2},         //98
            {instructions::STA, addrModes::absoluteY, 5},       //99
            {instructions::TXS, addrModes::implied, 2},         //9a
            INVALID_OPCODE(1),                                  //9b TAS byte count unknown?
            INVALID_OPCODE(3),                                  //9c
            {instructions::STA, addrModes::absoluteX, 5},       //9d
            INVALID_OPCODE(3),                                  //9e
            INVALID_OPCODE(3),                                  //9f
            {instructions::LDY, addrModes::immediate, 2},       //a0
            {instructions::LDA, addrModes::indirectX, 6},       //a1
            {instructions::LDX, addrModes::immediate, 2},       //a2
            INVALID_OPCODE(2),                                  //a3
            {instructions::LDY, addrModes::zeroPage, 3},        //a4
            {instructions::LDA, addrModes::zeroPage, 3},        //a5
            {instructions::LDX, addrModes::zeroPage, 3},        //a6
            INVALID_OPCODE(2),                                  //a7
            {instructions::TAY, addrModes::implied, 2},         //a8
            {instructions::LDA, addrModes::immediate, 2},       //a9
            {instructions::TAX, addrModes::implied, 2},         //aa
            INVALID_OPCODE(2),                                  //ab
            {instructions::LDY, addrModes::absolute, 4},        //ac
            {instructions::LDA, addrModes::absolute, 4},        //ad
            {instructions::LDX, addrModes::absolute, 4},        //ae
            INVALID_OPCODE(3),                                  //af
            {instructions::BCS, addrModes::relative, 2},        //b0
            {instructions::LDA, addrModes::indirectY, 5},       //b1
            INVALID_OPCODE(1),                                  //b2
            INVALID_OPCODE(2),                                  //b3
            {instructions::LDY, addrModes::zeroPageX, 4},       //b4
            {instructions::LDA, addrModes::zeroPageX, 4},       //b5
            {instructions::LDX, addrModes::zeroPageY, 4},       //b6
            INVALID_OPCODE(2),                                  //b7
            {instructions::CLV, addrModes::implied, 2},         //b8
            {instructions::LDA, addrModes::absoluteY, 4},       //b9
            {instructions::TSX, addrModes::implied, 2},         //ba
            INVALID_OPCODE(3),                                  //bb
            {instructions::LDY, addrModes::absoluteX, 4},       //bc
            {instructions::LDA, addrModes::absoluteX, 4},       //bd
            {instructions::LDX, addrModes::absoluteY, 4},       //be
            INVALID_OPCODE(3),                                  //bf
            {instructions::CPY, addrModes::immediate, 2},       //c0
            {instructions::CMP, addrModes::indirectX, 6},       //c1
            INVALID_OPCODE(2),                                  //c2
            INVALID_OPCODE(2),                                  //c3
            {instructions::CPY, addrModes::zeroPage, 3},        //c4
            {instructions::CMP, addrModes::zeroPage, 3},        //c5
            {instructions::DEC, addrModes::zeroPage, 5},        //c6
            INVALID_OPCODE(2),                                  //c7
            {instructions::INY, addrModes::implied, 2},         //c8
            {instructions::CMP, addrModes::immediate, 2},       //c9
            {instructions::DEX, addrModes::implied, 2},         //ca
            INVALID_OPCODE(2),                                  //cb
            {instructions::CPY, addrModes::absolute, 4},        //cc
            {instructions::CMP, addrModes::absolute, 4},        //cd
            {instructions::DEC, addrModes::absolute, 3},        //ce
            INVALID_OPCODE(3),                                  //cf
            {instructions::BNE, addrModes::relative, 2},        //d0
            {instructions::CMP, addrModes::indirectY, 5},       //d1
            INVALID_OPCODE(1),                                  //d2
            INVALID_OPCODE(2),                                  //d3
            INVALID_OPCODE(2),                                  //d4
            {instructions::CMP, addrModes::zeroPageX, 4},       //d5
            {instructions::DEC, addrModes::zeroPageX, 6},       //d6
            INVALID_OPCODE(2),                                  //d7
            {instructions::CLD, addrModes::implied, 2},         //d8
            {instructions::CMP, addrModes::absoluteY, 4},       //d9
            INVALID_OPCODE(1),                                  //da
            INVALID_OPCODE(3),                                  //db
            INVALID_OPCODE(3),                                  //dc
            {instructions::CMP, addrModes::absoluteX, 4},       //dd
            {instructions::DEC, addrModes::absoluteX, 7},       //de
            INVALID_OPCODE(3),                                  //df
            {instructions::CPX, addrModes::immediate, 2},       //e0
            {instructions::SBC, addrModes::indirectX, 6},       //e1
            INVALID_OPCODE(2),                                  //e2
            INVALID_OPCODE(2),                                  //e3
            {instructions::CPX, addrModes::zeroPage, 3},        //e4
            {instructions::SBC, addrModes::zeroPage, 3},        //e5
            {instructions::INC, addrModes::zeroPage, 5},        //e6
            INVALID_OPCODE(2),                                  //e7
            {instructions::INX, addrModes::implied, 2},         //e8
            {instructions::SBC, addrModes::immediate, 2},       //e9
            {instructions::NOP, addrModes::implied, 2},         //ea
            INVALID_OPCODE(2),                                  //eb
            {instructions::CPX, addrModes::absolute, 4},        //ec
            {instructions::SBC, addrModes::absolute, 4},        //ed
            {instructions::INC, addrModes::absolute, 6},        //ee
            INVALID_OPCODE(3),                                  //ef
            {instructions::BEQ, addrModes::relative, 2},        //f0
            {instructions::SBC, addrModes::indirectY, 5},       //f1
            INVALID_OPCODE(1),                                  //f2
            INVALID_OPCODE(2),                                  //f3
            INVALID_OPCODE(2),                                  //f4  
            {instructions::SBC, addrModes::zeroPageX, 4},       //f5
            {instructions::INC, addrModes::zeroPageX, 6},       //f6
            INVALID_OPCODE(2),                                  //f7
            {instructions::SED, addrModes::implied, 2},         //f8
            {instructions::SBC, addrModes::absoluteY, 4},       //f9
            INVALID_OPCODE(1),                                  //fa
            INVALID_OPCODE(3),                                  //fb
            INVALID_OPCODE(3),                                  //fc
            {instructions::SBC, addrModes::absoluteX, 4},       //fd
            {instructions::INC, addrModes::absoluteX, 7},       //fe
            INVALID_OPCODE(3)                                   //ff
        };
        #undef INVALID_OPCODE
};