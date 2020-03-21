#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include "threads.h"
#include "utility.h"


namespace audiere {

  bool AI_CreateThread(AI_ThreadRoutine routine, void* opaque, int priority) {
    return true;
  }


  void AI_Sleep(unsigned milliseconds) {
  }


  struct Mutex::Impl {
  };

  Mutex::Mutex() {
    m_impl = new Impl;
  }

  Mutex::~Mutex() {
    delete m_impl;
  }

  void Mutex::lock() {
  }

  void Mutex::unlock() {
  }


  struct CondVar::Impl {
  };

  CondVar::CondVar() {
    m_impl = new Impl;
  }

  CondVar::~CondVar() {
    delete m_impl;
  }

  void CondVar::wait(Mutex& mutex, float seconds) {
  }

  void CondVar::notify() {
  }

}
