#pragma once

class pulseChannel
{
    public:
        void setRegister(byte index, byte value);
        void step();
        byte getCurrentOutput();
        void lengthSweepClock();
		void envelopeClock();
        bool enabled = false;
        ushort lengthCounter = 0;
		bool pulse2 = false; //pulse 2 has a sliiiightly different sweep unit
    private:
        const bool dutyTable[4][8] = {{0,1,0,0,0,0,0,0},{0,1,1,0,0,0,0,0},{0,1,1,1,1,0,0,0},{1,0,0,1,1,1,1,1}};
        const byte lengthTable[32] = {10,254,20,2,40,4,80,6,160,8,60,10,14,12,26,14,12,16,24,18,48,20,96,22,192,24,72,26,16,28,32,30};
        byte volume = 0;
        byte duty = 0;
        byte waveformSequencer = 0;
        short freqTimer = 0;
        ushort freqTimerInitial = 0;
        bool APUCycle = false;
        bool lengthCounterHalt = false;
		//envelope
		bool constantVolume = false;
		bool envelopeStartFlag = false;
		byte decayLevel = 0;
		byte envelopeDivider = 0;
		//sweep
		bool sweepEnabled = false;
		byte sweepPeriod = 0;
		bool sweepNegate = false;
		byte sweepShift = 0;
		bool sweepReload = false;
		ushort sweepTargetPeriod = 0;
		byte sweepDivider = 0;
};