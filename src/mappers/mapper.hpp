#pragma once

class mapper
{
    public:
        virtual byte getCPU(ushort address) = 0;
        virtual void setCPU(ushort address, byte value) = 0;
        virtual byte getPPU(ushort address) = 0;
        virtual void setPPU(ushort address, byte value) = 0;
        byte* PRGROM;
        unsigned int PRGROMSize;
        byte* CHRROM;
        unsigned int CHRROMSize;
};