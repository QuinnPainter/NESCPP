#include "../includes.hpp"
#include "pulsechannel.hpp"

void pulseChannel::setRegister(byte index, byte value)
{
    switch (index)
    {
        case 0:
        {
            duty = value >> 6;
            lengthCounterHalt = getBit(value, 5);
			constantVolume = getBit(value, 4);
            volume = value & 0xF;
            break;
        }
        case 1:
        {
			sweepEnabled = getBit(value, 7);
			sweepPeriod = ((value & 0x70) >> 4);
			sweepNegate = getBit(value, 3);
			sweepShift = value & 0x7;
			sweepReload = true;
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
            waveformSequencer = 0;
            freqTimer = freqTimerInitial;
            lengthCounter = lengthTable[value >> 3];
			envelopeStartFlag = true;
            break;
        }
    }
}

void pulseChannel::step()
{
    if (!enabled)
    {
        lengthCounter = 0;
    }

	int freqTimerChange = freqTimerInitial >> sweepShift;
	if (sweepNegate)
	{
		if (pulse2)
		{
			freqTimerChange *= -1;
		}
		else
		{
			freqTimerChange *= -1;
			freqTimerChange--;
		}
	}
	sweepTargetPeriod = freqTimerInitial + freqTimerChange;

    //APU runs half the clockspeed of CPU, so this should happen every second CPU cycle
    APUCycle = !APUCycle;
    if (APUCycle)
    {
        freqTimer--;
        if (freqTimer == -1)
        {
            freqTimer = freqTimerInitial;
            waveformSequencer++;
            waveformSequencer &= 7; // same as (waveformSequencer %= 8)
        }
    }
}

byte pulseChannel::getCurrentOutput()
{
    if (freqTimerInitial >= 8 && sweepTargetPeriod <= 0x7FF && dutyTable[duty][waveformSequencer] && (bool)lengthCounter)
    {
		if (constantVolume)
		{
			return volume;
		}
		return decayLevel;
    }
    return 0;
}

void pulseChannel::lengthSweepClock()
{
	//Length counter
    if (lengthCounter > 0 && !lengthCounterHalt)
    {
        lengthCounter--;
    }

	//Sweep
	if (sweepDivider == 0 && sweepEnabled && sweepTargetPeriod <= 0x7FF && freqTimerInitial >= 8 && sweepShift > 0)
	{
		freqTimerInitial = sweepTargetPeriod;
	}
	if (sweepDivider == 0 || sweepReload)
	{
		sweepDivider = sweepPeriod;
		sweepReload = 0;
	}
	else
	{
		sweepDivider--;
	}
}

void pulseChannel::envelopeClock()
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