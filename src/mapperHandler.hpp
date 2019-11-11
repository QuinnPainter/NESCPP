#pragma once

#include "mappers/mapper0.hpp"
#include "mappers/mapper1.hpp"
#include "mappers/mapper2.hpp"
#include "mappers/mapper.hpp"

class mapperHandler
{
    public:
        mapper* getMapper(byte mapperNum, byte* PRGROM, unsigned int PRGROMSize, byte* CHRROM, unsigned int CHRROMSize, bool mirroring);
};