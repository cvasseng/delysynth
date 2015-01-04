/*******************************************************************************

This is Delysynth, a simple and tiny additive synthesizer.

Written by Chris Vasseng 2014

*******************************************************************************/

#ifndef h__delsynth_generators__
#define h__delsynth_generators__

#ifndef PI
  #define PI 3.14159265359
#endif 

#ifndef TWOPI
  #define TWOPI PI * 2
#endif

#ifndef SAMPLE_RATE
	#define SAMPLE_RATE 44100
#endif

namespace ds {

	//Generate a frame of a sine wave
	inline double gen_sin(unsigned int t, double freq, double velocity) {
	  return sin(TWOPI * t / (SAMPLE_RATE / freq)) * velocity;
	}

	//Generate a frame of a square wave
	inline double gen_squ(unsigned int t, double freq, double velocity) {
	  return gen_sin(t, freq, 1) >= 0 ? velocity : 0;
	}

	//Generate a frame of white noise
	inline double gen_wno(unsigned int t, double freq, double velocity) {
	  return (( (double)rand() / RAND_MAX - 1) * 2.0 - 1.0) * velocity;
	}

	//Generate a saw tooth
	inline double gen_saw(unsigned int t, double frequency, double amp) {
	  double samplesPerCycle = SAMPLE_RATE / frequency;
	  int sawIndex = t % (int)samplesPerCycle;
	  
	  double r = amp * (2.0 * (fmodf(sawIndex, samplesPerCycle) / samplesPerCycle) - 1.0);
	  
	  return r;
	}

	//Generate a triangle wave
	inline double gen_tri(unsigned int t, double freq, double velocity) {
	  return 0.0;
	}

}

#endif
