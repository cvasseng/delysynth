//
//  main.cpp
//  delysynth
//
//  Created by Chris M. Vasseng on 6/21/14.
//  Copyright (c) 2014 Chris Vasseng. All rights reserved.
//

#include <stdlib.h>
#include <math.h>
#include <iostream>

#include <AudioToolbox/AudioQueue.h>
#include <CoreAudio/CoreAudioTypes.h>
#include <CoreFoundation/CFRunLoop.h>
#include <CoreMIDI/CoreMIDI.h>

#include "del.synth.h"
#include "del.types.h"

#define SAMPLE_RATE 44100
#define NUM_BUFFERS 3
#define NUM_CHANNELS 2
#define SAMPLE_TYPE short

void handleMIDIPacket(const MIDIPacket* packet) {
  if (packet->length > 0) {
    if (packet->data[0] >= 0x80 && packet->data[0] < 0x90) {
      ds::noteOff(packet->data[0] - 0x80, packet->data[1]);
    } else if (packet->data[0] >= 0x90 && packet->data[0] < 0x99 && packet->length > 1) {
      ds::noteOn(packet->data[0] - 0x90, packet->data[1], packet->data[2]);
    }
  }
}

void midiInputCallback(const MIDIPacketList *packetList, void* readProcRefCon,
                       void* srcConnRefCon) {
  MIDIPacket *packet = (MIDIPacket*)packetList->packet;
  int j;
  int count = packetList->numPackets;
  for (j=0; j<count; j++) {
    handleMIDIPacket(packet);
    
    if (packet->data[0] == 0xb0 && packet->data[1] == 21) {
        
        ds::ActiveChannels[0].osc[1].tune = (((double)packet->data[2] / 127.0) * 32.0) - 16;
    }
      
    packet = MIDIPacketNext(packet);
  }
}

void callback(void *custom_data, AudioQueueRef queue, AudioQueueBufferRef buffer);

void midiSetup() {
  MIDIClientRef   midiClient;
  MIDIPortRef     inputPort;
  OSStatus        status;
  
  status = MIDIClientCreate(CFSTR("MIDI client"), NULL, NULL, &midiClient);
  
  if (status != noErr) {
    return;
  }
  
  status = MIDIInputPortCreate(midiClient, CFSTR("MIDI input"), midiInputCallback, NULL, &inputPort);
  
  ItemCount numOfDevices = MIDIGetNumberOfDevices();
  
  for (ItemCount i = 0; i < numOfDevices; i++) {
    MIDIEndpointRef src = MIDIGetSource(i);
    MIDIPortConnectSource(inputPort, src, NULL);
  }
}

int main(int argc, const char * argv[])
{
  
 // ds::noteOn(0, 60, 100);
  
  //Test if the set system works or if it crashes everything..
  ds::set(0, ds::OSC1_SIN, 1.0);
  ds::set(0, ds::OSC1_SAW, 0.3);
  ds::set(0, ds::OSC2_SIN, 1.0);
  //ds::set(0, ds::OSC2_TUN, 1.0);
  // ds::set(0, ds::OSC1_ATK, 1.0);
  // ds::set(0, ds::OSC1_REL, 1.0);
    //ds::ActiveChannels[0].osc[1].pan = 0.5;
    //ds::ActiveChannels[0].osc[0].pan = 0.5;
 
  ds::precalc_envelope(ds::ActiveChannels[0].osc[0]);
  
 // ds::set(0, ds::OSC2_SIN, 0.5);
  
  //Do a test write
  //ds::saveInstruments("delysynth.bank");
  //ds::loadInstruments("delysynth.bank");
  
  //Test modulation
  //ds::ActiveChannels[0].lfo1.targetA = ds::MOD_OSC1_SQU;
    //ds::ActiveChannels[0].lfo1.targetB = ds::MOD_OSC2_FRE;
  
  midiSetup();

  AudioStreamBasicDescription format;
  AudioQueueRef queue;
  AudioQueueBufferRef buffers[NUM_BUFFERS];
  
  format.mSampleRate       = SAMPLE_RATE;
  format.mFormatID         = kAudioFormatLinearPCM;
  format.mFormatFlags      = kLinearPCMFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
  format.mBitsPerChannel   = 8 * sizeof(SAMPLE_TYPE);
  format.mChannelsPerFrame = NUM_CHANNELS;
  format.mBytesPerFrame    = sizeof(SAMPLE_TYPE) * NUM_CHANNELS;
  format.mFramesPerPacket  = 1;
  format.mBytesPerPacket   = format.mBytesPerFrame * format.mFramesPerPacket;
  format.mReserved         = 0;
  
  AudioQueueNewOutput(&format, callback, NULL, CFRunLoopGetCurrent(), kCFRunLoopCommonModes, 0, &queue);
  
  for (int i = 0; i < NUM_BUFFERS; i++) {
    AudioQueueAllocateBuffer(queue, BUFFER_SIZE, &buffers[i]);
    
    buffers[i]->mAudioDataByteSize = BUFFER_SIZE;
    
    callback(NULL, queue, buffers[i]);
  }
	
  AudioQueueStart(queue, NULL);
  
  CFRunLoopRun();
  
  return 0;
}

void callback(void *custom_data, AudioQueueRef queue, AudioQueueBufferRef buffer) {
  SAMPLE_TYPE *casted_buffer = (SAMPLE_TYPE *)buffer->mAudioData;
  
  if (!ds::fillBuffer(casted_buffer)) {
    AudioQueueStop(queue, false);
    AudioQueueDispose(queue, false);
    CFRunLoopStop(CFRunLoopGetCurrent());
  }

  AudioQueueEnqueueBuffer(queue, buffer, 0, NULL);
  
}