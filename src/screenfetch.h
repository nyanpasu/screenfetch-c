#ifndef SCREENFETCH_C_H
#define SCREENFETCH_C_H

#define _POSIX_C_SOURCE 200112L /* makes all these systems play nicely together */

/* TODO Use an enum */
#define UNKNOWN 0
#define CYGWIN 1
#define OSX 2
#define LINUX 3
#define FREEBSD 4
#define NETBSD 5
#define OPENBSD 6
#define DFBSD 7
#define SOLARIS 8

/* quick macro for when all BSDs have the same function syntax */
#define ISBSD() ((OS >= 4 && OS <= 7) ? true : false)

/* TODO Create platform/*.h for each platfor with appropriate
 * platform functions.
 */
#if defined(__CYGWIN__)
	#define OS CYGWIN
	extern FILE* popen(const char* command, const char* type);
	extern int pclose(FILE* stream);
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
#elif defined(__APPLE__) && defined(__MACH__)
	#define OS OSX
	#include <sys/utsname.h>
	#include <time.h>
#elif defined(__linux__)
	#define OS LINUX
	#include <sys/sysinfo.h>
	#include <sys/utsname.h>
	#include <X11/Xlib.h>
#elif defined(__FreeBSD__)
	#define OS FREEBSD
	#include <time.h>
#elif defined(__NetBSD__)
	#define OS NETBSD
#elif defined(__OpenBSD__)
	#define OS OPENBSD
	#include <sys/utsname.h>
	#include <time.h>
#elif defined(__DragonFly__)
	#define OS DFBSD
	#include <time.h>
#elif defined(__sun__)
	#define OS SOLARIS
	#include <utmpx.h>
	#include <time.h>
	#include <sys/types.h>
	#include <sys/param.h>
	#include <sys/utsname.h>
	#include <X11/Xlib.h>
#else 
	#define OS UNKNOWN
#endif


/* color/fmt definitions */
#define TNRM "\x1B[0m" /* normal */
#define TBLK "\x1B[30m" /* black */
#define TRED "\x1B[31m" /* red */
#define TGRN "\x1B[32m" /* green */
#define TBRN "\x1B[33m" /* brown */
#define TBLU "\x1B[34m" /* blue */
#define TPUR "\x1B[35m" /* purple */
#define TCYN "\x1B[36m" /* cyan */
#define TLGY "\x1B[37m" /* light gray */
#define TDGY "\x1B[1;30m" /* dark gray */
#define TLRD "\x1B[1;31m" /* light red */
#define TLGN "\x1B[1;32m" /* light green */
#define TYLW "\x1B[1;33m" /* yellow */
#define TLBL "\x1B[1;34m" /* light blue */
#define TLPR "\x1B[1;35m" /* light purple */
#define TLCY "\x1B[1;36m" /* light cyan */
#define TWHT "\x1B[1;37m" /* white */

/* other definitions, use with caution (not type safe) */
#define KB 1024
#define MB KB*KB
#define MAX_STRLEN 128

#define SET_DISTRO(str) (safe_strncpy(distro_str, str, MAX_STRLEN))
#define STRCMP(x, y) (!strcmp(x, y))
#define FILE_EXISTS(file) (!access(file, F_OK))
#define ERROR_OUT(str1, str2) (fprintf(stderr, TWHT "[[ " TLRD "!" TWHT " ]] " TNRM "%s%s\n", str1, str2))
#define VERBOSE_OUT(str1, str2) (fprintf(stdout, TLRD ":: " TNRM "%s%s\n", str1, str2))

struct char_pair_t {
        const char *title;
        const char *value;
};

struct screenfetch_t {
        struct char_pair_t distro;
        struct char_pair_t arch;
        struct char_pair_t host;
        struct char_pair_t kernel;
        struct char_pair_t uptime;
        struct char_pair_t pkgs;
        struct char_pair_t cpu;
        struct char_pair_t gpu;
        struct char_pair_t disk;
        struct char_pair_t mem;
        struct char_pair_t shell;
        struct char_pair_t res;
        struct char_pair_t de;
        struct char_pair_t wm;
        struct char_pair_t wm_theme;
        struct char_pair_t gtk;
};

/* other function definitions */
/* TODO Move 'other function definitions' to utils.c */
int manual_input(void);
void output_logo_only(char* distro);
void main_ascii_output(char* data[], char* data_names[]);
void main_text_output(char* data[], char* data_names[]);
char* safe_strncpy(char* destination, const char* source, size_t num); 
void split_uptime(long uptime, int* secs, int* mins, int* hrs, int* days);
void display_version(void);
void display_help(void);
void take_screenshot(void);

#endif /* SCREENFETCH_C_H */
