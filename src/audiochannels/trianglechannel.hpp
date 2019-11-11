#pragma once

class triangleChannel
{
	public:
		void setRegister(byte index, byte value);
		void step();
		byte getCurrentOutput();
		void linearCounterClock();
		void lengthCounterClock();
		bool enabled = false;
		ushort lengthCounter = 0;
	private:
		const byte lengthTable[32] = { 10,254,20,2,40,4,80,6,160,8,60,10,14,12,26,14,12,16,24,18,48,20,96,22,192,24,72,26,16,28,32,30 };
		const byte triangleSequence[32] = {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
		byte waveformSequencer = 0;
		ushort freqTimerInitial = 0;
		short freqTimer = 0;
		bool counterControl = false;
		byte linearCounterReload = 0;
		bool linearCounterReloadFlag = false;
		byte linearCounter = 0;
};