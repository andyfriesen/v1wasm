#include <algorithm>
#include <string>
#include <stdio.h>

#include "device_coreaudio.h"
#include "debug.h"

#include <CoreServices/CoreServices.h>

namespace audiere {

  CAAudioDevice*
  CAAudioDevice::create(const ParameterList& parameters) {
    OSStatus result = noErr;
    Component comp;
    ComponentDescription desc;
    AudioStreamBasicDescription requestedDesc;

    // Setup a AudioStreamBasicDescription with the requested format
    requestedDesc.mFormatID = kAudioFormatLinearPCM;
    requestedDesc.mFormatFlags = kLinearPCMFormatFlagIsPacked;
    requestedDesc.mChannelsPerFrame = 2;
    requestedDesc.mSampleRate = 44100;

    requestedDesc.mBitsPerChannel = 16;
    requestedDesc.mFormatFlags |= kLinearPCMFormatFlagIsSignedInteger;

    requestedDesc.mFramesPerPacket = 1;
    requestedDesc.mBytesPerFrame = requestedDesc.mBitsPerChannel * \
                                   requestedDesc.mChannelsPerFrame / 8;
    requestedDesc.mBytesPerPacket = requestedDesc.mBytesPerFrame * \
                                    requestedDesc.mFramesPerPacket;

    // Locate the default output audio unit
    desc.componentType = kAudioUnitType_Output;
    desc.componentSubType = kAudioUnitSubType_DefaultOutput;
    desc.componentManufacturer = kAudioUnitManufacturer_Apple;
    desc.componentFlags = 0;
    desc.componentFlagsMask = 0;

    comp = FindNextComponent(NULL, &desc);
    if (comp == NULL) {
      ADR_LOG ("Failed to start CoreAudio: FindNextComponent returned NULL");
      return 0;
    }

    // Open & initialize the default output audio unit
    ComponentInstance output_audio_unit;
    result = OpenAComponent(comp, &output_audio_unit);
    if (result != noErr) {
      ADR_LOG ("Failed to start CoreAudio: OpenAComponent failed.");
      CloseComponent(output_audio_unit);
      return 0;
    }

    result = AudioUnitInitialize(output_audio_unit);
    if (result != noErr) {
      ADR_LOG ("Failed to start CoreAudio: AudioUnitInitialize failed.");
      CloseComponent(output_audio_unit);
      return 0;
    }

    // Set the input format of the audio unit.
    result = AudioUnitSetProperty(output_audio_unit,
                                  kAudioUnitProperty_StreamFormat,
                                  kAudioUnitScope_Input,
                                  0,
                                  &requestedDesc,
                                  sizeof(requestedDesc));
    if (result != noErr) {
      ADR_LOG ("Failed to start CoreAudio: AudioUnitSetProperty failed.");
      CloseComponent(output_audio_unit);
      return 0;
    }
    return new CAAudioDevice(output_audio_unit);
  }


  CAAudioDevice::CAAudioDevice(ComponentInstance output_audio_unit)
    : MixerDevice(44100),
      m_output_audio_unit (output_audio_unit)
  {
    // Set the audio callback
    AURenderCallbackStruct callback;

    callback.inputProc = fillInput;
    callback.inputProcRefCon = this;
    AudioUnitSetProperty(m_output_audio_unit,
                         kAudioUnitProperty_SetRenderCallback,
                         kAudioUnitScope_Input,
                         0,
                         &callback,
                         sizeof(callback));

    AudioOutputUnitStart(m_output_audio_unit);
  }


  CAAudioDevice::~CAAudioDevice() {
    ADR_GUARD("CAAudioDevice::~CAAudioDevice");
    OSStatus result;
    AURenderCallbackStruct callback;

    // stop processing the audio unit
    result = AudioOutputUnitStop(m_output_audio_unit);

    // Remove the input callback
    callback.inputProc = 0;
    callback.inputProcRefCon = 0;
    result = AudioUnitSetProperty(m_output_audio_unit,
                                  kAudioUnitProperty_SetRenderCallback,
                                  kAudioUnitScope_Input,
                                  0,
                                  &callback,
                                  sizeof(callback));
    result = CloseComponent(m_output_audio_unit);
  }


  OSStatus CAAudioDevice::fillInput(void                         *inRefCon,
                                    AudioUnitRenderActionFlags   *inActionFlags,
                                    const AudioTimeStamp         *inTimeStamp,
                                    UInt32                       inBusNumber,
                                    UInt32                       inNumberFrames,
                                    AudioBufferList              *ioData) {
    CAAudioDevice* device = static_cast<CAAudioDevice*>(inRefCon);
    UInt32 remaining, len;
    void* ptr;
    AudioBuffer *curBuffer;

    for (unsigned int currentBuffer = 0; currentBuffer < ioData->mNumberBuffers; currentBuffer++) {
        curBuffer = &ioData->mBuffers[currentBuffer];
        remaining = curBuffer->mDataByteSize;
        ptr = curBuffer->mData;
        while (remaining > 0) {
          len = device->read(remaining / 4, ptr);
          ptr = (char *)ptr + len;
          remaining -= len * 4;
        }
    }

    return noErr;
  }

  void ADR_CALL
  CAAudioDevice::update() {
    AI_Sleep (20);
  }


  const char* ADR_CALL
  CAAudioDevice::getName() {
    return "coreaudio";
  }

}
