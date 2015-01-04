/*******************************************************************************

This is Delysynth, a simple and tiny additive synthesizer.

Written by Chris Vasseng 2014

*******************************************************************************/

#ifndef h_delsynth__types__
#define h_delsynth__types__

#include "del.synth.h"

namespace ds {
	////////////////////////////////////////////////////////////////////////////////
	//STRUCTURES AND DEFINES

	//Channel
	enum SChannel {
	  LEFT,
	  RIGHT
	};

	//Waveform identifier
	enum WaveForm {
	  W_SIN,
	  W_SAW,
	  W_SQU
	};

	//Modulation types
  enum ModulationType {
  	MOD_NOP 		 = 0x0000,

  	//OSCILLATOR 1
  	MOD_OSC1_SIN = 0x0100,
	  MOD_OSC1_SAW = 0x0101,
	  MOD_OSC1_TRI = 0x0102,
	  MOD_OSC1_SQU = 0x0103,
	  MOD_OSC1_WNO = 0x0104,
	  MOD_OSC1_ATK = 0x0105,
	  MOD_OSC1_DEC = 0x0106,
	  MOD_OSC1_SUS = 0x0107,
	  MOD_OSC1_REL = 0x0108,
	  MOD_OSC1_TUN = 0x0109,
	  MOD_OSC1_PAN = 0x010A,
	  MOD_OSC1_FRE = 0x010B,

	  //OSCILLATOR 2
	  MOD_OSC2_SIN = 0x0200,
	  MOD_OSC2_SAW = 0x0201,
	  MOD_OSC2_TRI = 0x0202,
	  MOD_OSC2_SQU = 0x0203,
	  MOD_OSC2_WNO = 0x0204,
	  MOD_OSC2_ATK = 0x0205,
	  MOD_OSC2_DEC = 0x0206,
	  MOD_OSC2_SUS = 0x0207,
	  MOD_OSC2_REL = 0x0208,
	  MOD_OSC2_TUN = 0x0209,
	  MOD_OSC2_PAN = 0x020A,
	  MOD_OSC2_FRE = 0x020B,

	  //CHANNEL
	  MOD_CHAN_VOL = 0x0300,

	  //LFO 1
	  MOD_LFO1_RAT = 0x0401,
	  MOD_LFO1_AMP = 0x0402,
	  MOD_LFO1_DEP = 0x0403,

	  //LFO 2
	  MOD_LFO2_RAT = 0x0501,
	  MOD_LFO2_AMP = 0x0502,
	  MOD_LFO2_DEP = 0x0503,

	  //3-pole eq
	  MOD_3POL_LOW = 0x0601,
	  MOD_3POL_MID = 0x0602,
	  MOD_3POL_HIG = 0x0603,

	  //Filter 1
	  MOD_FLT1_CUT = 0x0700,
	  MOD_FLT1_RES = 0x0701,

	};

	//Describes a state for envelopes
	enum EnvState {
	  E_ATTACK  = 0x01,
	  E_SUSTAIN = 0x02,
	  E_DECAY   = 0x03,
	  E_RELEASE = 0x04,
	  E_IDLE    = 0x05
	};

	//This is used when setting values
	enum ValueTarget {
	  OSC1 = 0x01,
	  OSC2 = 0x02,
	  CHAN = 0x03,
	  LFO1 = 0x04,
	  LFO2 = 0x05,
	  POL3 = 0x06,
	  FLT1 = 0x07
	};
	  
	//Contains the state of a single note.
	struct Note {
	  //Velocity
	  double velocity;
	  //Frequency
	  double freq;
	  //MIDI note
    int8 note;
	  
	  //Current envelope
	  double envelope[2];
	  //Envelope state
	  EnvState state[2];
	  
	  Note() {
	    note = 0;
	  }
	};

	//Modulation values for a single oscillator
	struct OSCModulation {
	  double sin; //Sin volume
	  double saw; //Saw volume
	  double tri; //Tri volume
	  double squ; //Squ volume
	  double noi; //Noise 1 volume
	  double atk; //Attack
	  double dec; //Decay
	  double sus; //Sustain
	  double rel; //Release
	  double tun; //Tuning
	  double pan; //Panning
	  double fre; //Frequency
	};

	//This is a single oscillator 
	struct Osc {
	  //Waveform
	  double sin;
	  double saw;
	  double tri;
	  double squ;
	  double noi;
	  //Envelope info
	  double attack;
	  double decay;
	  double sustain;
	  double release;

	  double tune;
	  double pan;
	  //Current envelope
	  double envelope;

	  //Modulations
	  OSCModulation mod;
	  
	  //Precalc values
	  double atkCoef; //calcCoef(s.attack * SAMPLE_RATE, tratioA);
	  double decCoef; //calcCoef(s.decay * SAMPLE_RATE, tratioDR);
	  double relCoef;	//calcCoef(s.release * SAMPLE_RATE, tratioDR)

	  double atkBase; //((1 + tratioA) * (1.0 - coef))
	  double decBase; //((s.sustain - tratioDR) * (1.0 - coef))
	  double relBase;	//(-tratioDR * (1.0 - coef))


	  Osc();
	};

	enum FilterMode {
		OFF = 0,
		HPASS,
		MPASS,
		LPASS
	};

	struct LBHPassMod {
		double cut;
		double res;
		LBHPassMod() {
			cut = 0;
			res = 0;
		}
	};

