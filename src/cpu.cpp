#include "includes.hpp"
#include "cpu.hpp"

cpu::cpu(memory* mem, bool* NMI, bool* IRQ)
{
    Memory = mem;
    NMILatch = NMI;
    IRQLine = IRQ;
    #ifdef LOG
    logFile = fopen("nesemu.log", "w");
    #endif
}

cpu::~cpu()
{
    #ifdef LOG
    fclose(logFile);
    #endif
}

void cpu::reset()
{
    //maybe at some point I should start treating reset as an interrupt?
    state =
    {
        0,    //A
        0,    //X
        0,    //Y
        Memory->get16(0xFFFC), //PC
        0xFD, //SP
        0x34  //P
    };

    #ifdef NESTEST
    state.PC = 0xC000;
    state.P = 0x24;
    #endif
}

void cpu::setFlag(flags flag, bool value)
{
    state.P = setBit(state.P, (byte)flag, value);
}

bool cpu::getFlag(flags flag)
{
    return getBit(state.P, (byte)flag);
}

bool cpu::checkPageCrossed(ushort addr1, ushort addr2)
{
    return (addr1 & 0xFF00) != (addr2 & 0xFF00);
}

void cpu::setZeroNegative(byte result)
{
    setFlag(flags::zero, result == 0);
    setFlag(flags::negative, getBit(result, 7));
}

byte cpu::branch(bool cond, byte value)
{
    if (!cond)
    {
        return 0;
    }
    byte cycles = 0;
    sbyte offset = sbyte(value);
    cycles += 1;
    if (checkPageCrossed(state.PC, state.PC + offset))
    {
        cycles += 1;
    }
    state.PC += offset;
    return cycles;
}

byte cpu::serviceInterrupt(bool brk)
{
    ushort jumpVector = 0;
    if (brk)
    {
        jumpVector = Memory->get16(0xFFFE);
        state.PC += 1;
    }
    else if (*NMILatch)
    {
        jumpVector = Memory->get16(0xFFFA);
        *NMILatch = false;
    }
    else if (*IRQLine)
    {
        //"else if" because NMI takes precedence if both are active
        if (getFlag(flags::interrupt_disable))
        {
            return 0;
        }
        jumpVector = Memory->get16(0xFFFE);
    }
    else
    {
        //no interrupt pending
        return 0; 
    }
    stackPush16(state.PC);
    state.P = setBit(state.P, 4, brk);
    state.P = setBit(state.P, 5, true);
    stackPush(state.P);
    setFlag(flags::interrupt_disable, true);
    state.PC = jumpVector;
    return 7; //interrupts take 7 cycles
}

void cpu::stackPush(byte value)
{
    if (state.SP == 0)
    {
        logging::log("Stack overflow!", true, true);
    }
    Memory->set((ushort)state.SP + 0x0100, value);
    state.SP--;
}

void cpu::stackPush16(ushort value)
{
    stackPush(highByte(value));
    stackPush(lowByte(value));
}

byte cpu::stackPull()
{
    if (state.SP == 0xFF)
    {
        logging::log("Stack underflow!", true, true);
    }
    state.SP++;
    return Memory->get((ushort)state.SP + 0x0100);
}

ushort cpu::stackPull16()
{
    byte LSB = stackPull();
    byte MSB = stackPull();
    return combineBytes(MSB, LSB);
}

