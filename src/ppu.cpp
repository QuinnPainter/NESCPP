#include "includes.hpp"
#include "ppu.hpp"


//const unsigned int patTableWindowXRes = 256;
//const unsigned int patTableWindowYRes = 128;
//const unsigned int patTableWindowWidth = patTableWindowXRes * Scale;
//const unsigned int patTableWindowHeight = patTableWindowYRes * Scale;


ppu::ppu(mapper* m, bool* NMI, sf::RenderWindow* w)
{
    Mapper = m;
    NMILatch = NMI;
    mainWindow = w;
    windowTexture.create(256, 240);
    windowSprite.setTexture(windowTexture);
    windowSprite.setScale(sf::Vector2f(2, 2));
    /*
    patternTableWindow = SDL_CreateWindow("NESPatternTables", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, patTableWindowWidth, patTableWindowHeight, SDL_WINDOW_SHOWN);
    if(patternTableWindow == NULL)
    {
        logging::logerr("Window could not be created! SDL_Error: " + std::string(SDL_GetError()), true);
    }
    patternTableRenderer = SDL_CreateRenderer(patternTableWindow, -1, SDL_RENDERER_ACCELERATED);
    if(patternTableRenderer == NULL)
    {
        logging::logerr("Renderer could not be created! SDL_Error: " + std::string(SDL_GetError()), true);
    }
    patternTableTexture = SDL_CreateTexture(patternTableRenderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, patTableWindowXRes, patTableWindowYRes);
    */
   /*
    mainWindow = SDL_CreateWindow("NESCPP", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, mainWindowWidth, mainWindowHeight, SDL_WINDOW_SHOWN);
    if(mainWindow == NULL)
    {
        logging::logerr("Window could not be created! SDL_Error: " + std::string(SDL_GetError()), true);
    }
    mainRenderer = SDL_CreateRenderer(mainWindow, -1, SDL_RENDERER_ACCELERATED);
    if(mainRenderer == NULL)
    {
        logging::logerr("Renderer could not be created! SDL_Error: " + std::string(SDL_GetError()), true);
    }
    mainTexture = SDL_CreateTexture(mainRenderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, mainWindowXRes, mainWindowYRes);
    */

}

ppu::~ppu()
{
    //SDL_DestroyTexture(mainTexture);
    //SDL_DestroyRenderer(mainRenderer);
    //SDL_DestroyWindow(mainWindow);
    /*
    SDL_DestroyTexture(patternTableTexture);
    SDL_DestroyRenderer(patternTableRenderer);
    SDL_DestroyWindow(patternTableWindow);
    */
}

void ppu::OAMDMA(byte* ramptr)
{
    //copy starting from OAMAddress (usually 0) and wrap around
    memcpy(OAM + OAMAddress, ramptr, 256 - OAMAddress);
    if (OAMAddress != 0)
    {
        memcpy(OAM, ramptr + (256 - OAMAddress), OAMAddress);
    }
}

void ppu::paletteSet(ushort addr, byte value)
{
    //mirroring
    addr &= 0x1F;
    //sprite background mirroring
    if (addr == 0x10 || addr == 0x14 || addr == 0x18 || addr == 0x1C)
    {
        addr -= 0x10;
    }
    paletteRAM[addr] = value;
}

byte ppu::paletteGet(ushort addr)
{
    //mirroring
    addr &= 0x1F;
    //sprite background mirroring
    if (addr == 0x10 || addr == 0x14 || addr == 0x18 || addr == 0x1C)
    {
        addr -= 0x10;
    }
    return paletteRAM[addr];
}

