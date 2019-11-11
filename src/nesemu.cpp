#include "includes.hpp"
#include "nesemu.hpp"

const unsigned int CPU_CLOCKSPEED = 1789773; //in hz
const unsigned int FRAMERATE = 60;
const unsigned int ClocksPerFrame = CPU_CLOCKSPEED / FRAMERATE;

const unsigned int Scale = 2;
const unsigned int mainWindowXRes = 256;
const unsigned int mainWindowYRes = 240;
const unsigned int mainWindowWidth = mainWindowXRes * Scale;
const unsigned int mainWindowHeight = mainWindowYRes * Scale;

int main(int argc, char** argv)
{
    if (argv[1] == NULL)
    {
        logging::logerr("No file provided!", true);
    }

	FILE *f = fopen(argv[1], "rb");
	if (f == NULL)
	{
        logging::logerr("error: Couldn't open " + std::string(argv[1]), true);
	}
	fseek(f, 0L, SEEK_END);
	int fsize = ftell(f);
	fseek(f, 0L, SEEK_SET);
    byte* cart = new byte[fsize];
    fread(cart, fsize, 1, f);
    fclose(f);

    logging::log("Opened " + std::string(argv[1]));

    //Check header (should be "NES" followed by DOS EOF character : "4E45531A")
    if (cart[0] == 0x4E && cart[1] == 0x45 && cart[2] == 0x53 && cart[3] == 0x1A)
    {
        logging::log("iNES Header is good");
    }
    else
    {
        logging::log("iNES Header is wrong!", true, true);
    }
    
    unsigned int PRGROMSize = cart[4] * 16384;
    logging::log("Size of PRG ROM is " + std::to_string(PRGROMSize) + " bytes (" + std::to_string(cart[4]) + " * 16kb)");
    unsigned int CHRROMSize = cart[5] * 8192;
    if (CHRROMSize == 0)
    {
        logging::log("CHR ROM is not present, so CHR RAM is used");
    }
    else
    {
        logging::log("Size of CHR ROM is " + std::to_string(CHRROMSize) + " bytes (" + std::to_string(cart[5]) + " * 8kb)");
    }

    bool trainer = getBit(cart[6], 2);
    if (trainer)
    {
        logging::log("Trainer is present, skipping over it");
    }
    else
    {
        logging::log("No trainer present");
    }

    byte mapperNum = (cart[6] >> 4) | (cart[7] & 0xF0);
    logging::log("Mapper number is " + std::to_string((int)mapperNum));

    bool mirroring = getBit(cart[6], 0);
    if (mirroring)
    {
        logging::log("Vertical nametable mirroring");
    }
    else
    {
        logging::log("Horizontal nametable mirroring");
    }
    

    byte* PRGROM = new byte[PRGROMSize];
    std::memcpy(PRGROM, cart + 16 + trainer * 512, PRGROMSize);
    byte* CHRROM = nullptr;
    if (CHRROMSize > 0)
    {
        CHRROM = new byte[CHRROMSize];
        std::memcpy(CHRROM, cart + 16 + trainer * 512 + PRGROMSize, CHRROMSize);
    }

    //"global" variables
    bool NMILatch = 0;
    bool IRQLine = 0;
    bool didOAM = 0;

    /*
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_AUDIO) != 0)
    {
        logging::logerr("SDL Init Error: " + std::string(SDL_GetError()), true);
    }
    */
    sf::RenderWindow mainWindow(sf::VideoMode(mainWindowWidth, mainWindowHeight, 32), "nescpp");
    mainWindow.setFramerateLimit(59);
    mainWindow.setKeyRepeatEnabled(false);
    mapperHandler MapperHandler{};
    mapper* Mapper = MapperHandler.getMapper(mapperNum, PRGROM, PRGROMSize, CHRROM, CHRROMSize, mirroring);
    ppu PPU(Mapper, &NMILatch, &mainWindow);
    apu APU(&IRQLine);
    input Input{};
    memory Memory(Mapper, &PPU, &Input, &APU, &didOAM);
    cpu CPU(&Memory, &NMILatch, &IRQLine);
    CPU.reset();

    int cycleCounter = 0;
    int cycles = 0;
	sf::Event event;

    while(mainWindow.isOpen())
    {
        //auto start = std::chrono::steady_clock::now();
        while (mainWindow.pollEvent(event))
        {
            switch(event.type)
            {
                case sf::Event::Closed:
                    mainWindow.close();
                    break;
                case sf::Event::KeyPressed:
                    Input.keyChanged(event.key.code, 1);
                    break;
                case sf::Event::KeyReleased:
                    Input.keyChanged(event.key.code, 0);
                    break;
            }
        }

        while (cycleCounter < ClocksPerFrame)
        {
            cycles = CPU.step();
            if (didOAM)
            {
                didOAM = false;
                cycles += 513;
            }
            //1 CPU cycle = 3 PPU cycles
            PPU.step(cycles * 3);
            APU.step(cycles);
            cycleCounter += cycles;
        }
        cycleCounter %= ClocksPerFrame;
        PPU.drawScreen();
        //auto end = std::chrono::steady_clock::now();
        //int64_t duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        //int delay = (1000 / FRAMERATE) - duration;
        //if (delay > 0)
        //{
        //    SDL_Delay(delay);
        //}
        //logging::log("time: " + std::to_string(duration));
    }

    delete Mapper;
    delete[] PRGROM;
    delete[] CHRROM;
    delete[] cart;
    logging::log("Exited successfully");
    return 0;
}