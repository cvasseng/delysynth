/*******************************************************************************

This is Delysynth, a simple and tiny additive synthesizer.

Written by Chris Vasseng 2014

*******************************************************************************/

#include "del.synth.h"
#include "del.types.h"
#include "del.generators.h"

#include <stdlib.h>
#include <math.h>
#include <iostream>

#ifndef PI
  #define PI 3.14159265359
#endif 

#ifndef TWOPI
  #define TWOPI PI * 2
#endif

using namespace ds;

////////////////////////////////////////////////////////////////////////////////
//GLOBALS
Channel ds::ActiveChannels[MAX_CHANNELS];

static double vsa = (1.0 / 4294967295.0);

uint32 g_framecount = 0;

//Used inside misc functions - defined here so we don't have to allocate stuff.
double output = 0.0, soutput = 0.0;
double output_left = 0.0;
double output_right = 0.0;
bool do_lfos = false;

//Used for envelope calculation
double tratioA = 0.3, tratioDR = 0.0001;

double wave_tbl_wno[SAMPLE_RATE * BUFFER_SIZE];

////////////////////////////////////////////////////////////////////////////////
//FORWARD DECLERATIONS
///bool setMod(uint8 channel, ModulationType what, double value);

////////////////////////////////////////////////////////////////////////////////
//UTILITY FUNCTIONS

//Convert a MIDI note to a frequency
inline double midiNoteToFreq(char midiNote, double tuning = 440) {
  return tuning * pow(2.0, (midiNote - 69.0) / 12.0);
}

//Calculate the coefficient between two numbers
inline double calcCoef(double rate, double targetRatio) {
  return exp(-log((1.0 + targetRatio) / targetRatio) / rate);
}

//Pan an input by a factor of pan
inline double pan(double input, double pan, SChannel chan) {
  return chan == LEFT ? sqrt(0.5 - pan) * input : sqrt(0.5 + pan) * input;
}

inline double clamp(double in, double min, double max) {
  return in > max ? max : (in < min ? min : in);
}

//Calculate filter feedback
inline void calc_lbhpass_feedback(LBHPass& filter) {
  filter.feb = filter.res + filter.res / (1.0 - filter.cut);
}

////////////////////////////////////////////////////////////////////////////////
//CALCULATE WAVE TABLES

//Calculate the white noise wave form
void calc_wno() {
  for (int i = 0; i < SAMPLE_RATE * BUFFER_SIZE; ++i) {
    wave_tbl_wno[i] = gen_wno(i, 0.0, 1.0);
  }
}

void calc_wavetables() {
  calc_wno();
}

void ds::init() {
  calc_wavetables();
}

////////////////////////////////////////////////////////////////////////////////
//EFFECTS

//This is a simple 3 pole filter
inline double fx_eq(EQSTATE &es, double sample) {
  // Locals
  double l,m,h; // Low / Mid / High - Sample Values

  // Filter #1 (lowpass)
  es.f1p0 += (es.lf * (sample - es.f1p0)) + vsa;
  es.f1p1 += (es.lf * (es.f1p0 - es.f1p1));
  es.f1p2 += (es.lf * (es.f1p1 - es.f1p2));
  es.f1p3 += (es.lf * (es.f1p2 - es.f1p3));

  l = es.f1p3;

  // Filter #2 (highpass)
  es.f2p0 += (es.hf * (sample - es.f2p0)) + vsa;
  es.f2p1 += (es.hf * (es.f2p0 - es.f2p1));
  es.f2p2 += (es.hf * (es.f2p1 - es.f2p2));
  es.f2p3 += (es.hf * (es.f2p2 - es.f2p3));

  h = es.sdm3 - es.f2p3;

  // Calculate midrange (signal - (low + high))

  //m = es.sdm3 - (h + l);
  m = sample - (h + l);

  // Scale, Combine and store

  l *= es.lg;
  m *= es.mg;
  h *= es.hg;

  // Shuffle history buffer 

  es.sdm3 = es.sdm2;
  es.sdm2 = es.sdm1;
  es.sdm1 = sample; 

  // Return result

  return(l + m + h);
}

