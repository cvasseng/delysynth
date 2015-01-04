/*******************************************************************************

This is Delysynth, a simple and tiny additive synthesizer.

Written by Chris Vasseng 2014

*******************************************************************************/

#ifndef SYNTH_64K

#include "del.types.h"
#include "del.synth.h"

#include <stdio.h>

using namespace ds;

//Copy the state and scale it to 0..127
void cpy_osc(Osc &src, PackedOsc &target) {
  //Waveforms
  target.sin = (int8)(src.sin * 127);
  target.saw = (int8)(src.saw * 127);
  target.tri = (int8)(src.tri * 127);
  target.squ = (int8)(src.squ * 127);
  target.wno = (int8)(src.noi * 127);

  //Envelopes
  target.atk = (int8)( (1 / src.attack) * 127.0);
  target.rel = (int8)( (1 / src.release) * 127.0);
  target.sus = (int8)(src.sustain * 127.0);
  target.dec = (int8)(src.decay * 127.0);

  target.pan = (int8)((0.5 + src.pan) * 127);

  //Other
  //int8 pan;   //-0.5..0.5
  //int8 tune;  //??
  //int8 semi;  //??

  //target.pan = (int8)0;
}

void cpy_flt3(EQSTATE &src, Packed3Pole &target) {
  target.lg = (int8)(src.lg * 127.0);
  target.mg = (int8)(src.mg * 127.0);
  target.hg = (int8)(src.hg * 127.0);
}

void cpy_lfo(LFO &src, PackedLFO &target) {
  target.rat = (int8)((1.0 / src.rat) * 127);
  target.dep = (int8)(src.dep * 127.0);
  target.amp = (int8)(src.amp * 127.0);
  target.wav = (int8)src.wav;
  target.taa = src.targetA;
  target.tab = src.targetB;
  target.tac = src.targetC;
}

void cpy_lbh(LBHPass &src, PackedLBHFilter &target) {
  target.cut = (int8)(src.cut * 127.0);
  target.res = (int8)(src.res * 127.0);
  target.mod = (int)target.mod;
}

//Reset the modulation for an entry
void clr_mod(uint8 channel, ModulationType tp) {
  if (tp == MOD_NOP) {
    return;
  }
  
  ValueTarget otarget = ValueTarget( (tp & 0xFF00) >> 8 );
  setMod(channel, tp, 0.0);
}

//Save state to buffer
int8* ds::getFullState(int32 &size) {
  //Right so we want to create a snapshot of the full state.
  Packed *d = new Packed;

  d->magic = MAGIC_NUMBER;
  d->chancount = MAX_CHANNELS;

  for (int16 i = 0; i < MAX_CHANNELS; i++) {
    cpy_osc(ActiveChannels[i].osc[0], d->channels[i].oscillators[0]);
    cpy_osc(ActiveChannels[i].osc[1], d->channels[i].oscillators[1]);

    cpy_lfo(ActiveChannels[i].lfo1, d->channels[i].lfos[0]);
    cpy_lfo(ActiveChannels[i].lfo2, d->channels[i].lfos[1]);

    cpy_flt3(ActiveChannels[i].eq, d->channels[i].flt3);

    cpy_lbh(ActiveChannels[i].flt1, d->channels[i].fltLBH);

    d->channels[i].volume = ActiveChannels[i].volume;
  }

  size = sizeof(*d);

  return (int8*)d;
}


////////////////////////////////////////////////////////////////////////////////

