#include "../includes.hpp"
#include "mapper2.hpp"

mapper2::mapper2(byte* PRG, unsigned int PRGSize, byte* CHR, unsigned int CHRSize, bool m)
{
    PRGROM = PRG;
    PRGROMSize = PRGSize;
    CHRROM = CHR;
    CHRROMSize = CHRSize;
    mirroring = m;
    //just assume it has max RAM
    cartRAM = new byte[8192];
    if (CHRROMSize == 0)
    {
        CHRRAM = new byte[8192];
        memset(CHRRAM, 0, 8192);
    }
    memset(cartRAM, 0, 8192);
}

mapper2::~mapper2()
{
    if (CHRROMSize == 0)
    {
        delete[] CHRRAM;
    }
    delete[] cartRAM;
}

byte mapper2::getCPU(ushort address)
{
    if (address < 0x4020)
    {
        logging::logerr("Tried to access non cart memory: " + ushortToString(address) + " through the mapper, something is wrong with the MMU");
        return 0;
    }
    else if (address < 0x6000)
    {
        //unused area
        return 0xFF;
    }
    else if (address < 0x8000)
    {
        return cartRAM[address - 0x6000];
    }
    else if (address < 0xC000)
    {
        return PRGROM[(address - 0x8000) + (bankSelect * 0x4000)];
    }
    else if (address <= 0xFFFF)
    {
        return PRGROM[(PRGROMSize - 0x8000) + (address - 0x8000)];
    }
    //this can (hopefully) never happen, just keeping compiler happy
    logging::logerr("How did this happen??? (mapper2), addr: " + ushortToString(address), true);
    return 0;
}

void mapper2::setCPU(ushort address, byte value)
{
    if (address >= 0x6000 && address < 0x8000)
    {
        cartRAM[address - 0x6000] = value;
    }
    else if (address >= 0x8000)
    {
        bankSelect = value & 0xF;
    }
}

byte mapper2::getPPU(ushort address)
{
    address &= 0x3FFF; //mirroring
    //pattern tables
    if (address < 0x2000)
    {
        if (CHRROMSize > 0)
        {
            return CHRROM[address];
        }
        else
        {
            return CHRRAM[address];
        }
    }
    //nametables
    else if (address < 0x3F00)
    {
        return nametableRAM[getNametableRAMIndex(address)];
    }
    logging::logerr("invalid PPU address in mapper2: " + ushortToString(address));
    return 0;
}

void mapper2::setPPU(ushort address, byte value)
{
    address &= 0x3FFF; //mirroring
    //pattern tables
    if (address < 0x2000)
    {
        if (CHRROMSize == 0)
        {
            CHRRAM[address] = value;
        }
    }
    //nametables
    else if (address < 0x3F00)
    {
        nametableRAM[getNametableRAMIndex(address)] = value;
    }
}

ushort mapper2::getNametableRAMIndex(ushort address)
{
    if (address > 0x2FFF)
    {
        address -= 0x1000;
    }
    if (mirroring)
    {
        //vertical mirroring
        if (address > 0x27FF)
        {
            address -= 0x800;
        }
    }
    else
    {
        //horizontal mirroring
        if (address > 0x23FF && address < 0x2800)
        {
            address -= 0x400;
        }
        else if (address > 0x27FF && address < 0x2C00)
        {
            address -= 0x400;
        }
        else if (address > 0x2BFF)
        {
            address -= 0x800;
        }
    }
    return address - 0x2000;
}