// *sigh*, looking forward to VS.NET...
#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif


#include <string>
#include "audiere.h"
#include "debug.h"
#include "device_null.h"
#include "internal.h"
#include "threads.h"

#ifdef _WIN32
  #include <windows.h>
#endif

#ifdef HAVE_WINMM
  #include <mmsystem.h>
  #include "device_mm.h"
#endif

#ifdef HAVE_DSOUND
  #include "device_ds.h"
#endif

#ifdef HAVE_ALSA
  #include "device_alsa.h"
#endif

#ifdef HAVE_OSS
  #include "device_oss.h"
#endif

#ifdef HAVE_PULSE
  #include "device_pulse.h"
#endif

#ifdef HAVE_AL
  #include "device_al.h"
#endif

#ifdef HAVE_SDL
  #include "device_sdl.h"
#endif

#ifdef HAVE_DSOUND
  #include "device_ds.h"
#endif

#ifdef HAVE_WINMM
  #include "device_mm.h"
#endif

#ifdef HAVE_PA
  #include "device_pa.h"
#endif

#ifdef HAVE_CORE_AUDIO
  #include "device_coreaudio.h"
#endif

namespace audiere {

  AbstractDevice::AbstractDevice() {
    m_thread_exists = false;
    m_thread_should_die = false;

    bool result = AI_CreateThread(eventThread, this, 2);
    if (!result) {
      ADR_LOG("THREAD CREATION FAILED");
    }
  }

  AbstractDevice::~AbstractDevice() {
    m_thread_should_die = true;

    // Trick the thread into no longer waiting.
    m_events_available.notify();

    while (m_thread_exists) {
      AI_Sleep(50);
    }
  }

  void AbstractDevice::registerCallback(Callback* callback) {
    m_callbacks.push_back(callback);
  }

  void AbstractDevice::unregisterCallback(Callback* callback) {
    for (size_t i = 0; i < m_callbacks.size(); ++i) {
      if (m_callbacks[i] == callback) {
        m_callbacks.erase(m_callbacks.begin() + i);
        return;
      }
    }
  }

  void AbstractDevice::clearCallbacks() {
    m_callbacks.clear();
  }

  void AbstractDevice::fireStopEvent(OutputStreamPtr stream, StopEvent::Reason reason) {
    StopEventPtr event = new StopEventImpl(stream, reason);
    fireStopEvent(event);
  }

  void AbstractDevice::fireStopEvent(const StopEventPtr& event) {
    m_event_mutex.lock();
    m_events.push(event.get());
    m_event_mutex.unlock();
    m_events_available.notify();
  }

  void AbstractDevice::eventThread(void* arg) {
    ADR_GUARD("AbstractDevice::eventThread[static]");
    ADR_LOG(arg ? "arg is valid" : "arg is not valid");
    
    AbstractDevice* This = static_cast<AbstractDevice*>(arg);
    This->eventThread();
  }

  void AbstractDevice::eventThread() {
    ADR_GUARD("AbstractDevice::eventThread");
    m_thread_exists = true;
    while (!m_thread_should_die) {
      m_event_mutex.lock();
      while (m_events.empty()) {
        m_events_available.wait(m_event_mutex, 1);
        if (m_thread_should_die) {
          break;
        }
      }
      if (m_thread_should_die) {
        m_event_mutex.unlock();
        break;
      }

      // Make a local copy of the events so they can be processed without
      // leaving the mutex locked.
      EventQueue events = m_events;

      // Queues don't support clear().  o_o
      while (!m_events.empty()) {
        m_events.pop();
      }

      m_event_mutex.unlock();

      // Process the events.
      while (!events.empty()) {
        EventPtr event = events.front();
        events.pop();
        processEvent(event.get());
      }
    }
    m_thread_exists = false;
  }

  void AbstractDevice::processEvent(Event* event) {
    for (size_t i = 0; i < m_callbacks.size(); ++i) {
      if (event->getType() == m_callbacks[i]->getType()) {
        m_callbacks[i]->call(event);
      }
    }
  }


  ADR_EXPORT(const char*) AdrGetSupportedAudioDevices() {
    return
#ifdef HAVE_ALSA
      "alsa:Advanced Linux Sound Architecture"  ";"
#endif
#ifdef HAVE_SDL
      "sdl:Simple Direct Media sound device" ";"
#endif
#ifdef HAVE_OSS
      "oss:Open Sound System"  ";"
#endif
#ifdef HAVE_PULSE
      "pulse: PulseAudio"  ";"
#endif
#ifdef HAVE_DSOUND
      "directsound:DirectSound (high-performance)"  ";"
#endif
#ifdef HAVE_WINMM
      "winmm:Windows Multimedia (compatible)"  ";"
#endif
#ifdef HAVE_AL
      "al:SGI AL"  ";"
#endif
#ifdef HAVE_PA
      "pa:portaudio compatible"  ";"
#endif
#ifdef HAVE_CORE_AUDIO
      "coreaudio:Core Audio (Mac OS X)"  ";"
#endif
      "null:Null output (no sound)"  ;
  }

