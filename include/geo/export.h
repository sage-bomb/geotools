#ifndef GEO_EXPORT_H
#define GEO_EXPORT_H

#if defined(_WIN32) || defined(__CYGWIN__)
  #if defined(GEO_BUILD_DLL)
    #define GEO_API __declspec(dllexport)
  #else
    #define GEO_API __declspec(dllimport)
  #endif
#else
  #define GEO_API
#endif

#endif
