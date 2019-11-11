#include "includes.hpp"
#include "mapperHandler.hpp"

mapper* mapperHandler::getMapper(byte mapperNum, byte* PRGROM, unsigned int PRGROMSize, byte* CHRROM, unsigned int CHRROMSize, bool mirroring)
{
    switch (mapperNum)
    {
        case 0:
        {
            return new mapper0(PRGROM, PRGROMSize, CHRROM, CHRROMSize, mirroring);
        }
        case 1:
        {
            return new mapper1(PRGROM, PRGROMSize, CHRROM, CHRROMSize);
        }
        case 2:
        {
            return new mapper2(PRGROM, PRGROMSize, CHRROM, CHRROMSize, mirroring);
        }
		case 71:
		{
			//this one's a clone, so mapper 2 is close enough
			return new mapper2(PRGROM, PRGROMSize, CHRROM, CHRROMSize, mirroring);
		}
        default:
        {
            logging::log("Unsupported mapper: " + std::to_string((int)mapperNum) + ". Treating it as mapper 0", true, true);
            return new mapper0(PRGROM, PRGROMSize, CHRROM, CHRROMSize, mirroring);
        }
    }
}