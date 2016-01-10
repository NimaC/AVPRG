#include "ADSREnvelope.h"
#include <math.h>


void ADSREnvelope::setSampleRate(double newSampleRate)
{
	sampleRate = newSampleRate;
}


/*die Funktion wird in der Hauptklasse Synth aufgerufen. Es wird geprüft, ob sich der Envelope
nicht in den Phasen OFF oder SUSTAIN befindet. In der zweiten if-Abfrage werden zwei Indizes verglichen
und wenn sie gleich sind, ist eine Phase(ATTACK,DECAY,RELASE) abgeschlossen und es wird zur nächsten Phase
gesprungen. Falls dies nicht der Fall ist, wird currentLevel mit dem Multiplier multipliziert und der Index 
hochgezählt*/
double ADSREnvelope::nextSample()
{
	if (currentStage != ENVELOPE_STAGE_OFF &&
		currentStage != ENVELOPE_STAGE_SUSTAIN)
	{
		if (currentSampleIndex == nextStageSampleIndex)
		{
			EnvelopeStage newStage = static_cast<EnvelopeStage>((currentStage + 1) % kNumEnvelopeStages);
			enterStage(newStage);
		}
		currentLevel *= multiplier;
		currentSampleIndex++;
	}
	return currentLevel;
}

//Basiert auf http://www.musicdsp.org/showone.php?id=189
void ADSREnvelope::calculateMultiplier(double startLevel, double endLevel, unsigned long long lengthInSamples)
{
	multiplier = 1.0 + (log(endLevel) - log(startLevel)) / (lengthInSamples);
}

/*In der zweiten if-Abfrage von nextSample() wird enterStage() aufgerufen. Und zwar wird die akutuelle Phase übergeben
und der Index zurückgesetzt. Falls es sich nicht um die OFF oder SUSTAIN PHASE handelt, wird der Index für die nächste Phase gesetzt.
Innerhalb der aktuellen Phase wird dann der Multiplier für den logarithmischen Verlauf berechnet. In nextSample() wird der Multiplier dann
so lange mit dem currentLevel multipliziert bis die nächste Phase erreicht ist.*/
void ADSREnvelope::enterStage(EnvelopeStage newStage) {
	if (currentStage == newStage) return;

	currentStage = newStage;
	currentSampleIndex = 0;
	if (currentStage == ENVELOPE_STAGE_OFF ||
		currentStage == ENVELOPE_STAGE_SUSTAIN) {
		nextStageSampleIndex = 0;
	}
	else {
		nextStageSampleIndex = stageValue[currentStage] * sampleRate;
	}
	switch (newStage) {
	case ENVELOPE_STAGE_OFF:
		currentLevel = 0.0;
		multiplier = 1.0;
		break;
	case ENVELOPE_STAGE_ATTACK:
		currentLevel = minimumLevel;
		calculateMultiplier(currentLevel,
			1.0,
			nextStageSampleIndex);
		break;
	case ENVELOPE_STAGE_DECAY:
		currentLevel = 1.0;
		calculateMultiplier(currentLevel,
			fmax(stageValue[ENVELOPE_STAGE_SUSTAIN], minimumLevel),
			nextStageSampleIndex);
		break;
	case ENVELOPE_STAGE_SUSTAIN:
		currentLevel = stageValue[ENVELOPE_STAGE_SUSTAIN];
		multiplier = 1.0;
		break;
	case ENVELOPE_STAGE_RELEASE:
		calculateMultiplier(currentLevel,
			minimumLevel,
			nextStageSampleIndex);
		break;
	default:
		break;
	}
}


void ADSREnvelope::setStageValue(EnvelopeStage stage, double value)
{
	stageValue[stage] = value;
	if (stage == currentStage)
	{
		if (currentStage == ENVELOPE_STAGE_ATTACK || currentStage == ENVELOPE_STAGE_DECAY || currentStage == ENVELOPE_STAGE_RELEASE)
		{
			double nextLevelValue;
			switch (currentStage)
			{
			case ENVELOPE_STAGE_ATTACK:
				nextLevelValue = 1.0;
				break;
			case ENVELOPE_STAGE_DECAY:
				nextLevelValue = fmax(stageValue[ENVELOPE_STAGE_SUSTAIN], minimumLevel);
				break;
			case ENVELOPE_STAGE_RELEASE:
				nextLevelValue = minimumLevel;
				break;
			default:
				break;
			}
			//Fortschritt der Stage
			double currentStageProcess = (currentSampleIndex + 0.0) / nextStageSampleIndex;
			//Wie lange geht die Stage noch
			double remainingStageProcess = 1.0 - currentStageProcess;
			unsigned long long samplesUntilNextStage = remainingStageProcess * value * sampleRate;
			nextStageSampleIndex = currentSampleIndex + samplesUntilNextStage;
			calculateMultiplier(currentLevel, nextLevelValue, samplesUntilNextStage);
		}
		else if (currentStage == ENVELOPE_STAGE_SUSTAIN)
		{
			currentLevel = value;
		}
	}
	if (currentStage == ENVELOPE_STAGE_DECAY && stage == ENVELOPE_STAGE_SUSTAIN)
	{
		unsigned long long samplesUntilNextStage = nextStageSampleIndex - currentSampleIndex;
		calculateMultiplier(currentLevel, fmax(stageValue[ENVELOPE_STAGE_SUSTAIN], minimumLevel), samplesUntilNextStage);
	}
}