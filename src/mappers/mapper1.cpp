#include "../includes.hpp"
#include "mapper1.hpp"

mapper1::mapper1(byte* PRG, unsigned int PRGSize, byte* CHR, unsigned int CHRSize)
{
    PRGROM = PRG;
    PRGROMSize = PRGSize;
    CHRROM = CHR;
    CHRROMSize = CHRSize;
    //just assume it has max RAM
    cartRAM = new byte[8192];
    if (CHRROMSize == 0)
    {
        CHRRAM = new byte[8192];
        memset(CHRRAM, 0, 8192);
    }
    memset(cartRAM, 0, 8192);
}

mapper1::~mapper1()
{
    if (CHRROMSize == 0)
    {
        delete[] CHRRAM;
    }
    delete[] cartRAM;
}

byte mapper1::getCPU(ushort address)
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
        //Bank Area 1
        switch (PRGBankMode)
        {
            case 0: case 1:
            {
                return PRGROM[(address - 0x8000) + (0x8000 * (PRGBank & 0xE))];
                break;
            }
            case 2:
            {
                return PRGROM[(address - 0x8000)];
                break;
            }
            case 3:
            {
                return PRGROM[(address - 0x8000) + (0x4000 * PRGBank)];
                break;
            }
        }
    }
    else if (address <= 0xFFFF)
    {
        //Bank Area 2
        switch (PRGBankMode)
        {
            case 0: case 1:
            {
                return PRGROM[(address - 0x8000) + (0x8000 * (PRGBank & 0xE))];
                break;
            }
            case 2:
            {
                return PRGROM[(address - 0x8000) + (0x4000 * PRGBank)];
                break;
            }
            case 3:
            {
                return PRGROM[(address - 0x8000) + (PRGROMSize - 0x8000)];
                break;
            }
        }
    }
    //this can (hopefully) never happen, just keeping compiler happy
    logging::logerr("How did this happen??? (mapper1), addr: " + ushortToString(address), true);
    return 0;
}

void mapper1::setCPU(ushort address, byte value)
{
    if (address >= 0x6000 && address < 0x8000)
    {
        cartRAM[address - 0x6000] = value;
    }
    else if (address >= 0x8000)
    {
        if (getBit(value, 7))
        {
            shiftReg = 0x10;
            PRGBankMode = 3;
            return;
        }
        bool regFull = ((shiftReg & 1) == 1);
        shiftReg >>= 1;
        shiftReg = setBit(shiftReg, 4, value & 1);
        if (regFull)
        {
            if (address < 0xA000)
            {
                //control
                mirroringType = shiftReg & 0x3;
                PRGBankMode = (shiftReg >> 2) & 0x3;
                CHRBankMode = getBit(shiftReg, 4);
            }
            else if (address < 0xC000)
            {
                //CHR bank 0
                CHRBank0 = shiftReg;
                if (CHRROMSize <= 8192)
                {
                    CHRBank0 &= 1;
                }
            }
            else if (address < 0xE000)
            {
                //CHR bank 1
                CHRBank1 = shiftReg;
                if (CHRROMSize <= 8192)
                {
                    CHRBank1 &= 1;
                }
            }
            else//if (address <= 0xFFFF)
            {
                //PRG bank
                PRGBank = shiftReg & 0xF;
            }
            shiftReg = 0x10;
        }
    }
}

byte mapper1::getPPU(ushort address)
{
    address &= 0x3FFF; //mirroring
    //pattern tables
    if (address < 0x2000)
    {
        if (CHRROMSize > 0)
        {
            if (CHRBankMode)
            {
                if (address < 0x1000)
                {
                    return CHRROM[address + (0x1000 * CHRBank0)];
                }
                else
                {
                    return CHRROM[(address - 0x1000) + (0x1000 * CHRBank1)];
                }
            }
            else
            {
                return CHRROM[address + (0x2000 * (CHRBank0 & 0x30))];
            }
        }
        else
        {
            if (CHRBankMode)
            {
                if (address < 0x1000)
                {
                    return CHRRAM[address + (0x1000 * CHRBank0)];
                }
                else
                {
                    return CHRRAM[(address - 0x1000) + (0x1000 * CHRBank1)];
                }
            }
            else
            {
                return CHRRAM[address];
            }
        }
    }
    //nametables
    else if (address < 0x3F00)
    {
        return nametableRAM[getNametableRAMIndex(address)];
    }
    logging::logerr("invalid PPU address in mapper1: " + ushortToString(address));
    return 0;
}

void mapper1::setPPU(ushort address, byte value)
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

ushort mapper1::getNametableRAMIndex(ushort address)
{
    if (address > 0x2FFF)
    {
        address -= 0x1000;
    }
    switch (mirroringType)
    {
        case 0:
        {
            //single-screen, lower bank
            while (address >= 0x2400)
            {
                address -= 0x400;
            }
            break;
        }
        case 1:
        {
            //single-screen, upper bank
            while (address >= 0x2400)
            {
                address -= 0x400;
            }
            address += 0x400;
            break;
        }
        case 2:
        {
            //vertical mirroring
            if (address > 0x27FF)
            {
                address -= 0x800;
            }
            break;
        }
        case 3:
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
            break;
        }
    }
    return address - 0x2000;
}