void ppu::setRegister(ushort addr, byte value)
{
    //logging::log("write " + byteToString(value) + " to " + ushortToString(addr));
    switch (addr)
    {
        case 0x2000:
        {
            //PPUCTRL todo: PPU master/slave mode
            if (generateNMI == false && getBit(value, 7) == true && inVBlank)
            {
                *NMILatch = true;
            }
            generateNMI = getBit(value, 7);
            addressIncrement = getBit(value, 2) ? 32 : 1;
            BGPatTable = getBit(value, 4);
            SpritePatTable = getBit(value, 3);
            SpriteMode = getBit(value, 5);
            tempAddr = (tempAddr & 0xF3FF) | ((ushort)(value & 0x3) << 10);
            break;
        }
        case 0x2001:
        {
            //PPUMASK
            //Only todo thing about ppumask is the colour emphasis bits, when I figure out how they work
            PPUMask = value;
            break;
        }
        case 0x2002:
        {
            //read-only register: PPUSTATUS
            break;
        }
        case 0x2003:
        {
            //OAMADDR
            OAMAddress = value;
            break;
        }
        case 0x2004:
        {
            //OAMDATA
            OAM[OAMAddress] = value;
            OAMAddress++;
            break;
        }
        case 0x2005:
        {
            //PPUSCROLL
            if (!writeToggle)
            {
                //replace first 5 bits of tempAddr with last 5 bits of value
                tempAddr = (tempAddr & 0xFFE0) | (value >> 3);
                fineX = value & 0x7;
            }
            else
            {
                //fine Y
                tempAddr = (tempAddr & 0x8FFF) | ((ushort)(value & 0x7) << 12);
                //coarse Y
                tempAddr = (tempAddr & 0xFC1F) | ((ushort)(value & 0xF8) << 2);
            }
            writeToggle = !writeToggle;
            break;
        }
        case 0x2006:
        {
            //PPUADDR
            if (!writeToggle)
            {
                tempAddr = (tempAddr & 0x80FF) | ((ushort)(value & 0x3F) << 8);
            }
            else
            {
                tempAddr = (tempAddr & 0xFF00) | value;
                currentAddr = tempAddr;
            }
            writeToggle = !writeToggle;
            break;
        }
        case 0x2007:
        {
            ushort writeAddr = currentAddr & 0x3FFF;
            //PPUDATA
            if (writeAddr >= 0x3F00)
            {
                paletteSet(writeAddr, value);
            }
            else
            {
                Mapper->setPPU(writeAddr, value);
            }
            currentAddr += addressIncrement;
            break;
        }
        default:
        {
            logging::log("Invalid PPU register: " + ushortToString(addr), true, true);
            break;
        }
    }
}

byte ppu::getRegister(ushort addr)
{
    switch (addr)
    {
        case 0x2002:
        {
            //PPUSTATUS
            writeToggle = false;
            byte ret = setBit(0, 7, inVBlank);
            ret = setBit(ret, 6, SpriteZeroHit);
            ret = setBit(ret, 5, SpriteOverflow);
            inVBlank = false;
            return ret;
        }
        case 0x2004:
        {
            //OAMDATA
            return OAM[OAMAddress];
        }
        case 0x2007:
        {
            ushort readAddr = currentAddr & 0x3FFF;
            //PPUDATA
            if (readAddr >= 0x3F00)
            {
                //paletteRAM mirroring
                byte p = paletteGet(readAddr);
                //grayscale mode
                if (getBit(PPUMask, 0))
                {
                    p &= 0x30;
                }
                PPUDataReadBuffer = Mapper->getPPU(readAddr - 0x1000);
                currentAddr += addressIncrement;
                return p;
            }
            else
            {
                byte ret = PPUDataReadBuffer;
                PPUDataReadBuffer = Mapper->getPPU(readAddr);
                currentAddr += addressIncrement;
                return ret;
            }
        }
        case 0x2000: case 0x2001: case 0x2003: case 0x2005: case 0x2006:
        {
            //write-only registers: PPUCTRL, PPUMASK, OAMADDR, PPUSCROLL, PPUADDR
            return 0;
        }
        default:
        {
            logging::log("Invalid PPU register: " + ushortToString(addr), true, true);
            return 0;
        }
    }
}