  #define TRY_RECURSE(NAME) do {                                \
    AudioDevice* device = DoOpenDevice(NAME, parameters);       \
    if (device) {                                               \
      return device;                                            \
    }                                                           \
  } while (0)

  #define MAKE_DEVICE(DeviceType) (DeviceType::create(parameters))


  AudioDevice* DoOpenDevice(
    const std::string& name,
    const ParameterList& parameters)
  {
    ADR_GUARD("DoOpenDevice");

    if (name == "" || name == "autodetect") {
      // in decreasing order of sound API quality
      TRY_RECURSE("alsa");
      TRY_RECURSE("al");
      TRY_RECURSE("directsound");
      TRY_RECURSE("winmm");
      TRY_RECURSE("sdl");
      TRY_RECURSE("pulse");
      TRY_RECURSE("oss");
      TRY_RECURSE("portaudio");
      TRY_RECURSE("coreaudio");
      return 0;
    }

    #ifdef HAVE_ALSA
      if (name == "alsa") {
        return MAKE_DEVICE(ALSAAudioDevice);
      }
    #endif

    #ifdef HAVE_OSS
      if (name == "oss") {
        return MAKE_DEVICE(OSSAudioDevice);
      }
    #endif

    #ifdef HAVE_SDL
      if (name == "sdl") {
        return MAKE_DEVICE(SDLAudioDevice);
      }
    #endif

    #ifdef HAVE_PULSE
      if (name == "pulse") {
        return MAKE_DEVICE(PulseAudioDevice);
      }
    #endif

    #ifdef HAVE_DSOUND
      if (name == "directsound") {
        return MAKE_DEVICE(DSAudioDevice);
      }
    #endif

    #ifdef HAVE_WINMM
      if (name == "winmm") {
        return MAKE_DEVICE(MMAudioDevice);
      }
    #endif

    #ifdef HAVE_AL
      if (name == "al") {
        return MAKE_DEVICE(ALAudioDevice);
      }
    #endif

    #ifdef HAVE_PA
      if (name == "portaudio") {
        return MAKE_DEVICE(PAAudioDevice);
      }
    #endif

    #ifdef HAVE_CORE_AUDIO
      if (name == "coreaudio") {
        return MAKE_DEVICE(CAAudioDevice);
      }
    #endif

    if (name == "null") {
      return MAKE_DEVICE(NullAudioDevice);
    }

    // no devices
    return 0;
  }


  class ThreadedDevice : public RefImplementation<AudioDevice> {
  public:
    ThreadedDevice(AudioDevice* device) {
      ADR_GUARD("ThreadedDevice::ThreadedDevice");
      if (device) {
        ADR_LOG("Device is valid");
      } else {
        ADR_LOG("Device is not valid");
      }

      m_device = device;
      m_thread_exists = false;
      m_thread_should_die = false;

      /// @todo  what if thread creation fails?
      bool result = AI_CreateThread(threadRoutine, this, 2);
      if (!result) {
        ADR_LOG("THREAD CREATION FAILED");
      }
    }

    ~ThreadedDevice() {
      m_thread_should_die = true;
      while (m_thread_exists) {
        AI_Sleep(50);
      }
    }

    // don't need to update the device...  the thread does it for us
    void ADR_CALL update() {
    }

    OutputStream* ADR_CALL openStream(SampleSource* source) {
      return m_device->openStream(source);
    }

    OutputStream* ADR_CALL openBuffer(
      void* samples, int frame_count,
      int channel_count, int sample_rate, SampleFormat sample_format)
    {
      return m_device->openBuffer(
        samples, frame_count,
        channel_count, sample_rate, sample_format);
    }

    const char* ADR_CALL getName() {
      return m_device->getName();
    }

    void ADR_CALL registerCallback(Callback* callback) {
      m_device->registerCallback(callback);
    }

    void ADR_CALL unregisterCallback(Callback* callback) {
      m_device->unregisterCallback(callback);
    }

    void ADR_CALL clearCallbacks() {
      m_device->clearCallbacks();
    }

  private:
    void run() {
      ADR_GUARD("ThreadedDevice::run");
      m_thread_exists = true;
      while (!m_thread_should_die) {
        m_device->update();
      }
      m_thread_exists = false;
    }

    static void threadRoutine(void* arg) {
      ADR_GUARD("ThreadedDevice::threadRoutine");
      if (arg) {
        ADR_LOG("arg is valid");
      } else {
        ADR_LOG("arg is not valid");
      }

      ThreadedDevice* This = static_cast<ThreadedDevice*>(arg);
      This->run();
    }

  private:
    RefPtr<AudioDevice> m_device;
    volatile bool m_thread_should_die;
    volatile bool m_thread_exists;
  };


  ADR_EXPORT(AudioDevice*) AdrOpenDevice(
    const char* name,
    const char* parameters)
  {
    ADR_GUARD("AdrOpenDevice");

    if (!name) {
      name = "";
    }
    if (!parameters) {
      parameters = "";
    }

    // first, we need an unthreaded audio device
    AudioDevice* device = DoOpenDevice(
      std::string(name),
      ParameterList(parameters));
    if (!device) {
      ADR_LOG("Could not open device");
      return 0;
    }

    ADR_LOG("creating threaded device");
    return new ThreadedDevice(device);
  }

}
