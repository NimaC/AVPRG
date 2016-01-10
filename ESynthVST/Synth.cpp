#include "Synth.h"
#pragma warning( suppress : 4101 4129 )
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "resource.h"

const int kNumPrograms = 5;

enum EParams
{
	mWaveform = 0,
	mAttack,
	mDecay,
	mSustain,
	mRelease,
	mFilterMode,
	mFilterCutoff,
	mFilterResonance,
	mFilterAttack,
	mFilterDecay,
	mFilterSustain,
	mFilerRelease,
	mFilterEnvelopeAmount,
	kNumParams
};

enum ELayout
{
  kWidth = GUI_WIDTH,
  kHeight = GUI_HEIGHT,

};

Synth::Synth(IPlugInstanceInfo instanceInfo)
  :	IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo)
{
  TRACE;

  //arguments are: name, defaultVal, minVal, maxVal, step, label
  //Hintergrund
  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
  pGraphics->AttachBackground(BACKGROUND_ID, BACKGROUND_FN);
  //Waveform
  GetParam(mWaveform)->InitEnum("Waveform", OSCILLATOR_MODE_SINE, kNumOscillatorModes);
  IBitmap waveformBitmap = pGraphics->LoadIBitmap(WAVEFORM_ID, WAVEFORM_FN, 4);
  pGraphics->AttachControl(new ISwitchControl(this, 55, 62, mWaveform, &waveformBitmap));
  //ADSR
  IBitmap knobBitmap = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, 41);
  //Attack
  GetParam(mAttack)->InitDouble("Attack", 0.01, 0.01, 10.0, 0.001);
  GetParam(mAttack)->SetShape(3);
  pGraphics->AttachControl(new IKnobMultiControl(this, 180, 53, mAttack, &knobBitmap));
  //Decay
  GetParam(mDecay)->InitDouble("Decay", 0.5, 0.01, 15.0, 0.001);
  GetParam(mDecay)->SetShape(3);
  pGraphics->AttachControl(new IKnobMultiControl(this, 280, 53, mDecay, &knobBitmap));
  //Sustain
  GetParam(mSustain)->InitDouble("Sustain", 0.1, 0.001, 1.0, 0.001);
  GetParam(mSustain)->SetShape(3);
  pGraphics->AttachControl(new IKnobMultiControl(this, 380, 53, mSustain, &knobBitmap));
  //Release
  GetParam(mRelease)->InitDouble("Release", 1.0, 0.001, 15.0, 0.001);
  GetParam(mRelease)->SetShape(3);
  pGraphics->AttachControl(new IKnobMultiControl(this, 480, 53, mRelease, &knobBitmap));
  //FilterMode
  GetParam(mFilterMode)->InitEnum("Filter Mode", Filter::FILTER_MODE_LOWPASS, Filter::kNumFilterModes);
  IBitmap filtermodeBitmap = pGraphics->LoadIBitmap(FILTERMODE_ID, FILTERMODE_FN, 3);
  pGraphics->AttachControl(new ISwitchControl(this, 55, 225, mFilterMode, &filtermodeBitmap));
  //Filter
  IBitmap knob2Bitmap = pGraphics->LoadIBitmap(KNOB2_ID, KNOB2_FN, 41);
  //Cutoff
  GetParam(mFilterCutoff)->InitDouble("Cutoff", 0.99, 0.01, 0.99, 0.001);
  GetParam(mFilterCutoff)->SetShape(2);
  pGraphics->AttachControl(new IKnobMultiControl(this, 280, 225, mFilterCutoff, &knob2Bitmap));
  //Resonance
  GetParam(mFilterResonance)->InitDouble("Resonance", 0.01, 0.01, 1.0, 0.001);
  pGraphics->AttachControl(new IKnobMultiControl(this, 380, 225, mFilterResonance, &knob2Bitmap));


	

  AttachGraphics(pGraphics);

  //MakePreset("preset 1", ... );
  CreatePresets();

  mMIDIReceiver.noteOn.Connect(this, &Synth::onNoteOn);
  mMIDIReceiver.noteOff.Connect(this, &Synth::onNoteOff);
}

Synth::~Synth() {}

void Synth::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
  // Mutex is already locked for us.

	double *leftOutput = outputs[0];
	double *rightOutput = outputs[1];

	for (int i = 0; i < nFrames; ++i)
	{
		mMIDIReceiver.advance();
		int velocity = mMIDIReceiver.getLastVelocity();
		if (velocity > 0)
		{
			mOscillator.setFrequency(mMIDIReceiver.getLastFrequency());
			mOscillator.setMuted(false);
		}
		else
		{
			mOscillator.setMuted(true);
		}
		
		//leftOutput[i] = rightOutput[i] = mOscillator.nextSample() * velocity / 127.0;
		leftOutput[i] = rightOutput[i] = mFilter.process(mOscillator.nextSample() * mADSREnvelope.nextSample() * velocity / 127.0);
	}
	mMIDIReceiver.Flush(nFrames);
  
}

void Synth::Reset()
{
  TRACE;
  IMutexLock lock(this);
  mOscillator.setSampleRate(GetSampleRate());
  mADSREnvelope.setSampleRate(GetSampleRate());
}

void Synth::OnParamChange(int paramIdx)
{
	IMutexLock lock(this);
	switch (paramIdx)
	{
	case mWaveform:
		mOscillator.setMode(static_cast<OscillatorMode>(GetParam(mWaveform)->Int()));
		break;
	case mFilterMode:
		mFilter.setFilterMode(static_cast<Filter::FilterMode>(GetParam(paramIdx)->Int()));
		break;
	case mAttack:
	case mDecay:
	case mSustain:
	case mRelease:
		mADSREnvelope.setStageValue(static_cast<ADSREnvelope::EnvelopeStage>(paramIdx), GetParam(paramIdx)->Value());
		break;
	case mFilterCutoff:
		mFilter.setCutoff(GetParam(paramIdx)->Value());
		break;
	case mFilterResonance:
		mFilter.setResonance(GetParam(paramIdx)->Value());
	}
}

void Synth::CreatePresets()
{
	MakeDefaultPreset((char *) "-", kNumPrograms);
}

void Synth::ProcessMidiMsg(IMidiMsg* pMsg)
{
	mMIDIReceiver.onMessageReceived(pMsg);
}
