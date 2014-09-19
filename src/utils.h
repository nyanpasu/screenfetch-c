#ifndef SCREENFETCH_UTILS_H
#define SCREENFETCH_UTILS_H

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

#endif
