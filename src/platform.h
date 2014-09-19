#ifndef SCREENFETCH_PLATFORM_H
#define SCREENFETCH_PLATFORM_H

#if defined(__CYGWIN__)
	#define PLATFORM_WINDOWS
	extern FILE* popen(const char* command, const char* type);
	extern int pclose(FILE* stream);
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
#elif defined(__APPLE__) && defined(__MACH__)
	#define PLATFORM_OSX
	#include <sys/utsname.h>
	#include <time.h>
#elif defined(__linux__)
	#define PLATFORM_LINUX
	#include <sys/sysinfo.h>
	#include <sys/utsname.h>
	#include <X11/Xlib.h>
#elif defined(__sun__)
	#define PLATOFRM_SOLARIS
	#include <utmpx.h>
	#include <time.h>
	#include <sys/types.h>
	#include <sys/param.h>
	#include <sys/utsname.h>
	#include <X11/Xlib.h>
#elif defined(__DragonFly__)
	#define PLATFORM_BSD
	#define BSD_DRAGONFLY
	#include <time.h>
#elif defined(__FreeBSD__)
	#define PLATFORM_BSD
	#define BSD_FREE
	#include <time.h>
#elif defined(__NetBSD__)
	#define PLATFORM_BSD
	#define BSD_NET
#elif defined(__OpenBSD__)
	#define PLATFORM_BSD
	#define BSD_OPEN
	#include <sys/utsname.h>
	#include <time.h>
#else 
	#define PLATFORM_UNKNOWN
#endif

#endif // SCREENFETCH_PLATFORM_H
