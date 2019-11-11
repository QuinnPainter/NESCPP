#pragma once

#include "audiochannels/pulsechannel.hpp"
#include "audiochannels/trianglechannel.hpp"
#include "audiochannels/noisechannel.hpp"

class APUSoundStream : public sf::SoundStream
{
    public:
        void init(short* buf, int* bufIndex, sf::Mutex* mut);
    private:
        short zeroBuf[512] = {};
        short* buffer;
        int* bufferIndex;
		sf::Mutex* bufferMutex;
		virtual bool onGetData(Chunk& data);
        virtual void onSeek(sf::Time timeOffset) {}
};

class apu
{
    public:
        void step(int cycles);
        void setRegister(ushort addr, byte value);
        byte getRegister(ushort addr);
        apu(bool* irq);
    private:
        APUSoundStream stream;

        pulseChannel pulse1;
		pulseChannel pulse2;
		triangleChannel triangle;
		noiseChannel noise;

        bool* IRQLine;
        bool frameCounterIRQ = 0;
        int addToBufferCounter = 0;
		sf::Mutex AudioBufferMutex;
        short buffer[16384] = {};
        int bufferIndex = 0;
        bool frameCounterMode = 0;
        bool frameCounterInterruptInhibit = 0;
        unsigned int frameCounterCycles = 0;
        unsigned int frameCounter = 0;

		float mixerPulseTable[31];
		float mixerTNDTable[203];
};