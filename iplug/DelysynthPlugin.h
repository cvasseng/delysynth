#ifndef __DELYSYNTHPLUGIN__
#define __DELYSYNTHPLUGIN__

#include "IPlug_include_in_plug_hdr.h"
#include "IMidiQueue.h"

#include "del.synth.h"

#include <string>
#include <sstream>

// http://www.musicdsp.org/archive.php?classid=3#257

class CParamSmooth
{
public:
  CParamSmooth() { a = 0.99; b = 1. - a; z = 0.; };
  ~CParamSmooth() {};
  inline double Process(double in) { z = (in * b) + (z * a); return z; }
private:
  double a, b, z;
};

const IColor COLOR_LGRAY(102, 102, 102, 102);

class DelysynthPlugin : public IPlug
{
public:

  DelysynthPlugin(IPlugInstanceInfo instanceInfo);
  ~DelysynthPlugin();

  void Reset();
  void OnParamChange(int paramIdx);

  void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames);
  bool HostRequestingAboutBox();

  int GetNumKeys();
  bool GetKeyStatus(int key);
  void ProcessMidiMsg(IMidiMsg* pMsg);
  
  int lfo1A;
  int lfo1B;
  int lfo1C;
  
  int lfo2A;
  int lfo2B;
  int lfo2C;
  
  void selChannel(unsigned int chan) {
    mSelChannel = chan;
  }
  
  // Implementations should set a mutex lock and call SerializeParams() after custom data is serialized
  virtual bool SerializeState(ByteChunk* pChunk) {
    IMutexLock lock(this);
    
    //return true;
    
    //Serialize the state on all channels here.
    int size;
    char* data = ds::getFullState(size);
    pChunk->PutBytes(data, size);
    
    return SerializeParams(pChunk);;
  }
  
  // Return the new chunk position (endPos).
  int UnserializeState(ByteChunk *pChunk, int startPos) {
    IMutexLock lock(this);
    //return startPos;
    
    if (pChunk->Size() < ds::packedChanSize()) {
      return UnserializeParams(pChunk, startPos);
    }
    
    ds::int32 magic;
    
    pChunk->GetBytes(&magic, sizeof(magic), startPos);
    
    if (magic != MAGIC_NUMBER) {
      return UnserializeParams(pChunk, startPos);
    }

    char* data = new char[ds::packedChanSize()];
    startPos = pChunk->GetBytes(data, ds::packedChanSize(), startPos);
    
    ds::load(data);
    
    mSelChannel = 0;
    OnParamChange(0xFF00FF);
    
    return UnserializeParams(pChunk, startPos);
  }
  
protected:
  

private:
  IBitmapOverlayControl* mAboutBox;

  int mMeterIdx_L, mMeterIdx_R;

  IMidiQueue mMidiQueue;

  int mNumKeys; // how many keys are being played (via midi)
  bool mKeyStatus[128]; // array of on/off for each key

  int mPhase;
  int mNote;
  int mKey;
  
  int mSelChannel;

  double mGainL, mGainR;
  double mSampleRate;
  double mFreq;
  double mNoteGain;
  double mPrevL, mPrevR;

  ITimeInfo mTimeInfo;

  CParamSmooth mGainLSmoother, mGainRSmoother;
};

class ITargetListener {
  virtual void onSelect(uint32 item) = 0;
};

#include <vector>
#include <string>

struct ModTarget {
  std::string name;
  int val;
  ModTarget(const std::string& n, int v) {
    name = n;
    val = v;
  }
};

