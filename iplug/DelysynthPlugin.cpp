#include "DelysynthPlugin.h"
#include "IPlug_include_in_plug_src.h"
#include "resource.h"

#include "IControl.h"
#include "IKeyboardControl.h"

#include "del.synth.h"

const int kNumPrograms = 8;

#define PITCH 440.

#ifndef M_PI
#define M_PI  (3.14159265)
#endif

enum EParams
{
  
  P_CHAN01_VOL = 0,
  P_CHAN02_VOL,
  P_CHAN03_VOL,
  P_CHAN04_VOL,
  P_CHAN05_VOL,
  P_CHAN06_VOL,
  P_CHAN07_VOL,
  P_CHAN08_VOL,
  P_CHAN09_VOL,
  P_CHAN10_VOL,
  P_CHAN11_VOL,
  P_CHAN12_VOL,
  P_CHAN13_VOL,
  P_CHAN14_VOL,
  P_CHAN15_VOL,
  P_CHAN16_VOL,
  
  kGainL,
  kGainR,
  kMode,
  
  
  //OSC 1 waveforms
  P_OSC1_SIN,
  P_OSC1_SQU,
  P_OSC1_TRI,
  P_OSC1_SAW,
  P_OSC1_WHI,
  P_OSC1_PIN,
  
  //OSC 1 AUX
  P_OSC1_SEM,
  P_OSC1_SN1,
  P_OSC1_SN2,
  
  //OSC 1 Envelope
  P_OSC1_ATK,
  P_OSC1_DEC,
  P_OSC1_SUS,
  P_OSC1_REL,
  
  //OSC 2 waveforms
  P_OSC2_SIN,
  P_OSC2_SQU,
  P_OSC2_TRI,
  P_OSC2_SAW,
  P_OSC2_WHI,
  P_OSC2_PIN,
  
  //OSC 2 AUX
  P_OSC2_SEM,
  P_OSC2_SN1,
  P_OSC2_SN2,
  
  //OSC 2 Envelope
  P_OSC2_ATK,
  P_OSC2_DEC,
  P_OSC2_SUS,
  P_OSC2_REL,
  
  P_CHAN_SEL,
  P_EQ_LG,
  P_EQ_MG,
  P_EQ_HG,
  
  P_LFO1_RATE,
  P_LFO1_DEPTH,
  P_LFO1_AMP,
  P_LFO1_TA,
  P_LFO1_TB,
  P_LFO1_TC,
  
  P_LFO2_RATE,
  P_LFO2_DEPTH,
  P_LFO2_AMP,
  P_LFO2_TA,
  P_LFO2_TB,
  P_LFO2_TC,
  
  P_CHAN_VOL,
  P_CHAN_PAN,
  
  
  kNumParams
};

DelysynthPlugin::DelysynthPlugin(IPlugInstanceInfo instanceInfo)
  : IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo),
    mGainL(1.),
    mGainR(1.),
    mNoteGain(0.),
    mPhase(0),
    mSampleRate(44100.),
    mFreq(440.),
    mNumKeys(0),
    mKey(-1),
    mPrevL(0.0),
    mPrevR(0.0)

