#ifndef __PLATFORM_DEF_HPP__
#define __PLATFORM_DEF_HPP__

#ifdef __linux__
#define __LINUX__ 1
#endif

#ifdef WIN32
#define __WINDOWS__ 2
#endif

#ifdef __CYGWIN__
#define __WINDOWS__ 2
#endif

#endif