void ppu::copyX()
{
    //hori(v) = hori(t)
    currentAddr = (currentAddr & 0xFBE0) | (tempAddr & 0x41F);
}

void ppu::copyY()
{
    //vert(v) = vert(t)
    currentAddr = (currentAddr & 0x841F) | (tempAddr & 0x7BE0);
}

void ppu::incrementX()
{
    //inc hori(v)
    if ((currentAddr & 0x001F) == 31)   // if coarse X == 31
    {
        currentAddr &= ~0x001F;         // coarse X = 0
        currentAddr ^= 0x0400;          // switch horizontal nametable
    }
    else
    {
        currentAddr += 1;               // increment coarse X
    }
}

void ppu::incrementY()
{
    //inc vert(v)
    if ((currentAddr & 0x7000) != 0x7000)                   // if fine Y < 7
    {
        currentAddr += 0x1000;                              // increment fine Y
    }
    else
    {
        currentAddr &= ~0x7000;                             // fine Y = 0
        ushort y = (currentAddr & 0x03E0) >> 5;             // let y = coarse Y
        if (y == 29)
        {
            y = 0;                                          // coarse Y = 0
            currentAddr ^= 0x0800;                          // switch vertical nametable
        }
        else if (y == 31)
        {
            y = 0;                                          // coarse Y = 0, nametable not switched
        }
        else
        {
            y += 1;                                         // increment coarse Y
        }
        currentAddr = (currentAddr & ~0x03E0) | (y << 5);   // put coarse Y back into v
    }
}

void ppu::fetchNametableByte()
{
    ushort tileAddress = 0x2000 | (currentAddr & 0x0FFF);
    nametableByte = Mapper->getPPU(tileAddress);
}

void ppu::fetchAttributeByte()
{
    ushort attributeAddress = 0x23C0 | (currentAddr & 0x0C00) | ((currentAddr >> 4) & 0x38) | ((currentAddr >> 2) & 0x07);
    byte fullAttributeByte = Mapper->getPPU(attributeAddress);
    //Shift the attribute byte by the appropriate amount depending on which sector the current tile is in
    byte shift = ((currentAddr & 0x40) >> 4) | (currentAddr & 0x2);
    tileAttribute = ((fullAttributeByte >> shift) & 0x3);
}

void ppu::fetchLowBGByte()
{
    byte fineY = currentAddr >> 12;
    ushort tileAddr = (BGPatTable * 0x1000) + ((ushort)nametableByte * 16) + fineY;
    lowBGByte = Mapper->getPPU(tileAddr);
}

void ppu::fetchHighBGByte()
{
    byte fineY = currentAddr >> 12;
    ushort tileAddr = (BGPatTable * 0x1000) + ((ushort)nametableByte * 16) + fineY + 8;
    highBGByte = Mapper->getPPU(tileAddr);
}

void ppu::copyTileData()
{
    lowBGShiftReg |= (ushort)lowBGByte;
    highBGShiftReg |= (ushort)highBGByte;
    attributeLatch = tileAttribute;
}

ushort ppu::getSpriteTileData(byte spriteIndex)
{
    byte tileRow = scanlineCounter - OAM[spriteIndex*4];
    byte tileIndex = OAM[(spriteIndex*4) + 1];
    byte sAttributes = OAM[(spriteIndex*4) + 2];
    //Flip Y
    if (getBit(sAttributes, 7))
    {
        tileRow = ((SpriteMode ? 16 : 8) - 1) - tileRow;
    }

    ushort patTableAddr = 0;
    if (SpriteMode)
    {
        //8x16
        byte patTable = tileIndex & 1;
		tileIndex &= 0xFE;
		if (tileRow > 7)
        {
			tileIndex++;
			tileRow -= 8;
		}
        patTableAddr = (0x1000 * patTable) + (tileIndex * 16) + tileRow;
    }
    else
    {
        //8x8
        patTableAddr = (0x1000 * SpritePatTable) + (tileIndex * 16) + tileRow;
    }

    //Flip X
    if (getBit(sAttributes, 6))
    {
        return combineBytes(reverseByte(Mapper->getPPU(patTableAddr + 8)), reverseByte(Mapper->getPPU(patTableAddr)));
    }
    else
    {
        return combineBytes(Mapper->getPPU(patTableAddr + 8), Mapper->getPPU(patTableAddr));
    }
}