{
  TRACE;
  
  mSelChannel = 0;
  
  ds::init();
  
  //Test if the set system works or if it crashes everything..
  //ds::ActiveChannels[0].lfo1.targetA = ds::MOD_OSC1_SQU;
  
  //ds::set(0, ds::LFO1_TAA, ds::MOD_OSC1_SAW);

  memset(mKeyStatus, 0, 128 * sizeof(bool));
  
  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
  IBitmap knob = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, kKnobFrames);
  IBitmap fader = pGraphics->LoadIBitmap(AFADER_ID, AFADER_FN);

  //arguments are: name, defaultVal, minVal, maxVal, step, label
  
  //////////////////////////////////////////////////////////////////////////////
  //OSCILATOR 1
  
  //Parameters
  GetParam(P_OSC1_SIN)->InitDouble("OSC1 Sine", 0, 0.0, 1.0, 0.01, "");
  GetParam(P_OSC1_SQU)->InitDouble("OSC1 Square", 0, 0.0, 1.0, 0.01, "");
  GetParam(P_OSC1_TRI)->InitDouble("OSC1 Triangle", 0, 0.0, 1.0, 0.01, "");
  GetParam(P_OSC1_SAW)->InitDouble("OSC1 Saw", 0, 0.0, 1.0, 0.01, "");
  GetParam(P_OSC1_WHI)->InitDouble("OSC1 White noise", 0, 0.0, 1.0, 0.01, "");
  GetParam(P_OSC1_PIN)->InitDouble("OSC1 Pink Noise", 0, 0.0, 1.0, 0.01, "");
  GetParam(P_OSC1_SEM)->InitDouble("OSC1 Semitune", 0, -20.0, 20.0, 1, "");
  GetParam(P_OSC1_SN1)->InitDouble("OSC1 Send 1", 0.5, 0.0, 1.0, 0.01, "");
  GetParam(P_OSC1_SN2)->InitDouble("OSC1 Panning", 0, -.5, 0.5, 0.01, "");
  GetParam(P_OSC1_ATK)->InitDouble("OSC1 Attack", 0.1, 0.0, 1.0, 0.01, "");
  GetParam(P_OSC1_DEC)->InitDouble("OSC1 Decay", 0, 0.0, 1.0, 0.01, "");
  GetParam(P_OSC1_SUS)->InitDouble("OSC1 Sustain", 1.0, 0.0, 1.0, 0.01, "");
  GetParam(P_OSC1_REL)->InitDouble("OSC1 Release", 0.1, 0.0, 10.0, 0.01, "");
  
  //Knobs
  pGraphics->AttachControl(new IKnobRotaterControl(this, 10, 60, P_OSC1_SIN, &knob));

  pGraphics->AttachControl(new IKnobRotaterControl(this, 70, 60, P_OSC1_SQU, &knob));
  pGraphics->AttachControl(new IKnobRotaterControl(this, 130, 60, P_OSC1_TRI, &knob));
  pGraphics->AttachControl(new IKnobRotaterControl(this, 190, 60, P_OSC1_SAW, &knob));
  pGraphics->AttachControl(new IKnobRotaterControl(this, 250, 60, P_OSC1_WHI, &knob));
  pGraphics->AttachControl(new IKnobRotaterControl(this, 310, 60, P_OSC1_PIN, &knob));
  
  pGraphics->AttachControl(new IKnobRotaterControl(this, 370, 60, P_OSC1_SEM, &knob));
  pGraphics->AttachControl(new IKnobRotaterControl(this, 430, 60, P_OSC1_SN1, &knob));
  pGraphics->AttachControl(new IKnobRotaterControl(this, 490, 60, P_OSC1_SN2, &knob));
  
  pGraphics->AttachControl(new IKnobRotaterControl(this, 550, 60, P_OSC1_ATK, &knob));
  pGraphics->AttachControl(new IKnobRotaterControl(this, 610, 60, P_OSC1_DEC, &knob));
  pGraphics->AttachControl(new IKnobRotaterControl(this, 670, 60, P_OSC1_SUS, &knob));
  pGraphics->AttachControl(new IKnobRotaterControl(this, 730, 60, P_OSC1_REL, &knob));
  
  
  //////////////////////////////////////////////////////////////////////////////
  //OSCILATOR 2
  
  //Parameters
  GetParam(P_OSC2_SIN)->InitDouble("OSC2 Sine", 0, 0.0, 1.0, 0.01, "");
  GetParam(P_OSC2_SQU)->InitDouble("OSC2 Square", 0, 0.0, 1.0, 0.01, "");
  GetParam(P_OSC2_TRI)->InitDouble("OSC2 Triangle", 0, 0.0, 1.0, 0.01, "");
  GetParam(P_OSC2_SAW)->InitDouble("OSC2 Saw", 0, 0.0, 1.0, 0.01, "");
  GetParam(P_OSC2_WHI)->InitDouble("OSC2 White noise", 0, 0.0, 1.0, 0.01, "");
  GetParam(P_OSC2_PIN)->InitDouble("OSC2 Pink Noise", 0, 0.0, 1.0, 0.01, "");
  GetParam(P_OSC2_SEM)->InitDouble("OSC2 Semitune", 0, -20.0, 20.0, 1, "");
  GetParam(P_OSC2_SN1)->InitDouble("OSC2 Send 1", 0.5, 0.0, 1.0, 0.01, "");
  GetParam(P_OSC2_SN2)->InitDouble("OSC2 Panning", 0, -0.5, 0.5, 0.01, "");
  GetParam(P_OSC2_ATK)->InitDouble("OSC2 Attack", 0.1, 0.0, 1.0, 0.01, "");
  GetParam(P_OSC2_DEC)->InitDouble("OSC2 Decay", 0, 0.0, 1.0, 0.01, "");
  GetParam(P_OSC2_SUS)->InitDouble("OSC2 Sustain", 1.0, 0.0, 1.0, 0.01, "");
  GetParam(P_OSC2_REL)->InitDouble("OSC2 Release", 0.1, 0.0, 10.0, 0.01, "");
  
  //Knobs
  pGraphics->AttachControl(new IKnobRotaterControl(this, 10,  135, P_OSC2_SIN, &knob, -0.75 * PI, 0.75 * PI, 1));
  pGraphics->AttachControl(new IKnobRotaterControl(this, 70,  135, P_OSC2_SQU, &knob));
  pGraphics->AttachControl(new IKnobRotaterControl(this, 130, 135, P_OSC2_TRI, &knob));
  pGraphics->AttachControl(new IKnobRotaterControl(this, 190, 135, P_OSC2_SAW, &knob));
  pGraphics->AttachControl(new IKnobRotaterControl(this, 250, 135, P_OSC2_WHI, &knob));
  pGraphics->AttachControl(new IKnobRotaterControl(this, 310, 135, P_OSC2_PIN, &knob));
  
  pGraphics->AttachControl(new IKnobRotaterControl(this, 370, 135, P_OSC2_SEM, &knob));
  pGraphics->AttachControl(new IKnobRotaterControl(this, 430, 135, P_OSC2_SN1, &knob));
  pGraphics->AttachControl(new IKnobRotaterControl(this, 490, 135, P_OSC2_SN2, &knob));
  
  pGraphics->AttachControl(new IKnobRotaterControl(this, 550, 135, P_OSC2_ATK, &knob));
  pGraphics->AttachControl(new IKnobRotaterControl(this, 610, 135, P_OSC2_DEC, &knob));
  pGraphics->AttachControl(new IKnobRotaterControl(this, 670, 135, P_OSC2_SUS, &knob));
  pGraphics->AttachControl(new IKnobRotaterControl(this, 730, 135, P_OSC2_REL, &knob));
  
  //////////////////////////////////////////////////////////////////////////////
  //LFO 1
  
  GetParam(P_LFO1_RATE)->InitDouble("LFO1 Rate", 1, 0, 10, 0.05, "hz");
  GetParam(P_LFO1_DEPTH)->InitDouble("LFO1 Depth", 1, 0, 5, 0.01, "");
  GetParam(P_LFO1_AMP)->InitDouble("LFO1 Amp", 1, 0, 1, 0.1, "");
  
  pGraphics->AttachControl(new IKnobRotaterControl(this, 70,  210, P_LFO1_RATE, &knob));
  pGraphics->AttachControl(new IKnobRotaterControl(this, 130, 210, P_LFO1_DEPTH, &knob));
  pGraphics->AttachControl(new IKnobRotaterControl(this, 190, 210, P_LFO1_AMP, &knob));
  pGraphics->AttachControl(new ITargetSelector(this, IRECT(260, 212, 350, 222), P_LFO1_TA, &lfo1A));
  pGraphics->AttachControl(new ITargetSelector(this, IRECT(260, 225, 350, 235), P_LFO1_TB, &lfo1B));
  pGraphics->AttachControl(new ITargetSelector(this, IRECT(260, 238, 350, 248), P_LFO1_TC, &lfo1C));
  
  //////////////////////////////////////////////////////////////////////////////
  //LFO 2
  
  GetParam(P_LFO2_RATE)->InitDouble("LFO2 Rate", 1, 0, 10, 0.05, "hz");
  GetParam(P_LFO2_DEPTH)->InitDouble("LFO2 Depth", 1, 0, 5, 0.01, "");
  GetParam(P_LFO2_AMP)->InitDouble("LFO2 Amp", 1, 0, 1, 0.1, "");
  
  pGraphics->AttachControl(new IKnobRotaterControl(this, 70,  285, P_LFO2_RATE, &knob));
  pGraphics->AttachControl(new IKnobRotaterControl(this, 130, 285, P_LFO2_DEPTH, &knob));
  pGraphics->AttachControl(new IKnobRotaterControl(this, 190, 285, P_LFO2_AMP, &knob));
  pGraphics->AttachControl(new ITargetSelector(this, IRECT(260, 287, 350, 297), P_LFO2_TA, &lfo2A));
  pGraphics->AttachControl(new ITargetSelector(this, IRECT(260, 300, 350, 310), P_LFO2_TB, &lfo2B));
  pGraphics->AttachControl(new ITargetSelector(this, IRECT(260, 313, 350, 323), P_LFO2_TC, &lfo2C));
  
  
  //////////////////////////////////////////////////////////////////////////////
  GetParam(P_CHAN_SEL)->InitDouble("Channel", 0, 0, MAX_CHANNELS, 1, "ch");
  
  //EQ
  GetParam(P_EQ_LG)->InitDouble("Filter Low Gain", 1.0, 0.0, 4.0, 0.05, "");
  GetParam(P_EQ_MG)->InitDouble("Filter Mid Gain", 1.0, 0.0, 4.0, 0.05, "");
  GetParam(P_EQ_HG)->InitDouble("Filter High Gain", 1.0, 0.0, 4.0, 0.05, "");
  
  pGraphics->AttachControl(new IKnobRotaterControl(this, 10, 435, P_EQ_LG, &knob));
  pGraphics->AttachControl(new IKnobRotaterControl(this, 70, 435, P_EQ_MG, &knob));
  pGraphics->AttachControl(new IKnobRotaterControl(this, 130, 435, P_EQ_HG, &knob));
  
  //Channel
  //GetParam(P_CHAN_VOL)->InitDouble("Channel Volume", 0.5, 0.0, 1.0, 0.01, "");
  //GetParam(P_CHAN_PAN)->InitDouble("Channel Panning", 0.0, -0.5, 0.5, 0.1, "");
  //pGraphics->AttachControl(new IKnobRotaterControl(this, 730, 210, P_CHAN_VOL, &knob));
 // pGraphics->AttachControl(new IKnobRotaterControl(this, 670, 435, P_CHAN_PAN, &knob));
  
  
  GetParam(kGainL)->InitDouble("GainL", -12.0, -70.0, 12.0, 0.1, "dB");
  GetParam(kGainR)->InitDouble("GainR", -12.0, -70.0, 12.0, 0.1, "dB");
  GetParam(kMode)->InitEnum("Mode", 0, 6);
  GetParam(kMode)->SetDisplayText(0, "a");
  GetParam(kMode)->SetDisplayText(1, "b");
  GetParam(kMode)->SetDisplayText(2, "c");
  GetParam(kMode)->SetDisplayText(3, "d");
  GetParam(kMode)->SetDisplayText(4, "e");
  GetParam(kMode)->SetDisplayText(5, "f");
  
  //MIXER
  GetParam(P_CHAN01_VOL)->InitDouble("Chan01 Vol", 0.5, 0.0, 1.0, 0.01, "");
  GetParam(P_CHAN02_VOL)->InitDouble("Chan02 Vol", 0.5, 0.0, 1.0, 0.01, "");
  GetParam(P_CHAN03_VOL)->InitDouble("Chan03 Vol", 0.5, 0.0, 1.0, 0.01, "");
  GetParam(P_CHAN04_VOL)->InitDouble("Chan04 Vol", 0.5, 0.0, 1.0, 0.01, "");
  GetParam(P_CHAN05_VOL)->InitDouble("Chan05 Vol", 0.5, 0.0, 1.0, 0.01, "");
  GetParam(P_CHAN06_VOL)->InitDouble("Chan06 Vol", 0.5, 0.0, 1.0, 0.01, "");
  GetParam(P_CHAN07_VOL)->InitDouble("Chan07 Vol", 0.5, 0.0, 1.0, 0.01, "");
  GetParam(P_CHAN08_VOL)->InitDouble("Chan08 Vol", 0.5, 0.0, 1.0, 0.01, "");
  GetParam(P_CHAN09_VOL)->InitDouble("Chan09 Vol", 0.5, 0.0, 1.0, 0.01, "");
  GetParam(P_CHAN10_VOL)->InitDouble("Chan10 Vol", 0.5, 0.0, 1.0, 0.01, "");
  GetParam(P_CHAN11_VOL)->InitDouble("Chan11 Vol", 0.5, 0.0, 1.0, 0.01, "");
  GetParam(P_CHAN12_VOL)->InitDouble("Chan12 Vol", 0.5, 0.0, 1.0, 0.01, "");
  GetParam(P_CHAN13_VOL)->InitDouble("Chan13 Vol", 0.5, 0.0, 1.0, 0.01, "");
  GetParam(P_CHAN14_VOL)->InitDouble("Chan14 Vol", 0.5, 0.0, 1.0, 0.01, "");
  GetParam(P_CHAN15_VOL)->InitDouble("Chan15 Vol", 0.5, 0.0, 1.0, 0.01, "");
  GetParam(P_CHAN16_VOL)->InitDouble("Chan16 Vol", 0.5, 0.0, 1.0, 0.01, "");

  for (int i = 0; i < 16; i++) {
    pGraphics->AttachControl(new IFaderControl(this, 12 + (i * 49), 508, 126, P_CHAN01_VOL + i, &fader, kVertical));
  }
  
  //pGraphics->AttachControl(new IFaderControl(this, 16, 508, 126, P_CHAN02_VOL, &fader, kVertical));

  
  pGraphics->AttachBackground(BG_ID, BG_FN);
  
  pGraphics->AttachControl(new IPresetMenu(this, IRECT(5, 20, 250, 35)));
  pGraphics->AttachControl(new IChannelSelector(this, IRECT(280, 20, 480, 35), &mSelChannel));

  IBitmap about = pGraphics->LoadIBitmap(ABOUTBOX_ID, ABOUTBOX_FN);
  mAboutBox = new IBitmapOverlayControl(this, 100, 100, &about, IRECT(540, 250, 680, 290));
  pGraphics->AttachControl(mAboutBox);
  AttachGraphics(pGraphics);

  //MakePreset("preset 1", ... );
  //MakeDefaultPreset((char *) "-", kNumPrograms);
}