class ITargetSelector : public IControl {
public:
  ITargetSelector(IPlugBase *pPlug, IRECT pR, int tid, int *listener = NULL)
  : IControl(pPlug, pR, -1) {
    mTextEntryLength = MAX_PRESET_NAME_LEN - 3;
    mText = IText(9, &COLOR_WHITE, "Monospace", IText::kStyleNormal, IText::kAlignNear);
    mListener = listener;
    mTid = tid;
    
    //Create a map that maps an index to the actual value
    mHash.push_back(ModTarget("None", 0x0));
    mHash.push_back(ModTarget("- - - - - -", 0x0));
    
    mHash.push_back(ModTarget("MOD_OSC1_SIN", 0x0100));
    mHash.push_back(ModTarget("MOD_OSC1_SAW", 0x0101));
	  mHash.push_back(ModTarget("MOD_OSC1_TRI", 0x0102));
	  mHash.push_back(ModTarget("MOD_OSC1_SQU", 0x0103));
    mHash.push_back(ModTarget("MOD_OSC1_WNO", 0x0104));
	  mHash.push_back(ModTarget("MOD_OSC1_ATK", 0x0105));
	  mHash.push_back(ModTarget("MOD_OSC1_DEC", 0x0106));
	  mHash.push_back(ModTarget("MOD_OSC1_SUS", 0x0107));
	  mHash.push_back(ModTarget("MOD_OSC1_REL", 0x0108));
	  mHash.push_back(ModTarget("MOD_OSC1_TUN", 0x0109));
	  mHash.push_back(ModTarget("MOD_OSC1_PAN", 0x010A));
	  mHash.push_back(ModTarget("MOD_OSC1_FRE", 0x010B));
    
    mHash.push_back(ModTarget("- - - - - -", 0x0));
    
    mHash.push_back(ModTarget("MOD_OSC2_SIN", 0x0200));
    mHash.push_back(ModTarget("MOD_OSC2_SAW", 0x0201));
	  mHash.push_back(ModTarget("MOD_OSC2_TRI", 0x0202));
	  mHash.push_back(ModTarget("MOD_OSC2_SQU", 0x0203));
    mHash.push_back(ModTarget("MOD_OSC2_WNO", 0x0204));
	  mHash.push_back(ModTarget("MOD_OSC2_ATK", 0x0205));
	  mHash.push_back(ModTarget("MOD_OSC2_DEC", 0x0206));
	  mHash.push_back(ModTarget("MOD_OSC2_SUS", 0x0207));
	  mHash.push_back(ModTarget("MOD_OSC2_REL", 0x0208));
	  mHash.push_back(ModTarget("MOD_OSC2_TUN", 0x0209));
	  mHash.push_back(ModTarget("MOD_OSC2_PAN", 0x020A));
	  mHash.push_back(ModTarget("MOD_OSC2_FRE", 0x020B));
    
    mHash.push_back(ModTarget("- - - - - -", 0x0));
    
    //LFO 1
	  mHash.push_back(ModTarget("MOD_LFO1_RAT", 0x0401));
	  mHash.push_back(ModTarget("MOD_LFO1_AMP", 0x0402));
	  mHash.push_back(ModTarget("MOD_LFO1_DEP", 0x0403));
    
    mHash.push_back(ModTarget("- - - - - -", 0x0));
    
	  //LFO 2
	  mHash.push_back(ModTarget("MOD_LFO2_RAT", 0x0501));
	  mHash.push_back(ModTarget("MOD_LFO2_AMP", 0x0502));
	  mHash.push_back(ModTarget("MOD_LFO2_DEP", 0x0503));
    
  }
  
  bool Draw(IGraphics* pGraphics) {

   // mDisp.SetFormatted(32, "-");
    
    pGraphics->FillIRect(&COLOR_LGRAY, &mRECT);
    
    if (CSTR_NOT_EMPTY(mDisp.Get())) {
      return pGraphics->DrawIText(&mText, mDisp.Get(), &mRECT);
    }
    
    return true;
  }
  
  void doPopupMenu() {
    IPopupMenu menu;
    IGraphics* gui = mPlug->GetGUI();
    
    for (int i = 0; i < mHash.size(); i++) {
      menu.AddItem(mHash[i].name.c_str(), i);
    }
    
    if(gui->CreateIPopupMenu(&menu, &mRECT)) {
      int itemChosen = menu.GetChosenItemIdx();
      
      if (itemChosen > -1) {
        mDisp.Set(mHash[itemChosen].name.c_str());
        if (mListener) {
          *mListener = mHash[itemChosen].val;
          mPlug->OnParamChange(mTid);
        }
        //Redraw(); // seems to need this
        //SetDirty();
       // mPlug->DirtyParameters();
      }
     // printf("SEL %i", itemChosen);
    }
  }
  
  void TextFromTextEntry(const char* txt) {
    WDL_String safeName;
    safeName.Set(txt, MAX_PRESET_NAME_LEN);
    
    //mPlug->ModifyCurrentPreset(safeName.Get());
    //mPlug->InformHostOfProgramChange();
    //mPlug->DirtyParameters();
    SetDirty(false);
  }
  
  void OnMouseDown(int x, int y, IMouseMod* pMod) {
    // if (pMod->R) {
    //   const char* pname = mPlug->GetPresetName(mPlug->GetCurrentPresetIdx());
    //   mPlug->GetGUI()->CreateTextEntry(this, &mText, &mRECT, pname);
    // } else {
    doPopupMenu();
    //}
    
    Redraw(); // seems to need this
    SetDirty();
  }
  
  
protected:
private:
  WDL_String mDisp;
  int *mListener;
  std::vector<ModTarget> mHash;
  int mTid;
};

class IChannelSelector : public IControl {
public:
  IChannelSelector(IPlugBase *pPlug, IRECT pR, int *chanPtr = NULL)
  : IControl(pPlug, pR, -1)
  {
    mTextEntryLength = MAX_PRESET_NAME_LEN - 3;
    mText = IText(14, &COLOR_BLACK, "Arial", IText::kStyleNormal, IText::kAlignNear);
    mSelected = 0;
    mListener = chanPtr;
  }
  
