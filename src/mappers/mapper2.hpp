#pragma once

#include "mapper.hpp"

class mapper2 : public mapper
{
    private:
        byte* cartRAM;
        byte* CHRRAM;
        byte nametableRAM[2048]{};
        bool mirroring;
        ushort getNametableRAMIndex(ushort address);

        byte bankSelect = 0;
    public:
        byte getCPU(ushort address);
        void setCPU(ushort address, byte value);
        byte getPPU(ushort address);
        void setPPU(ushort address, byte value);

        mapper2(byte* PRG, unsigned int PRGSize, byte* CHR, unsigned int CHRSize, bool m);
        ~mapper2();
};