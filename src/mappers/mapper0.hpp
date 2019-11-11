#pragma once

#include "mapper.hpp"

class mapper0 : public mapper
{
    private:
        byte* cartRAM;
        byte* CHRRAM;
        byte nametableRAM[2048]{};
        bool mirroring;
        ushort getNametableRAMIndex(ushort address);
    public:
        byte getCPU(ushort address);
        void setCPU(ushort address, byte value);
        byte getPPU(ushort address);
        void setPPU(ushort address, byte value);

        mapper0(byte* PRG, unsigned int PRGSize, byte* CHR, unsigned int CHRSize, bool m);
        ~mapper0();
};