//Set the value of one of the sound parameters 
bool ds::set(uint8 channel, ValueType what, double value) {
  if (channel >= MAX_CHANNELS) {
    return false;
  }

  //Figure out the target to set
  ValueTarget target = ValueTarget( (what & 0xFF00) >> 8 );

  //We need to handle 6 cases manually..
  if (what == LFO1_TAA) { 
    clr_mod(channel, ActiveChannels[channel].lfo1.targetA);
    ActiveChannels[channel].lfo1.targetA = ModulationType((int)value); 
    return true; 
  }

  if (what == LFO1_TAB) { 
    clr_mod(channel, ActiveChannels[channel].lfo1.targetB);
    ActiveChannels[channel].lfo1.targetB = ModulationType((int)value); 
    return true; 
  }

  if (what == LFO1_TAC) { 
    clr_mod(channel, ActiveChannels[channel].lfo1.targetC);
    ActiveChannels[channel].lfo1.targetC = ModulationType((int)value); 
    return true; 
  }

  if (what == LFO2_TAA) { 
    clr_mod(channel, ActiveChannels[channel].lfo2.targetA);
    ActiveChannels[channel].lfo2.targetA = ModulationType((int)value); 
    return true; 
  }

  if (what == LFO2_TAB) { 
    clr_mod(channel, ActiveChannels[channel].lfo2.targetB);
    ActiveChannels[channel].lfo2.targetB = ModulationType((int)value); 
    return true; 
  }

  if (what == LFO2_TAC) { 
    clr_mod(channel, ActiveChannels[channel].lfo2.targetC);
    ActiveChannels[channel].lfo2.targetC = ModulationType((int)value); 
    return true; 
  }

  if (what == FLT1_MOD) {
    ActiveChannels[channel].flt1.mode = FilterMode((int)value);
    return true;
  }

  double *ptr = NULL;

  //The member number
  uint16 of = (what & 0x00FF);
  uint16 offset = of * 8;

  //Right, so now we know what to set and what value to set it to.
  if (target == OSC1) {
    ptr = (double *)((char *)&(ActiveChannels[channel].osc[0]) + offset);
  } else if (target == OSC2) {
    ptr = (double *)((char *)&(ActiveChannels[channel].osc[1]) + offset);
  } else if (target == POL3) {
    ptr = (double *)((char *)&(ActiveChannels[channel].eq) + offset);
  } else if (target == LFO1) {
    ptr = (double *)((char *)&(ActiveChannels[channel].lfo1) + offset);
  } else if (target == LFO2) {
    ptr = (double *)((char *)&(ActiveChannels[channel].lfo2) + offset);
  } else if (target == CHAN) {
    ptr = (double *)((char *)&(ActiveChannels[channel]) + offset);
  } else if (target == FLT1) {
    ptr = (double *)((char *)&(ActiveChannels[channel].flt1) + offset);
  }

  if (of >= 5 && of <= 8) {
    if (target == OSC1) {
      precalc_envelope(ActiveChannels[channel].osc[0]);
    } else if (target == OSC2) {
      precalc_envelope(ActiveChannels[channel].osc[1]);
    }
  }

  if (ptr) {
    *ptr = value;
    return true;
  }

  return false;
}
////////////////////////////////////////////////////////////////////////////////



int32 ds::packedChanSize() {
  return sizeof(Packed);
}

//Get the value of a specific attribute on a specific channel
double ds::get(uint8 channel, ValueType what) {
  if (channel >= MAX_CHANNELS) {
    return false;
  }

  if (what == FLT1_MOD) {
    return ActiveChannels[channel].flt1.mode;
  }

  //if (what == FLT2_MOD) {
   // return ActiveChannels[channel].flt1.mode;
 // }


  //Figure out the target to set
  ValueTarget target = ValueTarget( (what & 0xFF00) >> 8 );

  double *ptr = NULL;

  //The member number
  uint16 offset = (what & 0x00FF) * 8;

  //Right, so now we know what to set and what value to set it to.
  if (target == OSC1) {
    ptr = (double *)((char *)&(ActiveChannels[channel].osc[0]) + offset);
  } else if (target == OSC2) {
    ptr = (double *)((char *)&(ActiveChannels[channel].osc[1]) + offset);
  } else if (target == POL3) {
    ptr = (double *)((char *)&(ActiveChannels[channel].eq) + offset);
  } else if (target == LFO1) {
    ptr = (double *)((char *)&(ActiveChannels[channel].lfo1) + offset);
  } else if (target == LFO2) {
    ptr = (double *)((char *)&(ActiveChannels[channel].lfo2) + offset);
  } else if (target == CHAN) {
    ptr = (double *)((char *)&(ActiveChannels[channel]) + offset);
  } else if (target == FLT1) {
    ptr = (double *)((char *)&(ActiveChannels[channel].flt1) + offset);
  }
  
  if (ptr) {
    //printf("Getting %X value %f\n", what, *ptr);
    return *ptr;
  }

  return 0;
}


//Load instruments from file
bool ds::loadInstruments(const char* filename) {
  FILE *f =  fopen(filename, "rb");
  
  if (!f) {
    return false;
  }

  Packed *p = new Packed;
  fread(p, sizeof(*p), 1, f);
  
  ds::load((int8*)p);

  fclose(f);

  return true;
}

//Save packed instruments to file
bool ds::saveInstruments(const char* filename) {
  FILE *f =  fopen(filename, "wb");
  
  if (!f) {
    return false;
  }

  int32 size;
  int8 *data = ds::getFullState(size);
  
  fwrite(data, size, 1, f);

  fclose(f);

  return true;
}


#endif
