#ifndef _SYNTH_OSCILLATOR_
#define _SYNTH_OSCILLATOR_
#include <math.h>

enum OscillatorMode
{
	OSCILLATOR_MODE_SINE = 0,
	OSCILLATOR_MODE_SAW,
	OSCILLATOR_MODE_TRIANGLE,
	OSCILLATOR_MODE_SQUARE,
	kNumOscillatorModes
};

class Oscillator
{
private:
	OscillatorMode mOscillatorMode;
	const double mPI;
	const double twoPI;
	bool isMuted;
	double mFrequency;
	//Beschreibt die Position auf der Kurve
	double mPhase;		
	double mSampleRate;
	//Wird mPhase für jedes Sample hinzugefügt
	double mPhaseIncrement;	
	//Wird aufgerufen wenn sich die Samplerate oder die Frequenz ändert
	void updateIncrement();		
public:
	void setMode(OscillatorMode mode);
	void setFrequency(double frequency);
	void setSampleRate(double sampleRate);
	Oscillator() :
		mOscillatorMode(OSCILLATOR_MODE_SINE),
		mPI(2 * acos(0.0)),
		twoPI(2 * mPI),
		isMuted(true),		
		mFrequency(440.0),
		mPhase(0.0),
		//Hier wird updateIncrement im Konstruktor einmal aufgerufen, damit mPhaseIncrement einen Wert hat
		mSampleRate(44100.0) {updateIncrement(); }; 
	
	inline void setMuted(bool muted) { isMuted = muted; }
	double nextSample();
};
#endif _SYNTH_OSCILLATOR_
