#include "includes.hpp"
#include "apu.hpp"

const unsigned int ClocksPerFrameSequencer = 7457; //CPU_CLOCKSPEED / 240hz
const int sampleFrequency = 44100;
const int cyclesPerSample = 40; //CPU_CLOCKSPEED / sampleFrequency
const ushort sampleSize = 16384;
const int volumeMultiplier = 20000;

void APUSoundStream::init(short* buf, int* bufIndex, sf::Mutex* mut)
{
    buffer = buf;
    bufferIndex = bufIndex;
	bufferMutex = mut;
    initialize(1, 44100);
}

bool APUSoundStream::onGetData(Chunk& data)
{
    if (*bufferIndex < 10)
    {
        //logging::log("Ran out of audio data!");
        data.samples = zeroBuf;
        data.sampleCount = 512;
    }
    else
    {
		bufferMutex->lock();
        data.samples = buffer;
        data.sampleCount = *bufferIndex;
        *bufferIndex = 0;
		bufferMutex->unlock();
    }
    return true;
}

apu::apu(bool* irq)
{
    IRQLine = irq;
	pulse1.pulse2 = false;
	pulse2.pulse2 = true;

	//Initialize mixer lookup tables
	mixerPulseTable[0] = 0;
	for (int i = 1; i < 31; i++)
	{
		mixerPulseTable[i] = (95.52 / ((8128.0 / i) + 100));
	}
	mixerTNDTable[0] = 0;
	for (int i = 1; i < 203; i++)
	{
		mixerTNDTable[i] = (163.67 / ((24329.0 / i) + 100));
	}

    stream.init(buffer, &bufferIndex, &AudioBufferMutex);
    stream.play();
}

void apu::setRegister(ushort addr, byte value)
{
    switch (addr)
    {
		case 0x4000: case 0x4001: case 0x4002: case 0x4003: pulse1.setRegister(addr - 0x4000, value); break;
		case 0x4004: case 0x4005: case 0x4006: case 0x4007: pulse2.setRegister(addr - 0x4004, value); break;
		case 0x4008: case 0x4009: case 0x400A: case 0x400B: triangle.setRegister(addr - 0x4008, value); break;
		case 0x400C: case 0x400D: case 0x400E: case 0x400F: noise.setRegister(addr - 0x400C, value); break;
		case 0x4010: case 0x4011: case 0x4012: case 0x4013:
        {
            // DMC
            break;
        }
        case 0x4015:
        {
            // Status
            pulse1.enabled = getBit(value, 0);
			pulse2.enabled = getBit(value, 1);
			triangle.enabled = getBit(value, 2);
			noise.enabled = getBit(value, 3);
			//dmc.enabled = getBit(value, 4);
            break;
        }
        case 0x4017:
        {
            // Frame counter
            frameCounterMode = getBit(value, 7);
            frameCounterInterruptInhibit = getBit(value, 6);
            frameCounterCycles = 0;
            frameCounter = 0;
            if (frameCounterInterruptInhibit)
            {
                *IRQLine = false;
                frameCounterIRQ = false;
            }
            break;
        }
        default:
        {
            logging::logerr("Invalid APU register write address: " + ushortToString(addr));
            break;
        }
    }
}

byte apu::getRegister(ushort addr)
{
    switch (addr)
    {
        case 0x4015:
        {
            // Status
            byte ret = 0;
            //bit 7 = DMC interrupt
            ret = setBit(ret, 6, frameCounterIRQ);
            //bit 4 = DMC enabled
			ret = setBit(ret, 3, noise.lengthCounter > 0);
			ret = setBit(ret, 2, triangle.lengthCounter > 0);
			ret = setBit(ret, 1, pulse2.lengthCounter > 0);
            ret = setBit(ret, 0, pulse1.lengthCounter > 0);
            *IRQLine = false;
            frameCounterIRQ = false;
            return ret;
            break;
        } //why doesn't visual C++ have a shortcut for this??
		case 0x4000: case 0x4001: case 0x4002: case 0x4003: case 0x4004: case 0x4005: case 0x4006:
		case 0x4007: case 0x4008: case 0x4009: case 0x400A: case 0x400B: case 0x400C: case 0x400D:
		case 0x400E: case 0x400F: case 0x4010: case 0x4011: case 0x4012: case 0x4013: case 0x4017:
        {
            //Write only registers
            return 0;
            break;
        }
        default:
        {
            logging::logerr("Invalid APU register read address: " + ushortToString(addr));
            return 0;
            break;
        }
    }
}

void apu::step(int cycles)
{
	for (int c = 0; c < cycles; c++)
	{
		frameCounterCycles++;
		if (frameCounterCycles >= ClocksPerFrameSequencer)
		{
			frameCounterCycles = 0;
			frameCounter++;
			if (frameCounterMode)
			{
				if (frameCounter < 4 || frameCounter == 5)
				{
					//Envelope and triangle linear counter
					pulse1.envelopeClock();
					pulse2.envelopeClock();
					triangle.linearCounterClock();
					noise.envelopeClock();
				}
				if (frameCounter == 2 || frameCounter == 5)
				{
					//Length counter and sweep
					pulse1.lengthSweepClock();
					pulse2.lengthSweepClock();
					triangle.lengthCounterClock();
					noise.lengthCounterClock();
				}
				if (frameCounter == 5)
				{
					frameCounter = 0;
				}
			}
			else
			{
				//Envelope and triangle linear counter
				pulse1.envelopeClock();
				pulse2.envelopeClock();
				triangle.linearCounterClock();
				noise.envelopeClock();
				if (frameCounter % 2 == 0)
				{
					//Length counter and sweep
					pulse1.lengthSweepClock();
					pulse2.lengthSweepClock();
					triangle.lengthCounterClock();
					noise.lengthCounterClock();
				}
				if (frameCounter == 4)
				{
					if (!frameCounterInterruptInhibit)
					{
						*IRQLine = true;
						frameCounterIRQ = true;
					}
					frameCounter = 0;
				}
			}
		}

		pulse1.step();
		pulse2.step();
		triangle.step();
		noise.step();

		addToBufferCounter++;
		if (addToBufferCounter == cyclesPerSample)
		{
			addToBufferCounter = 0;

			float mixer = mixerPulseTable[pulse1.getCurrentOutput() + pulse2.getCurrentOutput()];
			mixer += mixerTNDTable[3 * triangle.getCurrentOutput() + 2 * noise.getCurrentOutput()/* + dmc*/];
			AudioBufferMutex.lock();
			buffer[bufferIndex] = mixer * volumeMultiplier;
			bufferIndex++;
			if (bufferIndex >= sampleSize)
			{
				logging::log("Audio buffer overflow!" + std::to_string(stream.getPlayingOffset().asSeconds()));
				bufferIndex = 0;
				//sfml is broken
				stream.setPlayingOffset(sf::seconds(0));
			}
			AudioBufferMutex.unlock();
		}
	}
}