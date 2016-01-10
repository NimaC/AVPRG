#include "Oscillator.h"

//Setter-Methoden für die Hauptklasse Synth.cpp
void Oscillator::setMode(OscillatorMode mode)
{
	mOscillatorMode = mode;
}

//Da die Kurve sich mit der Frequenz oder Samplerate verändert,
//muss auch mPhaseIncrement verändert werden
void Oscillator::setFrequency(double frequency)
{
	mFrequency = frequency;
	updateIncrement();
}

void Oscillator::setSampleRate(double sampleRate)
{
	mSampleRate = sampleRate;
	updateIncrement();
}

//Wird nur noch aufgerufen, wenn sich Frequenz oder Samplerate verändern
void Oscillator::updateIncrement()
{
	mPhaseIncrement = mFrequency * 2 * mPI / mSampleRate;
}

double Oscillator::nextSample()
{
	//Wenn value den Wert 0 hat, ist iMuted true
	double value = 0.0;
	if (isMuted) return value;

	//Hier findet die Berechnung für die Kurven statt:
	//mPhase beginnt bei 0 und wird nach jedem Sample um mPhaseIncrement erhöht
	switch (mOscillatorMode)
	{
	case OSCILLATOR_MODE_SINE:
		value = sin(mPhase);
		break;
	case OSCILLATOR_MODE_SAW:
		value = 1.0 - (2.0 * mPhase / twoPI);
		break;
	case OSCILLATOR_MODE_SQUARE:
		if (mPhase <= mPI)
		{
			value = 1.0;
		}
		else
		{
			value = -1.0;
		}
		break;
	case OSCILLATOR_MODE_TRIANGLE:
		value = -1.0 + (2.0 * mPhase / twoPI);
		value = 2.0 * (fabs(value) - 0.5);
		break;
	}
	mPhase += mPhaseIncrement;
	//Wenn mPhase genauso groß wie 2PI is, dann ist die Periode einmal
	//durchlaufen und mPhase wird wieder 0
	while (mPhase >= twoPI)
	{
		mPhase -= twoPI;
	}
	return value;
}