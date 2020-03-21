#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#include <ctype.h>
#include <cstdio>
#include "utility.h"
#include "internal.h"


namespace audiere {

  ParameterList::ParameterList(const char* parameters) {
    std::string key;
    std::string value;

    std::string* current_string = &key;

    // walk the string and generate the parameter list
    const char* p = parameters;
    while (*p) {

      if (*p == '=') {

        current_string = &value;

      } else if (*p == ',') {

        if (key.length() && value.length()) {
          m_values[key] = value;
        }
        key   = "";
        value = "";
        current_string = &key;

      } else {
        *current_string += *p;
      }

      ++p;
    }

    // is there one more parameter without a trailing comma?
    if (key.length() && value.length()) {
      m_values[key] = value;
    }
  }

  std::string
  ParameterList::getValue(
    const std::string& key,
    const std::string& defaultValue) const
  {
    std::map<std::string, std::string>::const_iterator i = m_values.find(key);
    return (i == m_values.end() ? defaultValue : i->second);
  }

  bool
  ParameterList::getBoolean(const std::string& key, bool def) const {
    std::string value = getValue(key, (def ? "true" : "false"));
    return (value == "true" || atoi(value.c_str()) != 0);
  }

  int
  ParameterList::getInt(const std::string& key, int def) const {
    char str[20];
    sprintf(str, "%d", def);
    return atoi(getValue(key, str).c_str());
  }


  int strcmp_case(const char* a, const char* b) {
    while (*a && *b) {

      char c = tolower(*a++);
      char d = tolower(*b++);

      if (c != d) {
        return c - d;
      }
    }

    char c = tolower(*a);
    char d = tolower(*b);
    return (c - d);
  }
#if defined(__GNUC__) && ! defined(_WIN32) && !defined(__EMSCRIPTEN__)
#  include <features.h>
#  if __GNUC_PREREQ(4,1)
#    define ADR_HAVE_GCC_ATOMIC_INTRINSICS
#  endif
#endif

#if ( defined(_WIN32) || defined(_WIN64) ) && !defined(__CYGWIN__)
#  define ADR_USE_WIN32_INTERLOCKED
#elif defined(ADR_HAVE_GCC_ATOMIC_INTRINSICS)
#  define ADR_USE_GCC_ATOMIC_INTRINSICS
#endif


#if defined(ADR_USE_WIN32_INTERLOCKED)

  ADR_EXPORT(long) AdrAtomicIncrement(volatile long& var) {
    return InterlockedIncrement(&var);

  }

  ADR_EXPORT(long) AdrAtomicDecrement(volatile long& var) {
    return InterlockedDecrement(&var);
  }

#elif defined(ADR_USE_GCC_ATOMIC_INTRINSICS)

  ADR_EXPORT(long) AdrAtomicIncrement(volatile long& var) {
    return __sync_add_and_fetch(&var, 1);
  }

  ADR_EXPORT(long) AdrAtomicDecrement(volatile long& var) {
    return __sync_sub_and_fetch(&var, 1);
  }

#else
  /// @todo Warn that this isn't really atomic!
  ADR_EXPORT(long) AdrAtomicIncrement(volatile long& var) {
    return ++var;
  }

  ADR_EXPORT(long) AdrAtomicDecrement(volatile long& var) {
    return --var;
  }

#endif

  ADR_EXPORT(int) AdrGetSampleSize(SampleFormat format) {
    switch (format) {
      case SF_U8:  return 1;
      case SF_S16: return 2;
      default:     return 0;
    }
  }

}