DelysynthPlugin::~DelysynthPlugin() {}

void DelysynthPlugin::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames) {
  // Mutex is already locked for us.
//  double* in1 = inputs[0];
//  double* in2 = inputs[1];
  double* out1 = outputs[0];
  double* out2 = outputs[1];
  //double peakL = 0.0, peakR = 0.0;

  GetTime(&mTimeInfo);

  for (int offset = 0; offset < nFrames; ++offset, /*++in1, ++in2,*/ ++out1, ++out2) {
    while (!mMidiQueue.Empty()) {
      IMidiMsg* pMsg = mMidiQueue.Peek();
      if (pMsg->mOffset > offset) break;

      // TODO: make this work on win sa
#if !defined(OS_WIN) && !defined(SA_API)
      SendMidiMsg(pMsg);
#endif

      if (pMsg->StatusMsg() == IMidiMsg::kNoteOn && pMsg->Velocity() > 0) { //on
        ds::noteOn(pMsg->Channel(), pMsg->NoteNumber(), pMsg->Velocity());
      } else { //Off
        ds::noteOff(pMsg->Channel(), pMsg->NoteNumber());
      }
      
      mMidiQueue.Remove();
    }

    *out1 = ds::processLeft();
    *out2 = ds::processRight();

    //peakL = IPMAX(peakL, fabs(*out1));
    //peakR = IPMAX(peakR, fabs(*out2));
  }

  //const double METER_ATTACK = 0.6, METER_DECAY = 0.05;
  //double xL = (peakL < mPrevL ? METER_DECAY : METER_ATTACK);
  //double xR = (peakR < mPrevR ? METER_DECAY : METER_ATTACK);

  //peakL = peakL * xL + mPrevL * (1.0 - xL);
  //peakR = peakR * xR + mPrevR * (1.0 - xR);

  //mPrevL = peakL;
  //mPrevR = peakR;

  //if (GetGUI())
  //{
   // GetGUI()->SetControlFromPlug(mMeterIdx_L, peakL);
    // GetGUI()->SetControlFromPlug(mMeterIdx_R, peakR);
  //}

  mMidiQueue.Flush(nFrames);
}