inline double fx_lbh(LBHPass &f, double inputValue) {
  if (inputValue == 0.0 || f.mode == OFF) {
    return inputValue;
  }  

  f.cut = f.cut >= 1.0 ? 0.999999 : f.cut;

  f.calculateFeedbackAmount();
  double calculatedCutoff = f.getCalculatedCutoff();

  f.buf0 += calculatedCutoff * (inputValue - f.buf0 + f.feb * (f.buf0 - f.buf1));
  f.buf1 += calculatedCutoff * (f.buf0 - f.buf1);
  f.buf2 += calculatedCutoff * (f.buf1 - f.buf2);
  f.buf3 += calculatedCutoff * (f.buf2 - f.buf3);
  
  switch (f.mode) {
    case LPASS:
        return f.buf3;
    case HPASS:
        return inputValue - f.buf3;
    case MPASS:
        return f.buf0 - f.buf3;
    default:
        return 0.0;
  }
}

////////////////////////////////////////////////////////////////////////////////

//Generate a frame based on an osc
inline double gen_osc(Osc &o, unsigned int t, double freq, double vel, SChannel chan) {
  //Apply modulations
  freq += o.mod.fre;

	return pan(
             (
              (o.sin > 0 ? gen_sin(t, freq, (vel * o.sin) * o.mod.sin) : 0.0) +
              (o.tri > 0 ? gen_tri(t, freq, (vel * o.tri) * o.mod.tri) : 0.0) +
              (o.squ > 0 ? gen_squ(t, freq, (vel * o.squ) * o.mod.squ) : 0.0) +
              (o.saw > 0 ? gen_saw(t, freq, (vel * o.saw) * o.mod.saw) : 0.0) +
              (o.noi > 0 ? wave_tbl_wno[t % SAMPLE_RATE * BUFFER_SIZE] * (vel * o.noi) : 0.0)
             ),
             o.pan + clamp(o.mod.pan, -0.5, 0.5), 
             chan
         );
}

//Calculate the current envelope for a given note
inline double gen_env(Osc &s, EnvState &state, double &envelope) {
  //When done attacking, do decay.
  //When decay level reached, do sustain
  //When done releasing, do idle
  
  if (state == E_ATTACK) {
    envelope = s.atkBase + envelope * s.atkCoef;
    
    if (envelope >= 1.0) {
      state = E_DECAY;
      envelope = 1.0;
    }
  } else if (state == E_DECAY) {

    envelope = s.decBase + envelope * s.decCoef;
    if (envelope <= s.sustain) {
      envelope = s.sustain;
      state = E_SUSTAIN;
    }
  } else if (state == E_RELEASE) {
    envelope = s.relBase + envelope * s.relCoef;
    if (envelope <= 0.0) {
      state = E_IDLE;
      envelope = 0.0;
    }
    
  } else if (state == E_IDLE) {
    return 0.0;
  } else if (state == E_SUSTAIN) {
    envelope = s.sustain;
  }
  
  return envelope;
}


inline void process_lfo(uint32 t, LFO &lfo, uint8 targetChannel = 0) {
  double l = lfo.value;

  //Update the LFO
  if (lfo.wav == W_SIN) {
    lfo.value = gen_sin(t, lfo.rat, lfo.amp) * lfo.dep;
  } else if (lfo.wav == W_SQU) {
    lfo.value = gen_squ(t, lfo.rat, lfo.amp) * lfo.dep;
  }

  //Set modulation values
  if (l != lfo.value) {
    lfo.targetA != MOD_NOP ? setMod(targetChannel, lfo.targetA, lfo.value) : NULL;
    lfo.targetB != MOD_NOP ? setMod(targetChannel, lfo.targetB, lfo.value) : NULL;
    lfo.targetC != MOD_NOP ? setMod(targetChannel, lfo.targetC, lfo.value) : NULL;
  }
}

//Process channels

double process_channels(uint32 t, SChannel chan) {
  output = 0.0;

  do_lfos = t % 100 == 1;
  
  for (uint8 i = 0; i < MAX_CHANNELS; i++) {
    Channel *chn = &ActiveChannels[i];

    if (chn->activeVoices > 0 && chn->volume > 0.00001) {

      soutput = 0;

      for (int j = 0; j < MAX_VOICES; j++) {
        Note *n = &chn->voices[j];
      
        for (int o = 0; o < 2; o++) {
          if (n->state[o] != E_IDLE) {
            soutput += gen_osc(chn->osc[o], t, n->freq + chn->osc[o].tune, n->velocity, chan) * gen_env(chn->osc[o], n->state[o], n->envelope[o]);
          }
        }
      }

      //soutput = fx_lbh(chn->flt1, soutput);
      soutput = fx_eq(chn->eq, soutput);

      output = output + (soutput * chn->volume);

      if (output > 0.00001 && do_lfos) {
        process_lfo(t, ActiveChannels[i].lfo1, i);
        process_lfo(t, ActiveChannels[i].lfo2, i);
      }

    }
  }
  
  //Shitty clipping

  output = output > 1.0 ? 1.0 : output;

  return output;
}