byte cpu::doOneOp()
{
    byte opByte = Memory->get(state.PC);
    opInfo opcode = opcodes[opByte];
    byte cycles = opcode.baseCycles;
    //the value / address to perform the instruction on
    ushort value = 0;
    switch (opcode.addrMode)
    {
        case (addrModes::accumulator):
            value = state.A;
            state.PC += 1;
            break;
        case (addrModes::immediate):
            value = state.PC + 1;
            state.PC += 2;
            break;
        case (addrModes::implied):
            state.PC += 1;
            break;
        case (addrModes::relative):
            value = Memory->get(state.PC + 1);
            state.PC += 2;
            break;
        case (addrModes::absolute):
            value = Memory->get16(state.PC + 1);
            state.PC += 3;
            break;
        case (addrModes::zeroPage):
            value = Memory->get(state.PC + 1);
            state.PC += 2;
            break;
        case (addrModes::indirect):
            value = Memory->get16bugged(Memory->get16(state.PC + 1));
            state.PC += 3;
            break;
        case (addrModes::absoluteX):
            value = Memory->get16(state.PC + 1) + state.X;
            cycles += checkPageCrossed(value, Memory->get16(state.PC + 1));
            state.PC += 3;
            break;
        case (addrModes::absoluteY):
            value = Memory->get16(state.PC + 1) + state.Y;
            cycles += checkPageCrossed(value, Memory->get16(state.PC + 1));
            state.PC += 3;
            break;
        case (addrModes::zeroPageX):
            value = (byte)(Memory->get(state.PC + 1) + state.X);
            state.PC += 2;
            break;
        case (addrModes::zeroPageY):
            value = (byte)(Memory->get(state.PC + 1) + state.Y);
            state.PC += 2;
            break;
        case (addrModes::indirectX):
            value = Memory->get16bugged((byte)(Memory->get(state.PC + 1) + state.X));
            state.PC += 2;
            break;
        case (addrModes::indirectY):
            value = Memory->get16bugged(Memory->get(state.PC + 1)) + state.Y;
            cycles += checkPageCrossed(value, Memory->get16(Memory->get(state.PC + 1)));
            state.PC += 2;
            break;
    }

    switch (opcode.instr)
    {
        case instructions::ADC: //Add with Carry
        {
            byte toAdd = Memory->get(value);
            bool carry = getFlag(flags::carry);
            ushort result = state.A + toAdd + carry;
            setFlag(flags::carry, result > 0xFF);
            //setFlag(flags::overflow, (state.A^(byte)result)&(toAdd^(byte)result)&0x80);
            setFlag(flags::overflow, (~(state.A ^ toAdd))&(state.A ^ (byte)result)&0x80);
            setZeroNegative((byte)result);
            state.A = (byte)result;
            break;
        }
        case instructions::AND: //Logical AND
        {
            state.A &= Memory->get(value);
            setZeroNegative(state.A);
            break;
        }
        case instructions::ASL: //Arithmetic Shift Left
        {
            if (opcode.addrMode == addrModes::accumulator)
            {
                setFlag(flags::carry, getBit(state.A, 7));
                state.A <<= 1;
                setZeroNegative(state.A);
            }
            else
            {
                setFlag(flags::carry, getBit(Memory->get(value), 7));
                byte newValue = Memory->get(value) << 1;
                setZeroNegative(newValue);
                Memory->set(value, newValue);
            }
            break;
        }
        case instructions::BCC: //Branch if Carry Clear
        {
            cycles += branch(!getFlag(flags::carry), value);
            break;
        }
        case instructions::BCS: //Branch if Carry Set
        {
            cycles += branch(getFlag(flags::carry), value);
            break;
        }
        case instructions::BEQ: //Branch if Equal
        {
            cycles += branch(getFlag(flags::zero), value);
            break;
        }
        case instructions::BIT: //Bit Test
        {
            byte mem = Memory->get(value);
            byte result = state.A & mem;
            setFlag(flags::zero, result == 0);
            setFlag(flags::overflow, getBit(mem, 6));
            setFlag(flags::negative, getBit(mem, 7));
            break;
        }
        case instructions::BMI: //Branch if Minus
        {
            cycles += branch(getFlag(flags::negative), value);
            break;
        }
        case instructions::BNE: //Branch if Not Equal
        {
            cycles += branch(!getFlag(flags::zero), value);
            break;
        }
        case instructions::BPL: //Branch if Positive
        {
            cycles += branch(!getFlag(flags::negative), value);
            break;
        }
        case instructions::BRK: //Force Interrupt
        {
            cycles += serviceInterrupt(true);
            break;
        }
        case instructions::BVC: //Branch if Overflow Clear
        {
            cycles += branch(!getFlag(flags::overflow), value);
            break;
        }
        case instructions::BVS: //Branch if Overflow Set
        {
            cycles += branch(getFlag(flags::overflow), value);
            break;
        }
        case instructions::CLC: //Clear Carry Flag
        {
            setFlag(flags::carry, 0);
            break;
        }
        case instructions::CLD: //Clear Decimal Flag
        {
            setFlag(flags::decimal, 0);
            break;
        }
        case instructions::CLI: //Clear Interrupt Disable
        {
            setFlag(flags::interrupt_disable, 0);
            break;
        }
        case instructions::CLV: //Clear Overflow Flag
        {
            setFlag(flags::overflow, 0);
            break;
        }
        case instructions::CMP: //Compare
        {
            byte toCompare = Memory->get(value);
            setZeroNegative(state.A - toCompare);
            setFlag(flags::carry, state.A >= toCompare);
            break;
        }
        case instructions::CPX: //Compare X
        {
            byte toCompare = Memory->get(value);
            setZeroNegative(state.X - toCompare);
            setFlag(flags::carry, state.X >= toCompare);
            break;
        }
        case instructions::CPY: //Compare Y
        {
            byte toCompare = Memory->get(value);
            setZeroNegative(state.Y - toCompare);
            setFlag(flags::carry, state.Y >= toCompare);
            break;
        }
        case instructions::DEC: //Decrement Memory
        {
            byte finalValue = Memory->get(value) - 1;
            setZeroNegative(finalValue);
            Memory->set(value, finalValue);
            break;
        }
        case instructions::DEX: //Decrement X
        {
            state.X--;
            setZeroNegative(state.X);
            break;
        }
        case instructions::DEY: //Decrement Y
        {
            state.Y--;
            setZeroNegative(state.Y);
            break;
        }
        case instructions::EOR: //Exclusive OR
        {
            state.A ^= Memory->get(value);
            setZeroNegative(state.A);
            break;
        }
        case instructions::INC: //Increment Memory
        {
            byte finalValue = Memory->get(value) + 1;
            setZeroNegative(finalValue);
            Memory->set(value, finalValue);
            break;
        }
        case instructions::INX: //Increment X
        {
            state.X++;
            setZeroNegative(state.X);
            break;
        }
        case instructions::INY: //Increment Y
        {
            state.Y++;
            setZeroNegative(state.Y);
            break;
        }
        case instructions::JMP: //Jump
        {
            state.PC = value;
            break;
        }
        case instructions::JSR: //Jump to Subroutine
        {
            stackPush16(state.PC - 1);
            state.PC = value;
            break;
        }
        case instructions::LDA: //Load Accumulator
        {
            state.A = Memory->get(value);
            setZeroNegative(state.A);
            break;
        }
        case instructions::LDX: //Load X
        {
            state.X = Memory->get(value);
            setZeroNegative(state.X);
            break;
        }
        case instructions::LDY: //Load Y
        {
            state.Y = Memory->get(value);
            setZeroNegative(state.Y);
            break;
        }
        case instructions::LSR: //Logical Shift Right
        {
            if (opcode.addrMode == addrModes::accumulator)
            {
                setFlag(flags::carry, getBit(state.A, 0));
                state.A >>= 1;
                setZeroNegative(state.A);
            }
            else
            {
                setFlag(flags::carry, getBit(Memory->get(value), 0));
                byte newValue = Memory->get(value) >> 1;
                setZeroNegative(newValue);
                Memory->set(value, newValue);
            }
            break;
        }
        case instructions::NOP: //No Operation
        {
            break;
        }
        case instructions::ORA: //Logical Inclusive OR
        {
            state.A |= Memory->get(value);
            setZeroNegative(state.A);
            break;
        }
        case instructions::PHA: //Push Accumulator
        {
            stackPush(state.A);
            break;
        }
        case instructions::PHP: //Push Processor Status
        {
            //make sure the unused flag is always set
            state.P = setBit(state.P, 5, 1);
            //for the stack set the B flag but don't actually save it
            stackPush(setBit(state.P, 4, 1));
            break;
        }
        case instructions::PLA: //Pull Accumulator
        {
            state.A = stackPull();
            setZeroNegative(state.A);
            break;
        }
        case instructions::PLP: //Pull Processor Status
        {
            state.P = stackPull();
            //nestest doesn't like the B flag being on, but it doesn't matter
            state.P = setBit(state.P, 4, 0);
            //make sure the unused flag is always set
            state.P = setBit(state.P, 5, 1);
            break;
        }
        case instructions::ROL: //Rotate Left
        {
            if (opcode.addrMode == addrModes::accumulator)
            {
                bool oldCarry = getFlag(flags::carry);
                setFlag(flags::carry, getBit(state.A, 7));
                state.A <<= 1;
                state.A = setBit(state.A, 0, oldCarry);
                setZeroNegative(state.A);
            }
            else
            {
                byte memValue = Memory->get(value);
                bool oldCarry = getFlag(flags::carry);
                setFlag(flags::carry, getBit(memValue, 7));
                memValue <<= 1;
                memValue = setBit(memValue, 0, oldCarry);
                setZeroNegative(memValue);
                Memory->set(value, memValue);
            }
            break;
        }
        case instructions::ROR: //Rotate Right
        {
            if (opcode.addrMode == addrModes::accumulator)
            {
                bool oldCarry = getFlag(flags::carry);
                setFlag(flags::carry, getBit(state.A, 0));
                state.A >>= 1;
                state.A = setBit(state.A, 7, oldCarry);
                setZeroNegative(state.A);
            }
            else
            {
                byte memValue = Memory->get(value);
                bool oldCarry = getFlag(flags::carry);
                setFlag(flags::carry, getBit(memValue, 0));
                memValue >>= 1;
                memValue = setBit(memValue, 7, oldCarry);
                setZeroNegative(memValue);
                Memory->set(value, memValue);
            }
            break;
        }
        case instructions::RTI: //Return from Interrupt
        {
            state.P = stackPull();
            state.PC = stackPull16();
            //nestest doesn't like the B flag being on, but it doesn't matter
            state.P = setBit(state.P, 4, 0);
            //make sure the unused flag is always set
            state.P = setBit(state.P, 5, 1);
            break;
        }
        case instructions::RTS: //Return from Subroutine
        {
            state.PC = stackPull16() + 1;
            break;
        }
        case instructions::SBC: //Subtract with Carry
        {
            byte initialA = state.A;
            byte memValue = Memory->get(value);
            bool carry = getFlag(flags::carry);
            state.A = (initialA - memValue - (1 - carry));
            setZeroNegative(state.A);
            setFlag(flags::carry, int(initialA)-int(memValue)-int(1-carry) >= 0);
            setFlag(flags::overflow, (~(initialA ^ ~memValue))&(initialA ^ state.A)&0x80);
            //setFlag(flags::overflow, (initialA^memValue)&0x80 != 0 && (initialA^state.A)&0x80 != 0);
            break;
        }
        case instructions::SEC: //Set Carry Flag
        {
            setFlag(flags::carry, 1);
            break;
        }
        case instructions::SED: //Set Decimal Flag
        {
            setFlag(flags::decimal, 1);
            break;
        }
        case instructions::SEI: //Set Interrupt Disable
        {
            setFlag(flags::interrupt_disable, 1);
            break;
        }
        case instructions::STA: //Store Accumulator
        {
            Memory->set(value, state.A);
            break;
        }
        case instructions::STX: //Store X
        {
            Memory->set(value, state.X);
            break;
        }
        case instructions::STY: //Store Y
        {
            Memory->set(value, state.Y);
            break;
        }
        case instructions::TAX: //Transfer Accumulator to X
        {
            state.X = state.A;
            setZeroNegative(state.X);
            break;
        }
        case instructions::TAY: //Transfer Accumulator to Y
        {
            state.Y = state.A;
            setZeroNegative(state.Y);
            break;
        }
        case instructions::TSX: //Transfer Stack Pointer to X
        {
            state.X = state.SP;
            setZeroNegative(state.X);
            break;
        }
        case instructions::TXA: //Transfer X to Accumulator
        {
            state.A = state.X;
            setZeroNegative(state.A);
            break;
        }
        case instructions::TXS: //Transfer X to Stack Pointer
        {
            state.SP = state.X;
            break;
        }
        case instructions::TYA: //Transfer Y to Accumulator
        {
            state.A = state.Y;
            setZeroNegative(state.Y);
            break;
        }
        case instructions::INVALID:
        {
            logging::log("Invalid opcode: " + byteToString(opByte) + " PC: " + ushortToString(state.PC - 1), true, true);
            //hijack cycle count for the opcode length
            //this means all invalid opcodes take the same amount of cycles as they have bytes
            //hopefully this doesn't matter
            state.PC += (opcode.baseCycles - 1);
            break;
        }
        default:
        {
            logging::log("Unimplemented opcode: " + byteToString(opByte), true, true);
            break;
        }
    }
    return cycles;
}

