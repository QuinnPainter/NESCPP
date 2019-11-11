#pragma once

#include "mappers/mapper.hpp"
#include "ppu.hpp"
#include "input.hpp"
#include "apu.hpp"

class memory
{
    public:
        memory(mapper* m, ppu* p, input* i, apu* a, bool* dO);
        ~memory();
        byte get(ushort address);
        void set(ushort address, byte value);
        ushort get16(ushort address);
        ushort get16bugged(ushort address);
        void set16(ushort address, ushort value);
    private:
        ushort mirrorConvert(ushort address);
        byte* ram;
        mapper* Mapper;
        ppu* PPU;
        apu* APU;
        input* Input;
        byte tempOAM[256]{};
        bool* didOAM;
};