//Start playing a note on an synth
bool syn_startNote(Channel &o, int8 midiNote, int8 velocity) {
  int ind = -1, lowest = 0, same = -1;
  
  o.activeVoices++;
  
  //Find the first zero voice
  for (uint8 i = 0; i < MAX_VOICES - 1; i++) {  
    if (o.voices[i].state[0] == E_IDLE && o.voices[i].state[1] == E_IDLE) {
      ind = i;
      break;
    } else if (o.voices[i].note == midiNote) {
      same = i;
      break;
    } else if (o.voices[i].velocity < o.voices[lowest].velocity) {
      lowest = i;
    } 
  }

  if (ind >= 0) {         //There's a free slot
  } else if (same >= 0) { //Reactivate the same note
    ind = same; 
  } else {                //use the note with the lowest volume
    ind = lowest;
  }

  //Initializse the voice
  o.voices[ind].state[0] = o.voices[ind].state[1] = E_ATTACK;
  o.voices[ind].envelope[0] = o.voices[ind].envelope[1] = 0.0;
  o.voices[ind].note = midiNote;
  o.voices[ind].freq = midiNoteToFreq(midiNote);
  o.voices[ind].velocity = (double)velocity / 127.0;
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////
//PRE-CALC FUNCTIONS - BECAUSE WITHOUT IT, IT'S SLOW AS SHIT

//Pre calculate envelope - call every time set is called with env modifications.
void ds::precalc_envelope(Osc &o) {
  o.atkCoef = calcCoef(o.attack * SAMPLE_RATE, tratioA);
  o.decCoef = calcCoef(o.decay * SAMPLE_RATE, tratioDR);
  o.relCoef = calcCoef(o.release * SAMPLE_RATE, tratioDR);

  o.atkBase = (1 + tratioA) * (1.0 - o.atkCoef);
  o.decBase = (o.sustain - tratioDR) * (1.0 - o.decCoef);
  o.relBase = -tratioDR * (1.0 - o.relCoef);
}

////////////////////////////////////////////////////////////////////////////////

//Set a modulation value
inline bool ds::setMod(uint8 channel, ModulationType what, double value) {
  if (channel >= MAX_CHANNELS) {
    return false;
  }

  if (what == MOD_NOP) {
    return true;
  }

  double *ptr = NULL;

  ValueTarget target = ValueTarget( (what & 0xFF00) >> 8 );
  uint16 offset = (what & 0x00FF) * 8;

  //Right, so now we know what to set and what value to set it to.
  if (target == OSC1) {
    ptr = (double *)((char *)&(ActiveChannels[channel].osc[0].mod) + offset);
  } else if (target == OSC2) {
    ptr = (double *)((char *)&(ActiveChannels[channel].osc[1].mod) + offset);
  } else if (target == CHAN) {
  //  ptr = (double *)((char *)&(ActiveChannels[channel]) + offset);
  } else if (target == LFO1) {
    ptr = (double *)((char *)&(ActiveChannels[channel].lfo1.mod) + offset);
  } else if (target == LFO2) {
    ptr = (double *)((char *)&(ActiveChannels[channel].lfo2.mod) + offset);
  } else if (target == POL3) {

  } else if (target == FLT1) {
    ptr = (double *)((char *)&(ActiveChannels[channel].flt1.modulation) + offset);
  }

  if (ptr) {
    *ptr = value;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
//Loading util functions - turns packed settings into unpacked stuff

//Load OSC from a packed OSC
void cpy_osc(PackedOsc &src, Osc &target) {
  target.sin = (double)src.sin / 127.0;
  target.saw = (double)src.saw / 127.0;
  target.tri = (double)src.tri / 127.0;
  target.squ = (double)src.squ / 127.0;
  target.noi = (double)src.wno / 127.0;

  target.attack = (double)(src.atk / 127.0) * 20.0;
  target.release = (double)(src.rel / 127.0) * 20.0;
  target.sustain = (double)src.sus / 127.0;
  target.decay = (double)src.dec / 127.0;

  target.pan = 0.5 - ((double)src.pan / 127.0);
}

//Load an LFO from a packed LFO
void cpy_lfo(PackedLFO &src, LFO &target) {
  target.rat = (double)(src.rat / 127.0) * 10;
  target.dep = (double)src.dep / 127.0;
  target.amp = (double)src.amp / 127.0;
  target.wav = WaveForm(src.wav);
  target.targetA = ModulationType(src.taa);
  target.targetB = ModulationType(src.tab);
  target.targetC = ModulationType(src.tac);
}

//Load a filter from a packed filter
void cpy_flt3(Packed3Pole& src, EQSTATE &target) {
  target.lg = (double)src.lg / 127.0;
  target.mg = (double)src.mg / 127.0;
  target.hg = (double)src.hg / 127.0;
}

//Load a LBH filter from a packed one
void cpy_lbh(PackedLBHFilter &src, LBHPass &target) {
  target.cut = (double)src.cut / 127.0;
  target.res = (double)src.res / 127.0;
  target.mode = FilterMode((int)src.mod);
}

////////////////////////////////////////////////////////////////////////////////
//Constructors

//Inititalize osc
ds::Osc::Osc() {
  memset(this, 0, sizeof(Osc));
  memset(&mod, 0, sizeof(OSCModulation));
  
  attack = 0.05;
  sustain = 1.0;
  release = 0.5;

  mod.sin = mod.saw = mod.tri = mod.squ = mod.noi = 1.0;
  precalc_envelope(*this);
}

////////////////////////////////////////////////////////////////////////////////
//Basic public functions

//Load instruments from a char buffer
/*  
  Data is organized as such:

  | channel count | channel 1 | channel 2| ...
  |     1 byte    | sizeof(PackedChannel) | ... 

*/
bool ds::load(const int8 *data) {
  Packed *d = new Packed;

  memcpy(d, data, sizeof(*d));

  if (MAGIC_NUMBER != d->magic) {
    return false;
  }

  for (int32 i = 0; i < d->chancount; i++) {
    if (i < MAX_CHANNELS) {
      //Unpack the channel
      cpy_osc(d->channels[i].oscillators[0], ActiveChannels[i].osc[0]);
      cpy_osc(d->channels[i].oscillators[1], ActiveChannels[i].osc[1]);
      cpy_lfo(d->channels[i].lfos[0], ActiveChannels[i].lfo1);
      cpy_lfo(d->channels[i].lfos[1], ActiveChannels[i].lfo2);
      cpy_flt3(d->channels[i].flt3, ActiveChannels[i].eq);
      cpy_lbh(d->channels[i].fltLBH, ActiveChannels[i].flt1);

      //Do some precalcing
      precalc_envelope(ActiveChannels[i].osc[0]);
      precalc_envelope(ActiveChannels[i].osc[1]);
    }
  }

  return true;
}

//Process left channel
double ds::processLeft() {
  return process_channels(++g_framecount, LEFT);
}

//Process right channel
double ds::processRight() {
  return process_channels(++g_framecount, RIGHT);
}

//Fill up a buffer
bool ds::fillBuffer(int16 *buffer) {

  for (uint32 i = 0; i < BUFFER_SIZE / sizeof(int16); i += 2) {

    output_left = output_right = process_channels(g_framecount, LEFT);

    buffer[i] = (int16)(output_left * 32767);
    buffer[i + 1] = (int16)(output_right * 32767);

    g_framecount++;
  }
  
  return true;
}

//Turn a note on
void ds::noteOn(uint8 channel, int8 note, int8 velocity) {
  if (channel < MAX_CHANNELS) {
    syn_startNote(ActiveChannels[channel], note, velocity);
  }
}

//Turn a note off
void ds::noteOff(uint8 channel, int8 note) {
  if (channel < MAX_CHANNELS) {
    for (uint8 i = 0; i < MAX_VOICES; i++) {
      if (ActiveChannels[channel].voices[i].note == note) {
        ActiveChannels[channel].voices[i].state[0] = E_RELEASE;
        ActiveChannels[channel].voices[i].state[1] = E_RELEASE;
        ActiveChannels[channel].activeVoices--;
        //return;
      }
    }
  }
}

