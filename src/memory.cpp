#include "includes.hpp"
#include "memory.hpp"

memory::memory(mapper* m, ppu* p, input* i, apu* a, bool* dO)
{
    Mapper = m;
    PPU = p;
    Input = i;
    APU = a;
    didOAM = dO;
    ram = new byte[2048];
    memset(ram, 0, 2048);
}

memory::~memory()
{
    delete[] ram;
}

byte memory::get(ushort address)
{
    ushort convertedAddr = mirrorConvert(address);
    if (convertedAddr < 0x0800)
    {
        return ram[convertedAddr];
    }
    if (convertedAddr >= 0x2000 && convertedAddr < 0x2008)
    {
        return PPU->getRegister(convertedAddr);
    }
    if (convertedAddr > 0x401F)
    {
        return Mapper->getCPU(address);
    }
    if (convertedAddr == 0x4016)
    {
        return Input->registerRead();
    }
    if ((convertedAddr >= 0x4000 && convertedAddr <= 0x4013) || convertedAddr == 0x4015 || convertedAddr == 0x4017)
    {
        return APU->getRegister(convertedAddr);
    }

    //accessed unimplemented area of memory
    return 0;
}

void memory::set(ushort address, byte value)
{
    ushort convertedAddr = mirrorConvert(address);
    if (convertedAddr < 0x0800)
    {
        ram[convertedAddr] = value;
    }
    if (convertedAddr >= 0x2000 && convertedAddr < 0x2008)
    {
        PPU->setRegister(convertedAddr, value);
    }
    if (convertedAddr > 0x401F)
    {
        Mapper->setCPU(convertedAddr, value);
    }
    if (convertedAddr == 0x4016)
    {
        Input->registerWrite(value);
    }
    if (convertedAddr == 0x4014)
    {
        *didOAM = true;
        for (int i = 0; i < 256; i++)
        {
            tempOAM[i] = get(((ushort)value << 8) | (byte)i);
        }
        PPU->OAMDMA(tempOAM);
    }
    if ((convertedAddr >= 0x4000 && convertedAddr <= 0x4013) || convertedAddr == 0x4015 || convertedAddr == 0x4017)
    {
        APU->setRegister(convertedAddr, value);
    }
    //logging::log("Set " + ushortToString(address) + " to " + byteToString(value));
}

ushort memory::get16(ushort address)
{
    return combineBytes(get(address + 1), get(address));
}

//Bugged version of get16 that wraps when address is at the end of a page
ushort memory::get16bugged(ushort address)
{
    if ((address & 0x00FF) == 0x00FF)
    {
        return combineBytes(get(address & 0xFF00), get(address));
    }
    return get16(address);
}

void memory::set16(ushort address, ushort value)
{
    set(address, lowByte(value));
    set(address + 1, highByte(value));
}

ushort memory::mirrorConvert(ushort address)
{
    if (address > 0x07FF && address < 0x1000)
    {
        //RAM mirror 1
        return address - 0x0800;
    }
    else if (address > 0x0FFF && address < 0x1800)
    {
        //RAM mirror 2
        return address - 0x1000;
    }
    else if (address > 0x17FF && address < 0x2000)
    {
        //RAM mirror 3
        return address - 0x1800;
    }
    else if (address > 0x2007 && address < 0x4000)
    {
        //PPU register mirrors
        return 0x2000 + (address % 8);
    }
    return address;
}