void ppu::evaluateSprites()
{
    int spriteHeight = SpriteMode ? 16 : 8;
    byte numSprites = 0;
    for (byte i = 0; i < 64; i++)
    {
        byte spriteY = OAM[i*4];
        if (spriteY > scanlineCounter || (spriteY + spriteHeight) <= scanlineCounter)
        {
            continue;
        }

        if (numSprites == 8)
        {
            SpriteOverflow = 1;
            break;
        }

        ushort tileData = getSpriteTileData(i);
        lowSpriteTileBytes[numSprites] = lowByte(tileData);
        highSpriteTileBytes[numSprites] = highByte(tileData);
        spriteAttributes[numSprites] = OAM[(i * 4) + 2];
        spriteXCounters[numSprites] = OAM[(i * 4) + 3];
        spriteIndexes[numSprites] = i;
        numSprites++;
    }
}

void ppu::drawPixel()
{
    //byte BGPix = ((byte)(getBit(highBGShiftReg, 15 - fineX)) << 1) | (byte)(getBit(lowBGShiftReg, 15 - fineX));
    //byte BGAttribute = ((byte)(getBit(attributeReg, 15 - (fineX * 2))) << 1) | (byte)(getBit(attributeReg, 14 - (fineX * 2)));
	byte BGPix = (((highBGShiftReg & (1 << (15 - fineX))) >> ((15 - fineX) - 1)) | ((lowBGShiftReg & (1 << (15 - fineX))) >> (15 - fineX)));
	byte BGAttribute = (attributeReg >> (14 - (fineX * 2)) & 0x3);
    byte SpritePix = 0;
    byte SpritePalette = 0;
    bool Priority = 0;
    for (int i = 0; i < 8; i++)
    {
		if (spriteXCounters[i] <= 0 && spriteXCounters[i] > -8)
		{
			//byte pix = ((byte)(getBit(highSpriteTileBytes[i], 7)) << 1) | getBit(lowSpriteTileBytes[i], 7);
			byte pix = ((highSpriteTileBytes[i] & 0x80) >> 6) | ((lowSpriteTileBytes[i] & 0x80) >> 7);
            if (pix != 0)
            {
                SpritePix = pix;
                SpritePalette = spriteAttributes[i] & 0x3;
                Priority = getBit(spriteAttributes[i], 5);

                if (BGPix != 0 && spriteIndexes[i] == 0 && cycleCounter < 256)
                {
                    SpriteZeroHit = 1;
                }
            }
            lowSpriteTileBytes[i] <<= 1;
            highSpriteTileBytes[i] <<= 1;
        }
        spriteXCounters[i]--;
    }
    colour FinalPixColour;
    byte FinalPaletteColour;
    bool showBG = false;
    bool showSprite = false;
    if (SpritePix != 0 && BGPix == 0)
    {
        showSprite = true;
    }
    else if (SpritePix == 0 && BGPix != 0)
    {
        showBG = true;
    }
    else if (SpritePix != 0 && BGPix != 0)
    {
        if (Priority)
        {
            showBG = true;
        }
        else
        {
            showSprite = true;
        }
    }

    if (showBG)
    {
        FinalPaletteColour = paletteGet((BGAttribute << 2) | BGPix);
    }
    else if (showSprite)
    {
        FinalPaletteColour = paletteGet((SpritePalette << 2) | SpritePix | 0x10);
    }
    else//if (BGPix == 0 && SpritePix == 0)
    {
        FinalPaletteColour = paletteRAM[0];
    }
    //grayscale mode
    if (PPUMask & 1)
    {
        FinalPaletteColour &= 0x30;
    }
    FinalPixColour = paletteColours[FinalPaletteColour];

	//int memLocation = ((scanlineCounter * 4 * 256) + ((cycleCounter - 1) * 4));
	int memLocation = ((scanlineCounter << 10) + ((cycleCounter - 1) << 2));
    mainScreenData[memLocation] = FinalPixColour.r;
    mainScreenData[memLocation + 1] = FinalPixColour.g;
    mainScreenData[memLocation + 2] = FinalPixColour.b;
    mainScreenData[memLocation + 3] = 255; //alpha channel
}

