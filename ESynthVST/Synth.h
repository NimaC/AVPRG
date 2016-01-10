#ifndef __SYNTH__
#define __SYNTH__

#pragma warning( suppress : 4101 4129 )
#include "IPlug_include_in_plug_hdr.h"
#include "Oscillator.h"
#include "MIDIReceiver.h"
#include "ADSREnvelope.h"
#include "Filter.h"

class Synth : public IPlug
{
public:
  Synth(IPlugInstanceInfo instanceInfo);
  ~Synth();

  void Reset();
  void OnParamChange(int paramIdx);
  void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames);
  void ProcessMidiMsg(IMidiMsg* pMsg);

private:
  double mFrequency;
  void CreatePresets();
  Oscillator mOscillator;
  MIDIReceiver mMIDIReceiver;
  ADSREnvelope mADSREnvelope;
  Filter mFilter;

  inline void onNoteOn(const int noteNumber, const int velocity) { mADSREnvelope.enterStage(ADSREnvelope::ENVELOPE_STAGE_ATTACK); };
  inline void onNoteOff(const int noteNumber, const int velocity) { mADSREnvelope.enterStage(ADSREnvelope::ENVELOPE_STAGE_RELEASE); };

};

#endif
