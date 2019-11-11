#pragma once

class noiseChannel
{
	public:
		void setRegister(byte index, byte value);
		void step();
		byte getCurrentOutput();
		void lengthCounterClock();
		void envelopeClock();
		bool enabled = false;
		ushort lengthCounter = 0;
	private:
		const byte lengthTable[32] = { 10,254,20,2,40,4,80,6,160,8,60,10,14,12,26,14,12,16,24,18,48,20,96,22,192,24,72,26,16,28,32,30 };
		const ushort timerTable[16] = {4,8,16,32,64,96,128,160,202,254,380,508,762,1016,2034,4068};
		bool APUCycle = false;
		bool noiseMode = false;
		ushort noiseShiftReg = 1;
		ushort freqTimerInitial = 0;
		short freqTimer = 0;
		bool lengthCounterHalt = false;
		byte volume = 0;
		//envelope
		bool constantVolume = false;
		bool envelopeStartFlag = false;
		byte decayLevel = 0;
		byte envelopeDivider = 0;
};