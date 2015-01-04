/*******************************************************************************

This is Delysynth, a simple and tiny additive synthesizer.

Written by Chris Vasseng 2014

*******************************************************************************/

#ifndef h__delsynth__
#define h__delsynth__

#ifndef BUFFER_SIZE
	#define BUFFER_SIZE 2048
#endif

#ifndef SAMPLE_RATE
	#define SAMPLE_RATE 44100
#endif

#ifndef MAX_VOICES
	#define MAX_VOICES 8
#endif

#ifndef MAX_CHANNELS
	#define MAX_CHANNELS 16
#endif

#ifndef PI
  #define PI 3.14159265359
#endif 

#ifndef TWOPI
  #define TWOPI PI * 2
#endif

#define MAGIC_NUMBER 0xBADA

#define MAX_DELAY_LENGTH (SAMPLE_RATE * BUFFER_SIZE)

#include <stdlib.h>
#include <math.h>
#include <string.h>


namespace ds {
	//////////////////////////////////////////////////////////////////////////////
	//Type aliases
	typedef char int8;
	typedef unsigned char uint8;
	typedef short int16;
	typedef unsigned short uint16;
	typedef int int32;
	typedef unsigned int uint32;
	typedef long int64;
	typedef unsigned long uint64;
  
  //Init
  extern void init();
	//Turn a note on
	extern void noteOn(uint8 channel, int8 note, int8 velocity);
	//Turn a note off
	extern void noteOff(uint8 channel, int8 note);

	//Fill a buffer
	extern bool fillBuffer(short *buffer);

  //Process left channel
  extern double processLeft();
  //Process right channel
  extern double processRight();

  //Load instruments and midi data from a char array
  extern bool load(const int8 *data);

  //The stuff in here is only needed for the plugin version
  #ifndef SYNTH_64K

  	//High bit is ValueTarget, low bit is offset on target
		enum ValueType {
			//OSCILLATOR 1
		  OSC1_SIN = 0x0100,
		  OSC1_SAW = 0x0101,
		  OSC1_TRI = 0x0102,
		  OSC1_SQU = 0x0103,
		  OSC1_WNO = 0x0104,
		  OSC1_ATK = 0x0105,
		  OSC1_DEC = 0x0106,
		  OSC1_SUS = 0x0107,
		  OSC1_REL = 0x0108,
		  OSC1_TUN = 0x0109,
		  OSC1_PAN = 0x010A,

		  //OSCILLATOR 2
		  OSC2_SIN = 0x0200,
		  OSC2_SAW = 0x0201,
		  OSC2_TRI = 0x0202,
		  OSC2_SQU = 0x0203,
		  OSC2_WNO = 0x0204,
		  OSC2_ATK = 0x0205,
		  OSC2_DEC = 0x0206,
		  OSC2_SUS = 0x0207,
		  OSC2_REL = 0x0208,
		  OSC2_TUN = 0x0209,
		  OSC2_PAN = 0x020A,

		  //CHANNEL
		  CHAN_VOL = 0x0300,

		   //LFO 1
		  LFO1_RAT = 0x0400,
		  LFO1_DEP = 0x0401,
		  LFO1_AMP = 0x0402,
		  LFO1_TAA = 0x0403,
		  LFO1_TAB = 0x0404,
		  LFO1_TAC = 0x0405,

		  //LFO 2
		  LFO2_RAT = 0x0500,
		  LFO2_DEP = 0x0501,
		  LFO2_AMP = 0x0502,
		  LFO2_TAA = 0x0503,
		  LFO2_TAB = 0x0504,
		  LFO2_TAC = 0x0505,

		  //3-pole eq
		  POL3_LOW = 0x0600,
		  POL3_MID = 0x0601,
		  POL3_HIG = 0x0602,

		  //Filter 1
		  FLT1_CUT = 0x0700,
		  FLT1_RES = 0x0701,
		  FLT1_MOD = 0x0702
		};

  	//Save the instrument bank to a file
  	extern bool saveInstruments(const char* filename);
  	//Load the instrument bank from file
  	extern bool loadInstruments(const char* filename);
  	//Get the value of an attribute
  	extern double get(uint8 channel, ValueType what);
 	 	//Set an attribute
  	extern bool set(uint8 channel, ValueType what, double value);
  	//Get the current state of the instruments in a char array
  	extern int8* getFullState(int32 &size);
  	//The size of the packed structure
  	extern int32 packedChanSize();
  #endif

};

#endif