void DelysynthPlugin::Reset()
{
  TRACE;
  IMutexLock lock(this);

  mSampleRate = GetSampleRate();
  mMidiQueue.Resize(GetBlockSize());
}

void DelysynthPlugin::OnParamChange(int paramIdx)
{
  IMutexLock lock(this);
  
  if (paramIdx < 16) {
    //Channel volume change
    ds::set(paramIdx, ds::CHAN_VOL, GetParam(paramIdx)->Value());
    return;
  }

  switch (paramIdx)
  {
    case kGainL:
      mGainL = GetParam(kGainL)->DBToAmp();
      break;
    case kGainR:
      mGainR = GetParam(kGainR)->DBToAmp();
      break;
      
    //OSCILATOR #1
    case P_OSC1_SIN:
      ds::set(mSelChannel, ds::OSC1_SIN, GetParam(P_OSC1_SIN)->Value());
      break;
    case P_OSC1_SQU:
      ds::set(mSelChannel, ds::OSC1_SQU, GetParam(P_OSC1_SQU)->Value());
      break;
    case P_OSC1_SAW:
      ds::set(mSelChannel, ds::OSC1_SAW, GetParam(P_OSC1_SAW)->Value());
      break;
    case P_OSC1_TRI:
      ds::set(mSelChannel, ds::OSC1_TRI, GetParam(P_OSC1_TRI)->Value());
      break;
    case P_OSC1_WHI:
      ds::set(mSelChannel, ds::OSC1_WNO, GetParam(P_OSC1_WHI)->Value());
      break;
    case P_OSC1_SEM:
      ds::set(mSelChannel, ds::OSC1_TUN, GetParam(P_OSC1_SEM)->Value());
      break;
    case P_OSC1_ATK:
      ds::set(mSelChannel, ds::OSC1_ATK, GetParam(P_OSC1_ATK)->Value());
      break;
    case P_OSC1_DEC:
      ds::set(mSelChannel, ds::OSC1_DEC, GetParam(P_OSC1_DEC)->Value());
      break;
    case P_OSC1_SUS:
      ds::set(mSelChannel, ds::OSC1_SUS, GetParam(P_OSC1_SUS)->Value());
      break;
    case P_OSC1_REL:
      ds::set(mSelChannel, ds::OSC1_REL, GetParam(P_OSC1_REL)->Value());
      break;
    case P_OSC1_SN2:
      ds::set(mSelChannel, ds::OSC1_PAN, GetParam(P_OSC1_SN2)->Value());
      break;
      
    //OSCILATOR #2
    case P_OSC2_SIN:
      ds::set(mSelChannel, ds::OSC2_SIN, GetParam(P_OSC2_SIN)->Value());
      break;
    case P_OSC2_SQU:
      ds::set(mSelChannel, ds::OSC2_SQU, GetParam(P_OSC2_SQU)->Value());
      break;
    case P_OSC2_SAW:
      ds::set(mSelChannel, ds::OSC2_SAW, GetParam(P_OSC2_SAW)->Value());
      break;
    case P_OSC2_TRI:
      ds::set(mSelChannel, ds::OSC2_TRI, GetParam(P_OSC2_TRI)->Value());
      break;
    case P_OSC2_WHI:
      ds::set(mSelChannel, ds::OSC2_WNO, GetParam(P_OSC2_WHI)->Value());
      break;
    case P_OSC2_SEM:
      ds::set(mSelChannel, ds::OSC2_TUN, GetParam(P_OSC2_SEM)->Value());
      break;
    case P_OSC2_ATK:
      ds::set(mSelChannel, ds::OSC2_ATK, GetParam(P_OSC2_ATK)->Value());
      break;
    case P_OSC2_DEC:
      ds::set(mSelChannel, ds::OSC2_DEC, GetParam(P_OSC2_DEC)->Value());
      break;
    case P_OSC2_SUS:
      ds::set(mSelChannel, ds::OSC2_SUS, GetParam(P_OSC2_SUS)->Value());
      break;
    case P_OSC2_REL:
      ds::set(mSelChannel, ds::OSC2_REL, GetParam(P_OSC2_REL)->Value());
      break;
    case P_OSC2_SN2:
      ds::set(mSelChannel, ds::OSC2_PAN, GetParam(P_OSC2_SN2)->Value());
      break;
      
    //EQ
    case P_EQ_LG:
     // ds::ActiveChannels[mSelChannel].synth.eq.lg = GetParam(P_EQ_LG)->Value();
      ds::set(mSelChannel, ds::POL3_LOW, GetParam(P_EQ_LG)->Value());
      break;
    case P_EQ_MG:
      //ds::ActiveChannels[mSelChannel].synth.eq.mg = GetParam(P_EQ_MG)->Value();
      ds::set(mSelChannel, ds::POL3_MID, GetParam(P_EQ_MG)->Value());
      break;
    case P_EQ_HG:
      //ds::ActiveChannels[mSelChannel].synth.eq.hg = GetParam(P_EQ_HG)->Value();
      ds::set(mSelChannel, ds::POL3_HIG, GetParam(P_EQ_HG)->Value());
      break;
      
    //Channel
    case P_CHAN_PAN:
     // ds::ActiveChannels[mSelChannel].pan = GetParam(P_CHAN_PAN)->Value();
      break;
    case P_CHAN_VOL:
      ds::set(mSelChannel, ds::CHAN_VOL, GetParam(P_CHAN_VOL)->Value());
      break;
      
    //LFO 1
    case P_LFO1_RATE:
      ds::set(mSelChannel, ds::LFO1_RAT, GetParam(P_LFO1_RATE)->Value());
      break;
    case P_LFO1_DEPTH:
      ds::set(mSelChannel, ds::LFO1_DEP, GetParam(P_LFO1_DEPTH)->Value());
      break;
    case P_LFO1_AMP:
      ds::set(mSelChannel, ds::LFO1_AMP, GetParam(P_LFO1_AMP)->Value());
      break;
    case P_LFO1_TA:
      ds::set(mSelChannel, ds::LFO1_TAA, lfo1A);
      break;
    case P_LFO1_TB:
      ds::set(mSelChannel, ds::LFO1_TAB, lfo1B);
      break;
    case P_LFO1_TC:
      ds::set(mSelChannel, ds::LFO1_TAC, lfo1C);
      break;
      
    //LFO 2
    case P_LFO2_RATE:
      ds::set(mSelChannel, ds::LFO2_RAT, GetParam(P_LFO2_RATE)->Value());
      break;
    case P_LFO2_DEPTH:
      ds::set(mSelChannel, ds::LFO2_DEP, GetParam(P_LFO2_DEPTH)->Value());
      break;
    case P_LFO2_AMP:
      ds::set(mSelChannel, ds::LFO2_AMP, GetParam(P_LFO2_AMP)->Value());
      break;
    case P_LFO2_TA:
      ds::set(mSelChannel, ds::LFO2_TAA, lfo2A);
      break;
    case P_LFO2_TB:
      ds::set(mSelChannel, ds::LFO2_TAB, lfo2B);
      break;
    case P_LFO2_TC:
      ds::set(mSelChannel, ds::LFO2_TAC, lfo2C);
      break;
      
      
    //Sel channel
    case 0xFF00FF:
      //selChannel()
      //Update the UI
      printf("Selected channel: %i\n", mSelChannel);
      GetGUI()->SetParameterFromPlug(P_OSC1_SIN, ds::get(mSelChannel, ds::OSC1_SIN), false);
      GetGUI()->SetParameterFromPlug(P_OSC1_TRI, ds::get(mSelChannel, ds::OSC1_TRI), false);
      GetGUI()->SetParameterFromPlug(P_OSC1_SQU, ds::get(mSelChannel, ds::OSC1_SQU), false);
      GetGUI()->SetParameterFromPlug(P_OSC1_SAW, ds::get(mSelChannel, ds::OSC1_SAW), false);
      GetGUI()->SetParameterFromPlug(P_OSC1_WHI, ds::get(mSelChannel, ds::OSC1_WNO), false);
      GetGUI()->SetParameterFromPlug(P_OSC1_SEM, ds::get(mSelChannel, ds::OSC1_TUN), false);
      GetGUI()->SetParameterFromPlug(P_OSC1_SN2, ds::get(mSelChannel, ds::OSC1_PAN), false);
      GetGUI()->SetParameterFromPlug(P_OSC1_ATK, ds::get(mSelChannel, ds::OSC1_ATK), false);
      GetGUI()->SetParameterFromPlug(P_OSC1_DEC, ds::get(mSelChannel, ds::OSC1_DEC), false);
      GetGUI()->SetParameterFromPlug(P_OSC1_SUS, ds::get(mSelChannel, ds::OSC1_SUS), false);
      GetGUI()->SetParameterFromPlug(P_OSC1_REL, ds::get(mSelChannel, ds::OSC1_REL), false);
      
      GetGUI()->SetParameterFromPlug(P_OSC2_SIN, ds::get(mSelChannel, ds::OSC2_SIN), false);
      GetGUI()->SetParameterFromPlug(P_OSC2_TRI, ds::get(mSelChannel, ds::OSC2_TRI), false);
      GetGUI()->SetParameterFromPlug(P_OSC2_SQU, ds::get(mSelChannel, ds::OSC2_SQU), false);
      GetGUI()->SetParameterFromPlug(P_OSC2_SAW, ds::get(mSelChannel, ds::OSC2_SAW), false);
      GetGUI()->SetParameterFromPlug(P_OSC2_WHI, ds::get(mSelChannel, ds::OSC2_WNO), false);
      GetGUI()->SetParameterFromPlug(P_OSC2_SEM, ds::get(mSelChannel, ds::OSC2_TUN), false);
      GetGUI()->SetParameterFromPlug(P_OSC2_SN2, ds::get(mSelChannel, ds::OSC2_PAN), false);
      GetGUI()->SetParameterFromPlug(P_OSC2_ATK, ds::get(mSelChannel, ds::OSC2_ATK), false);
      GetGUI()->SetParameterFromPlug(P_OSC2_DEC, ds::get(mSelChannel, ds::OSC2_DEC), false);
      GetGUI()->SetParameterFromPlug(P_OSC2_SUS, ds::get(mSelChannel, ds::OSC2_SUS), false);
      GetGUI()->SetParameterFromPlug(P_OSC2_REL, ds::get(mSelChannel, ds::OSC2_REL), false);
      
      GetGUI()->SetParameterFromPlug(P_CHAN_VOL, ds::get(mSelChannel, ds::CHAN_VOL), false);
      
      //DirtyParameters();
      
     // SerializeState(<#ByteChunk *pChunk#>)
      
      break;
    
    default:
      break;
  }
}

