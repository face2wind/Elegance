#pragma once

#ifdef __linux__
#define __LINUX__
#endif

#ifdef WIN32
#define __WINDOWS__
#endif

#ifdef __CYGWIN__
#define __WINDOWS__
//#define __LINUX__
#endif

