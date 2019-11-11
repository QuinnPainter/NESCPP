#include "../includes.hpp"
#include "trianglechannel.hpp"

void triangleChannel::setRegister(byte index, byte value)
{
	switch (index)
	{
		case 0:
		{
			counterControl = getBit(value, 7);
			linearCounterReload = value & 0x7F;
			break;
		}
		case 2:
		{
			freqTimerInitial = (freqTimerInitial & 0xFF00) | value;
			break;
		}
		case 3:
		{
			freqTimerInitial = (freqTimerInitial & 0x00FF) | ((ushort)(value & 0x7) << 8);
			lengthCounter = lengthTable[value >> 3];
			linearCounterReloadFlag = true;
			break;
		}
	}
}

void triangleChannel::step()
{
	if (!enabled)
	{
		lengthCounter = 0;
	}

	if (linearCounter > 0 && lengthCounter > 0)
	{
		//bit of a hack recommended by nesdev to reduce popping
		if (freqTimerInitial > 2)
		{
			freqTimer--;
			if (freqTimer == -1)
			{
				freqTimer = freqTimerInitial;
				waveformSequencer++;
				waveformSequencer &= 31; // same as (waveformSequencer %= 32)
			}
		}
	}
}

byte triangleChannel::getCurrentOutput()
{
	return triangleSequence[waveformSequencer];
}

void triangleChannel::linearCounterClock()
{
	if (linearCounterReloadFlag)
	{
		linearCounter = linearCounterReload;
	}
	else if (linearCounter > 0)
	{
		linearCounter--;
	}
	if (!counterControl)
	{
		linearCounterReloadFlag = false;
	}
}

void triangleChannel::lengthCounterClock()
{
	if (lengthCounter > 0 && !counterControl)
	{
		lengthCounter--;
	}
}