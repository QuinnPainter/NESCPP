#pragma once

#include "mappers/mapper.hpp"

struct colour
{
    byte r;
    byte g;
    byte b;
};

class ppu
{
    public:
        ppu(mapper* m, bool* NMI, sf::RenderWindow* w);
        ~ppu();
        byte getRegister(ushort addr);
        void setRegister(ushort addr, byte value);
        void step(int cycles);
        void drawScreen();
        void OAMDMA(byte* ramptr);
    private:
        void paletteSet(ushort addr, byte value);
        byte paletteGet(ushort addr);

        void copyX();
        void copyY();
        void incrementX();
        void incrementY();
        void fetchNametableByte();
        void fetchAttributeByte();
        void fetchLowBGByte();
        void fetchHighBGByte();
        void copyTileData();
        void drawPixel();
        ushort getSpriteTileData(byte spriteIndex);
        void evaluateSprites();

        int cycleCounter = 0;
        int scanlineCounter = 0;
    
        byte paletteRAM[32]{};
        byte OAM[256]{};
        byte OAMAddress = 0;
        bool generateNMI = true;
        bool inVBlank = false;
        byte addressIncrement = 1;
        bool BGPatTable = false;
        byte PPUMask = 0;
        bool SpritePatTable = false;
        bool SpriteMode = false; //0: 8x8  1: 8x16
        bool SpriteOverflow = 0;
        bool SpriteZeroHit = 0;
        byte PPUDataReadBuffer = 0;

        ushort currentAddr = 0;     //v
        ushort tempAddr = 0;        //t
        byte fineX = 0;             //x
        bool writeToggle = false;   //w
        //"cache" of tile info
        byte nametableByte = 0;
        byte tileAttribute = 0;
        byte attributeLatch = 0;
        byte lowBGByte = 0;
        byte highBGByte = 0;
        //used in actual rendering
        ushort attributeReg = 0;
        ushort lowBGShiftReg = 0;
        ushort highBGShiftReg = 0;
        //sprite rendering
        byte lowSpriteTileBytes[8]{};
        byte highSpriteTileBytes[8]{};
        byte spriteAttributes[8]{};
        int spriteXCounters[8]{};
        byte spriteIndexes[8]{};

        bool* NMILatch;
        mapper* Mapper;
        //SDL_Window* patternTableWindow;
        //SDL_Renderer* patternTableRenderer;
        //SDL_Texture* patternTableTexture;
        //byte patternTableScreenData[256 * 128 * 3]{};
        //SDL_Window* mainWindow;
        //SDL_Renderer* mainRenderer;
        //SDL_Texture* mainTexture;
        sf::RenderWindow* mainWindow;
        sf::Texture windowTexture;
        sf::Sprite windowSprite;
        //byte mainScreenData[256 * 240 * 3]{};
        sf::Uint8 mainScreenData[256 * 240 * 4]{};
        const colour paletteColours[64] = { //http://www.thealmightyguru.com/Games/Hacking/Wiki/index.php/NES_Palette
            {124,124,124},
            {0,0,252},
            {0,0,188},
            {68,40,188},
            {148,0,132},
            {168,0,32},
            {168,16,0},
            {136,20,0},
            {80,48,0},
            {0,120,0},
            {0,104,0},
            {0,88,0},
            {0,64,88},
            {0,0,0},
            {0,0,0},
            {0,0,0},
            {188,188,188},
            {0,120,248},
            {0,88,248},
            {104,68,252},
            {216,0,204},
            {228,0,88},
            {248,56,0},
            {228,92,16},
            {172,124,0},
            {0,184,0},
            {0,168,0},
            {0,168,68},
            {0,136,136},
            {0,0,0},
            {0,0,0},
            {0,0,0},
            {248,248,248},
            {60,188,252},
            {104,136,252},
            {152,120,248},
            {248,120,248},
            {248,88,152},
            {248,120,88},
            {252,160,68},
            {248,184,0},
            {184,248,24},
            {88,216,84},
            {88,248,152},
            {0,232,216},
            {120,120,120},
            {0,0,0},
            {0,0,0},
            {252,252,252},
            {164,228,252},
            {184,184,248},
            {216,184,248},
            {248,184,248},
            {248,164,192},
            {240,208,176},
            {252,224,168},
            {248,216,120},
            {216,248,120},
            {184,248,184},
            {184,248,216},
            {0,252,252},
            {248,216,248},
            {0,0,0},
            {0,0,0}
        };
};