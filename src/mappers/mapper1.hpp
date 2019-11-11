#pragma once

#include "mapper.hpp"

class mapper1 : public mapper
{
    private:
        byte* cartRAM;
        byte* CHRRAM;
        byte nametableRAM[2048]{};
        ushort getNametableRAMIndex(ushort address);

        byte shiftReg = 0x10;
        byte mirroringType = 0;
        byte PRGBankMode = 3;
        bool CHRBankMode = 0;

        byte CHRBank0 = 0;
        byte CHRBank1 = 0;
        byte PRGBank = 0;
    public:
        byte getCPU(ushort address);
        void setCPU(ushort address, byte value);
        byte getPPU(ushort address);
        void setPPU(ushort address, byte value);

        mapper1(byte* PRG, unsigned int PRGSize, byte* CHR, unsigned int CHRSize);
        ~mapper1();
};