void ppu::step(int cycles)
{
	for (int c = 0; c < cycles; c++)
	{
		bool renderingEnabled = (PPUMask & (1 << 3)) || (PPUMask & (1 << 4));
		bool visibleLine = scanlineCounter < 240;
		bool prerenderLine = scanlineCounter == 261;
		bool renderLine = visibleLine || prerenderLine;
		bool preFetchCycle = cycleCounter >= 321 && cycleCounter <= 336;
		bool visibleCycle = cycleCounter >= 1 && cycleCounter <= 256;
		bool fetchCycle = preFetchCycle || visibleCycle;

		if (renderingEnabled)
		{
			//Background
			if (visibleLine && visibleCycle)
			{
				drawPixel();
			}
			if (renderLine && fetchCycle)
			{
				highBGShiftReg <<= 1;
				lowBGShiftReg <<= 1;
				attributeReg <<= 2;
				attributeReg |= (ushort)attributeLatch;
				switch (cycleCounter % 8)
				{
				case 1:
					fetchNametableByte();
					break;
				case 3:
					fetchAttributeByte();
					break;
				case 5:
					fetchLowBGByte();
					break;
				case 7:
					fetchHighBGByte();
					break;
				case 0:
					copyTileData();
					incrementX();
					break;
				}
			}
			//Sprites
			if (cycleCounter == 257 && visibleLine)
			{
				evaluateSprites();
				OAMAddress = 0;
			}

			if (prerenderLine && cycleCounter >= 280 && cycleCounter <= 304)
			{
				copyY();
			}
			if (renderLine)
			{
				if (cycleCounter == 256)
				{
					incrementY();
				}
				else if (cycleCounter == 257)
				{
					copyX();
				}
			}
		}

		if (scanlineCounter == 241 && cycleCounter == 1)
		{
			//vblank start
			if (generateNMI)
			{
				*NMILatch = true;
			}
			inVBlank = true;
		}
		else if (prerenderLine && cycleCounter == 1)
		{
			//end of vblank (pre-render line)
			inVBlank = false;
			SpriteZeroHit = 0;
			SpriteOverflow = 0;
		}

		cycleCounter++;
		if (cycleCounter > 340)
		{
			cycleCounter = 0;
			scanlineCounter++;
			if (scanlineCounter > 261)
			{
				scanlineCounter = 0;
			}
		}
	}
}