	//LBH pass filter
	/*
		See: http://www.martin-finke.de/blog/articles/audio-plugins-013-filter/
		When changing the cutoff/resonance, call calc_lbhpass_feedback.
	*/
	struct LBHPass {
		double cut;	//Cutoff
		double res;	//Resonance
		FilterMode mode;

		double feb; //Feedback

		//State
		double buf0;
		double buf1;
		double buf2;
		double buf3;

		bool enabled;

		LBHPassMod modulation;

		inline double getCalculatedCutoff() const {
    	return fmax(fmin(cut + modulation.cut, 0.99), 0.01);
		};

		inline void calculateFeedbackAmount() {
    	feb = res + res / (1.0 - getCalculatedCutoff());
		}

		LBHPass() {
			modulation.cut = modulation.res = buf0 = buf1 = buf2 = buf3 = 0.0;
		}
	};

	//3 pole filter
	//See http://www.musicdsp.org/showArchiveComment.php?ArchiveID=236
	struct EQSTATE {
	  //Gain Controls
	  double lg; // low gain
	  double mg; // mid gain
	  double hg; // high gain

	  // Filter #1 (Low band)
	  double lf; // Frequency
	  double f1p0; // Poles ...
	  double f1p1; 
	  double f1p2;
	  double f1p3;

	  // Filter #2 (High band)
	  double hf; // Frequency
	  double f2p0; // Poles ...
	  double f2p1;
	  double f2p2;
	  double f2p3;

	  // Sample history buffer
	  double sdm1; // Sample data minus 1
	  double sdm2; // 2
	  double sdm3; // 3

	  EQSTATE() {
	    // Clear state 
	    memset(this, 0, sizeof(EQSTATE));
	    
	    // Set Low/Mid/High gains to unity
	    lg = 1.0;
	    mg = 1.0;
	    hg = 1.0;

	    // Calculate filter cutoff frequencies
	    lf = 2 * sin(PI * ((double)800.0 / (double)SAMPLE_RATE));
	    hf = 2 * sin(PI * ((double)500.0 / (double)SAMPLE_RATE));
	  }

	}; 

	//LFO modulation
	struct LFOModulation {
		double rat;
		double dep;
		double amp;
	};

	//An LFO
	struct LFO {
	  double rat;
	  double dep;
	  double amp;

	  WaveForm wav;

	  ModulationType targetA;
	  ModulationType targetB;
	  ModulationType targetC;

	  //Current value
	  double value;
	  //Current modulation
	  LFOModulation mod;

	  LFO() {
	    wav = W_SIN;
	    rat = 1.1;
	    dep = 1;
	    amp = 1;
	    targetA = MOD_NOP;
	    targetB = MOD_NOP;
	    targetC = MOD_NOP;
	    value = 0;
	  }
	};

	//A channel. The idea is that there's one for each MIDI channel..
	struct Channel {
	  double volume;
	  double pan;

	  EQSTATE eq;
	  LFO lfo1;
	  LFO lfo2;
	  Osc osc[2];
	  LBHPass flt1;

	  //Voices
	  uint32 activeVoices;
	  Note voices[MAX_VOICES];
	  
	  Channel() {
	    volume = 0.5;
	    activeVoices = 0;

	    for (int i = 0; i < MAX_VOICES; i++) {
	      voices[i].state[0] = voices[i].state[1] = E_IDLE;
	    }
	  }
	};

	//////////////////////////////////////////////////////////////////////////////

	//This contains the packed state of a synth
	struct PackedOsc { //13 bytes
	  //Envelope
	  uint8 atk;  //0..20
	  uint8 rel;  //0..20
	  uint8 sus;  //0..1
	  uint8 dec;  //0..1

	  //Volumes
	  uint8 sin;  //0..1
	  uint8 saw;  //0..1
	  uint8 tri;  //0..1
	  uint8 squ;  //0..1
	  uint8 wno;  //0..1
	  uint8 pno;  //0..1

	  //Other
	  int8 pan;   //-0.5..0.5
	  int8 tune;  //??
	  int8 semi;  //??
	};

	//Packed LFO
	struct PackedLFO { //7 bytes
		uint8 rat;	//rate 			0..10
		uint8 dep;	//depth 		0..1
		uint8 amp;	//amp 			0..1
		uint8 wav;	//waveform 	[0, 1, 2]

		int32 taa;	//target a
		int32 tab;	//target b
		int32 tac;	//target c
	};

	//Packed 3 pole filter
	struct Packed3Pole { //3 bytes
		uint8 lg;		//Low gain 0..1
		uint8 mg; 	//Mid gain 0..1
		uint8 hg; 	//High gain 0..1
	};

	struct PackedLBHFilter {
		uint8 cut;
		uint8 res;
		uint8 mod;
	};

	//Packed channel
	struct PackedChannel { //27 bytes
	  PackedOsc oscillators[2];
	  PackedLFO lfos[2];
	  Packed3Pole flt3;
	  PackedLBHFilter fltLBH;
	  uint8 volume;             //0..1
	};

	struct Packed {
		int32 magic;
		int8 chancount;
		PackedChannel channels[MAX_CHANNELS];
	};

	//:(
	extern Channel ActiveChannels[MAX_CHANNELS];
	extern void precalc_envelope(Osc &o);
	extern bool setMod(uint8 channel, ModulationType what, double value);
}



#endif