void DelysynthPlugin::ProcessMidiMsg(IMidiMsg* pMsg)
{
  int status = pMsg->StatusMsg();
  int velocity = pMsg->Velocity();

  switch (status)
  {
    case IMidiMsg::kNoteOn:
    case IMidiMsg::kNoteOff:
      // filter only note messages
      if (status == IMidiMsg::kNoteOn && velocity)
      {
        mKeyStatus[pMsg->NoteNumber()] = true;
        mNumKeys += 1;
      }
      else
      {
        mKeyStatus[pMsg->NoteNumber()] = false;
        mNumKeys -= 1;
      }
      break;
    default:
      return; // if !note message, nothing gets added to the queue
  }

  mMidiQueue.Add(pMsg);
}

// Should return non-zero if one or more keys are playing.
int DelysynthPlugin::GetNumKeys()
{
  IMutexLock lock(this);
  return mNumKeys;
}

// Should return true if the specified key is playing.
bool DelysynthPlugin::GetKeyStatus(int key)
{
  IMutexLock lock(this);
  return mKeyStatus[key];
}

//Called by the standalone wrapper if someone clicks about
bool DelysynthPlugin::HostRequestingAboutBox()
{
  IMutexLock lock(this);
  if(GetGUI())
  {
    // get the IBitmapOverlay to show
    mAboutBox->SetValueFromPlug(1.);
    mAboutBox->Hide(false);
  }
  return true;
}