byte cpu::step()
{
    #ifdef NESTEST
    if (state.PC == 0xC66E)
    {
        logging::log("Nestest is now over.");
        logging::log("0x02 = " + byteToString(Memory->get(0x0002)));
        logging::log("0x03 = " + byteToString(Memory->get(0x0003)));
        exit(0);
    }
    #endif
    #ifdef INSTRTEST
    byte mem = Memory->get(0x6000);
    if (mem == 0x81)
    {
        logging::log("press reset");
    }
    if (mem == 0x80)
    {
        //logging::log("test running");
    }
    if (mem < 0x80 && Memory->get(0x6001) == 0xDE && Memory->get(0x6002) == 0xB0 && Memory->get(0x6003) == 0x61)
    {
        logging::log("test done");
        byte letter = 5;
        ushort counter = 0;
        std::string output = "";
        while (letter != 0)
        {
            letter = Memory->get(0x6004 + counter);
            output += letter;
            counter++;
        }
        logging::log(output);
        SDL_Quit();
        exit(0);
    }
    #endif

    #ifdef LOG
    std::string output = "";
    output += ushortToString(state.PC);
    output += " " + byteToString(Memory->get(state.PC));
    output += " A:" + byteToString(state.A);
    output += " X:" + byteToString(state.X);
    output += " Y:" + byteToString(state.Y);
    output += " P:" + byteToString(state.P);
    output += " SP:" + byteToString(state.SP) + "\n";
    char c [output.length() + 1];
    output.copy(c, output.length(), 0);
    c[output.length()] = '\0'; //zero terminator
    fprintf(logFile, "%s", c);
    //logging::log(" SP: " + ushortToString((ushort)state.SP + 0x100), false);
    //getchar();
    #endif
    byte cycles = 0;
    cycles += serviceInterrupt();
    cycles += cpu::doOneOp();
    return cycles;
}