void ppu::drawScreen()
{
    /*
    //2 pattern tables
    for (int patTable = 0; patTable < 2; patTable++)
    {
        //columns of tiles in 1 pattern table
        for (int tileY = 0; tileY < 16; tileY++)
        {
            //row of tiles in 1 pattern table
            for (int tileX = 0; tileX < 16; tileX++)
            {
                //rows of tile
                for (int yInTile = 0; yInTile < 8; yInTile++)
                {
                    int offset = (patTable * 256 * 16) + (tileX * 16) + (tileY * 16 * 16);
                    byte b1 = Mapper->getPPU(offset + yInTile);
                    byte b2 = Mapper->getPPU(offset + yInTile + 8);
                    //columns / bits in row
                    for (int xInTile = 0; xInTile < 8; xInTile++)
                    {
                        //either 0, 1, 2 or 3. gives the index of the colour in the palette
                        byte colourIndex = ((byte)(getBit(b2, 7 - xInTile)) << 1) | getBit(b1, 7 - xInTile);
                        if (colourIndex == 0)
                        {
                            //transparent pixel
                            continue;
                        }
                        //just use the first palette
                        colour c = paletteColours[paletteRAM[colourIndex]];
                        int memLocation = ((patTable * 128 * 3) + (tileY * 8 * 256 * 3) + (tileX * 8 * 3) + (xInTile * 3) + (yInTile * patTableWindowXRes * 3));
                        patternTableScreenData[memLocation] = c.r;
                        patternTableScreenData[memLocation + 1] = c.g;
                        patternTableScreenData[memLocation + 2] = c.b;
                    }
                }
            }
        }
    }

    SDL_UpdateTexture(patternTableTexture, NULL, patternTableScreenData, patTableWindowXRes * 3);
    SDL_RenderCopy(patternTableRenderer, patternTableTexture, NULL, NULL);
    SDL_RenderPresent(patternTableRenderer);
    */

    /*
    //draw the nametables
    //columns of tiles
    for (int tileY = 0; tileY < 30; tileY++)
    {
        //row of tiles
        for (int tileX = 0; tileX < 32; tileX++)
        {
            byte tileIndex = Mapper->getPPU(0x2000 + tileX + (tileY * 32));
            ushort tileMemAddress = (tileIndex * 16) + BGPatTable * 0x1000;

            //https://wiki.nesdev.com/w/index.php/PPU_attribute_tables
            ushort tileAttributeAddress = 0x23C0 + (tileX / 4) + ((tileY / 4) * 8);
            byte tileAttributeByte = Mapper->getPPU(tileAttributeAddress);
            byte palette = 0;
            if ((tileX % 4) > 1)        //2 or 3 - in right side
            {
                if ((tileY % 4) > 1)    //2 or 3 - in bottom right corner
                {
                    palette = (tileAttributeByte >> 6) & 0x3;
                }
                else                    //0 or 1 - in top right corner
                {
                    palette = (tileAttributeByte >> 2) & 0x3;
                }
            }
            else                        //0 or 1 - in left side
            {
                if ((tileY % 4) > 1)    //2 or 3 - in bottom left corner
                {
                    palette = (tileAttributeByte >> 4) & 0x3;
                }
                else                    //0 or 1 - in top left corner
                {
                    palette = tileAttributeByte & 0x3;
                }
            }
            
            for (int yInTile = 0; yInTile < 8; yInTile++)
            {
                byte b1 = Mapper->getPPU(tileMemAddress + yInTile);
                byte b2 = Mapper->getPPU(tileMemAddress + yInTile + 8);
                //columns / bits in row
                for (int xInTile = 0; xInTile < 8; xInTile++)
                {
                    //either 0, 1, 2 or 3. gives the index of the colour in the palette
                    byte colourIndex = ((byte)(getBit(b2, 7 - xInTile)) << 1) | getBit(b1, 7 - xInTile);
                    colour c;
                    if (colourIndex == 0)
                    {
                        //black
                        c = paletteColours[13];
                    }
                    c = paletteColours[paletteRAM[(palette * 4) + colourIndex]];
                    int memLocation = ((tileY * 8 * 3 * mainWindowXRes) + (tileX * 8 * 3) + (xInTile * 3) + (yInTile * mainWindowXRes * 3));
                    mainScreenData[memLocation] = c.r;
                    mainScreenData[memLocation + 1] = c.g;
                    mainScreenData[memLocation + 2] = c.b;
                }
            }
        }
    }
    */

    //SDL_UpdateTexture(mainTexture, NULL, mainScreenData, mainWindowXRes * 3);
    //SDL_RenderCopy(mainRenderer, mainTexture, NULL, NULL);
    //SDL_RenderPresent(mainRenderer);
    windowTexture.update(mainScreenData);
    mainWindow->draw(windowSprite);
    mainWindow->display();
}