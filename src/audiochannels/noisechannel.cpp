#include "../includes.hpp"
#include "noisechannel.hpp"

void noiseChannel::setRegister(byte index, byte value)
{
	switch (index)
	{
		case 0:
		{
			lengthCounterHalt = getBit(value, 5);
			constantVolume = getBit(value, 4);
			volume = value & 0xF;
			break;
		}
		case 2:
		{
			noiseMode = getBit(value, 7);
			freqTimerInitial = timerTable[value & 0xF];
			break;
		}
		case 3:
		{
			lengthCounter = lengthTable[value >> 3];
			envelopeStartFlag = true;
			break;
		}
	}
}

void noiseChannel::step()
{
	if (!enabled)
	{
		lengthCounter = 0;
	}

	//APU runs half the clockspeed of CPU, so this should happen every second CPU cycle
	APUCycle = !APUCycle;
	if (APUCycle)
	{
		freqTimer--;
		if (freqTimer == -1)
		{
			freqTimer = freqTimerInitial;
			bool feedback = (getBit(noiseShiftReg, 0) != getBit(noiseShiftReg, noiseMode ? 6 : 1));
			noiseShiftReg >>= 1;
			noiseShiftReg = setBit(noiseShiftReg, 14, feedback);
		}
	}
}

byte noiseChannel::getCurrentOutput()
{
	if (lengthCounter > 0 && !(noiseShiftReg & 1))
	{
		if (constantVolume)
		{
			return volume;
		}
		return decayLevel;
	}
	return 0;
}

void noiseChannel::lengthCounterClock()
{
	if (lengthCounter > 0 && !lengthCounterHalt)
	{
		lengthCounter--;
	}
}

void noiseChannel::envelopeClock()
{
	if (envelopeStartFlag)
	{
		envelopeStartFlag = false;
		decayLevel = 15;
		envelopeDivider = volume;
	}
	else
	{
		if (envelopeDivider == 0)
		{
			envelopeDivider = volume;
			if (decayLevel == 0)
			{
				//lengthCounterHalt is also the envelope loop flag
				if (lengthCounterHalt)
				{
					decayLevel = 15;
				}
			}
			else
			{
				decayLevel--;
			}
		}
		envelopeDivider--;
	}
}