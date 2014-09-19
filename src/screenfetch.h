#ifndef SCREENFETCH_C_H
#define SCREENFETCH_C_H

#define _POSIX_C_SOURCE 200112L /* makes all these systems play nicely together */

enum distro_t
{
	UNKNOWN,
	CYGWIN,
	OSX,
	LINUX,
	FREEBSD,
	NETBSD,
	OPENBSD,
	DFBSD,
	SOLARIS
};

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
void sf_display_version(void);
void sf_display_help(void);
void sf_take_screenshot(void);

#endif /* SCREENFETCH_C_H */
