#pragma once

#ifdef __linux__
#define __LINUX__
#endif

#ifdef WIN32 // vs2005
#define __WINDOWS__
#endif
#ifdef _WIN32  // vs2015
#define __WINDOWS__
#endif

#ifdef __CYGWIN__
#define __WINDOWS__
//#define __LINUX__
#endif