  bool Draw(IGraphics* pGraphics)
  {

    mDisp.SetFormatted(32, "Channel %02d", mSelected + 1);
    
    pGraphics->FillIRect(&COLOR_LGRAY, &mRECT);
    
    if (CSTR_NOT_EMPTY(mDisp.Get()))
    {
      return pGraphics->DrawIText(&mText, mDisp.Get(), &mRECT);
    }
    
    return true;
  }
  
  void doPopupMenu() {
    IPopupMenu menu;
    IGraphics* gui = mPlug->GetGUI();
    
    for (int i = 0; i < MAX_CHANNELS; i++) {
      WDL_String s;
      s.SetFormatted(32, "Channel %02d", i + 1);
      menu.AddItem(s.Get(), i);
    }
    
    if(gui->CreateIPopupMenu(&menu, &mRECT)) {
      int itemChosen = menu.GetChosenItemIdx();
      
      if (itemChosen > -1) {
        mSelected = itemChosen;
        if (mListener) {
          *mListener = mSelected;
        }
        mPlug->OnParamChange(0xFF00FF);
        
        Redraw(); // seems to need this
        SetDirty();
      }
    }
  }
  
  void TextFromTextEntry(const char* txt) {
    WDL_String safeName;
    safeName.Set(txt, MAX_PRESET_NAME_LEN);
    
    //mPlug->ModifyCurrentPreset(safeName.Get());
    //mPlug->InformHostOfProgramChange();
    //mPlug->DirtyParameters();
    SetDirty(false);
  }
  
  void OnMouseDown(int x, int y, IMouseMod* pMod) {
   // if (pMod->R) {
   //   const char* pname = mPlug->GetPresetName(mPlug->GetCurrentPresetIdx());
   //   mPlug->GetGUI()->CreateTextEntry(this, &mText, &mRECT, pname);
   // } else {
      doPopupMenu();
    //}
    
    Redraw(); // seems to need this
    SetDirty();
  }

  
protected:
private:
  WDL_String mDisp;
  int mSelected;
  int *mListener;
};

class IPresetMenu : public IControl
{
private:
  WDL_String mDisp;
public:
  IPresetMenu(IPlugBase *pPlug, IRECT pR)
  : IControl(pPlug, pR, -1)
  {
    mTextEntryLength = MAX_PRESET_NAME_LEN - 3;
    mText = IText(14, &COLOR_BLACK, "Arial", IText::kStyleNormal, IText::kAlignNear);
  }
  
  bool Draw(IGraphics* pGraphics)
  {
    int pNumber = mPlug->GetCurrentPresetIdx();
    mDisp.SetFormatted(32, "%02d: %s", pNumber+1, mPlug->GetPresetName(pNumber));
    
    pGraphics->FillIRect(&COLOR_LGRAY, &mRECT);
    
    if (CSTR_NOT_EMPTY(mDisp.Get()))
    {
      return pGraphics->DrawIText(&mText, mDisp.Get(), &mRECT);
    }
    
    return true;
  }
  
  void OnMouseDown(int x, int y, IMouseMod* pMod)
  {
    if (pMod->R)
    {
      const char* pname = mPlug->GetPresetName(mPlug->GetCurrentPresetIdx());
      mPlug->GetGUI()->CreateTextEntry(this, &mText, &mRECT, pname);
    }
    else
    {
      doPopupMenu();
    }
    
    Redraw(); // seems to need this
    SetDirty();
  }
  
  void doPopupMenu()
  {
    int numItems = mPlug->NPresets();
    IPopupMenu menu;
    
    IGraphics* gui = mPlug->GetGUI();
    
    int currentPresetIdx = mPlug->GetCurrentPresetIdx();
    
    for(int i = 0; i< numItems; i++)
    {
      const char* str = mPlug->GetPresetName(i);
      if (i == currentPresetIdx)
        menu.AddItem(str, -1, IPopupMenuItem::kChecked);
      else
        menu.AddItem(str);
    }
    
    menu.SetPrefix(2);
    
    if(gui->CreateIPopupMenu(&menu, &mRECT))
    {
      int itemChosen = menu.GetChosenItemIdx();
      
      if (itemChosen > -1)
      {
        mPlug->RestorePreset(itemChosen);
        mPlug->InformHostOfProgramChange();
        mPlug->DirtyParameters();
      }
    }
  }
  
  void TextFromTextEntry(const char* txt)
  {
    WDL_String safeName;
    safeName.Set(txt, MAX_PRESET_NAME_LEN);
    
    mPlug->ModifyCurrentPreset(safeName.Get());
    mPlug->InformHostOfProgramChange();
    mPlug->DirtyParameters();
    SetDirty(false);
  }
};

enum ELayout
{
  kWidth = GUI_WIDTH,  // width of plugin window
  kHeight = GUI_HEIGHT, // height of plugin window

  kKeybX = 1,
  kKeybY = 233,

  kGainX = 100,
  kGainY = 100,
  kKnobFrames = 1
};

#endif //__DELYSYNTHPLUGIN__
