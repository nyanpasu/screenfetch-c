#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "screenfetch.h"
#include "detect.h"
#include "thread.h"
#include "version.h"

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

/* other definitions */
static bool manual = false;
static bool logo = true;
static bool error = true;
static bool verbose = false;
static bool screenshot = false;

static const char* screenfetch_unknown = "Unknown";

int main(int argc, char** argv)
{
        struct screenfetch_t sf = {
                .distro   = {.title = "OS: "         , .value = screenfetch_unknown},
                .arch     = {.title = "Arch: "       , .value = screenfetch_unknown},
                .host     = {.title = "Arch: "       , .value = screenfetch_unknown},
                .kernel   = {.title = "Kernel: "     , .value = screenfetch_unknown},
                .uptime   = {.title = "Uptime: "     , .value = screenfetch_unknown},
                .pkgs     = {.title = "Packages: "   , .value = screenfetch_unknown},
                .cpu      = {.title = "CPU: "        , .value = screenfetch_unknown},
                .gpu      = {.title = "GPU: "        , .value = screenfetch_unknown},
                .disk     = {.title = "Disk: "       , .value = screenfetch_unknown},
                .mem      = {.title = "Memory: "     , .value = screenfetch_unknown},
                .shell    = {.title = "Shell: "      , .value = screenfetch_unknown},
                .res      = {.title = "Resolution: " , .value = screenfetch_unknown},
                .de       = {.title = "DE: "         , .value = screenfetch_unknown},
                .wm       = {.title = "WM: "         , .value = screenfetch_unknown},
                .wm_theme = {.title = "WM Theme: "   , .value = screenfetch_unknown},
                .gtk      = {.title = "GTK: "        , .value = screenfetch_unknown}
        };

        struct option sf_options[] =
        {
                {"manual"          , no_argument      , 0, 'm'},
                {"verbose"         , no_argument      , 0, 'v'},
                {"no-logo"         , no_argument      , 0, 'n'},
                {"screenshot"      , no_argument      , 0, 's'},
                {"distro"          , required_argument, 0, 'D'},
                {"suppress-errors" , no_argument      , 0, 'E'},
                {"version"         , no_argument      , 0, 'V'},
                {"help"            , no_argument      , 0, 'h'},
                {"logo-only"       , required_argument, 0, 'L'},
                { },
                NULL
        };

        /* TOOO Move these characters to the scope of the loop. */
        signed char c;
        int index = 0;
        while ((c = getopt_long(argc, argv, "mvnsD:EVhL:", sf_options, &index)) != -1)
        {
                switch (c)
                {
                        case 'm':
                                manual = true;
                                break;
                        case 'v':
                                verbose = true;
                                break;
                        case 'n':
                                logo = false;
                                break;
                        case 's':
                                screenshot = true;
                                break;
                        case 'D':
                                SET_DISTRO(optarg);
                                break;
                        case 'E':
                                error = false;
                                break;
                        case 'V':
                                display_version();
                                return EXIT_SUCCESS;
                        case 'h':
                                display_help(sf_options);
                                return EXIT_SUCCESS;
                        case 'L':
                                output_logo_only(optarg);
                                return EXIT_SUCCESS;
                        case '?':
                                if (optopt == 'D')
                                        ERROR_OUT("Error: ", "The -D (--distro) flag requires an argument.");
                                else if (optopt == 'L')
                                        ERROR_OUT("Error: ", "The -L (--logo-only) flag requires an argument.");
                                else
                                        ERROR_OUT("Error: ", "Unknown option or option character.");
                                return EXIT_FAILURE;
                }
        }

        /* TODO Rewrite how manual mode is handled. */
        if (manual) /* if the user has decided to enter manual mode */
        {
                int stat = manual_input();

                if (stat == EXIT_SUCCESS)
                {
                        /* these sections are ALWAYS detected */
                        detect_uptime (sf->uptime);
                        detect_pkgs   (sf->pkgs);
                        detect_disk   (sf->disk);
                        detect_mem    (sf->mem);

                        /* if the user specifies an asterisk, fill the data in for them */
                        // FIXME Just get rid of this altogether. Using a macro for now.
#define AUTO_DETECT(detector, target) if (STRCMP((target), "*")) detector((target))
                        AUTO_DETECT(detect_distro   , sf->distro);
                        AUTO_DETECT(detect_arch     , sf->arch);
                        AUTO_DETECT(detect_host     , sf->host);
                        AUTO_DETECT(detect_kernel   , sf->kernel);
                        AUTO_DETECT(detect_cpu      , sf->cpu);
                        AUTO_DETECT(detect_gpu      , sf->gpu);
                        AUTO_DETECT(detect_shell    , sf->shell);
                        AUTO_DETECT(detect_res      , sf->res);
                        AUTO_DETECT(detect_de       , sf->de);
                        AUTO_DETECT(detect_wm       , sf->wm);
                        AUTO_DETECT(detect_wm_theme , sf->wm_theme);
                        AUTO_DETECT(detect_gtk      , sf->gtk);
                }
                else /* if the user decided to leave manual mode without input */
                        return EXIT_SUCCESS;
        }

        else
        {
                detect_distro   (sf->distro);
                detect_arch     (sf->arch);
                detect_host     (sf->host);
                detect_kernel   (sf->kernel);
                detect_uptime   (sf->uptime);
                detect_pkgs     (sf->pkgs);
                detect_cpu      (sf->cpu);
                detect_gpu      (sf->gpu);
                detect_disk     (sf->disk);
                detect_mem      (sf->mem);
                detect_shell    (sf->shell);
                detect_res      (sf->res);
                detect_de       (sf->de);
                detect_wm       (sf->wm);
                detect_wm_theme (sf->wm_theme);
                detect_gtk      (sf->gtk);
        }

        if (logo)
                main_ascii_output(detected_arr, detected_arr_names);
        else
                main_text_output(detected_arr, detected_arr_names);

        if (screenshot)
                take_screenshot();

        return EXIT_SUCCESS;
}

/*  main_text_output
    the secondary output for screenfetch-c - all info WITHOUT ASCII art is printed here
    arguments char* data[], char* data_names[]: string arrays containing the names and data acquired
    */
void main_text_output(char* data[], char* data_names[])
{
        int i;

        for (i = 0; i < 16; i++)
                printf("%s %s\n", data_names[i], data[i]);

        return;
}

void sf_display_version(void)
{
        printf("%Screenfetch %d.%d.%d\n", SCREENFETCH_VERSION_MAJOR, SCREENFETCH_VERSION_MINOR, SCREENFETCH_VERSION_REVISION);

        printf("%s\n", "Operating Systems currently supported:");
        printf("%s\n", "Windows (via Cygwin), Linux, *BSD, OS X, and Solaris.");
        printf("%s\n", "Using screenfetch on an OS not listed above may not work entirely or at all (and is disabled by default).");
        printf("%s\n", "Please access 'man screenfetch' for in-depth information on compatibility and usage.");

        return;
}

void sf_display_help(struct options *o)
{
        printf("%s\n", "Usage: screenfetch [OPTION]...");
        printf("%s\n\n", "Displays formatted system information.");
        
        for(struct option *i = o; i != NULL; i++)
                printf("  -%c, --%s\n", o->val, o->name);

        return;
}

/*  take_screenshot
    takes a screenshot and saves it to $HOME/screenfetch_screenshot.jpg
    --
CAVEAT: THIS FUNCTION MAKES SYSTEM CALLS
--
*/
void sf_take_screenshot(void)
{
        printf("%s", "Taking shot in 3..");
        fflush(stdout);
        sleep(1);
        printf("%s", "2..");
        fflush(stdout);
        sleep(1);
        printf("%s", "1..");
        fflush(stdout);
        sleep(1);
        printf("%s\n", "0");

        if (OS == CYGWIN)
        {
#ifdef __CYGWIN__
                /* terrible hack, the printscreen key is simulated */
                keybd_event(VK_SNAPSHOT, 0, 0, 0);

                if (verbose)
                        VERBOSE_OUT("Screenshot has been saved to the clipboard.", "");


                /* NOT FINSISHED - HBITMAP needs to be saved
                   HDC screen_dc = GetDC(NULL);
                   HDC mem_dc = CreateCompatibleDC(screen_dc);

                   int horiz = GetDeviceCaps(screen_dc, HORZRES);
                   int vert = GetDeviceCaps(screen_dc, VERTRES);

                   HBITMAP bitmap = CreateCompatibleBitmap(screen_dc, horiz, vert);
                   HBITMAP old_bitmap = SelectObject(mem_dc, bitmap);

                   BitBlt(mem_dc, 0, 0, horiz, vert, screen_dc, 0, 0, SRCCOPY);
                   bitmap = SelectObject(mem_dc, old_bitmap);

                   DeleteDC(screen_dc);
                   DeleteDC(mem_dc);
                   */
#endif
        }

        else
        {
                if (OS == OSX)
                {
                        system("screencapture -x ~/screenfetch_screenshot.png 2> /dev/null");	
                }

                else if (OS == LINUX || ISBSD())
                {
                        system("scrot ~/screenfetch_screenshot.png 2> /dev/null");
                }

                char* loc = getenv("HOME");
                strncat(loc, "/screenfetch_screenshot.png", MAX_STRLEN);

                if (FILE_EXISTS(loc) && verbose)
                {
                        VERBOSE_OUT("Screenshot successfully saved.", "");
                }

                else if (verbose)

                        ERROR_OUT("Error: ", "Problem saving screenshot.");
                }
        }

        return;
}

/*  manual_input
    generates (or reads) the ~/.screenfetchc file based upon user input
    returns an int indicating status (SUCCESS or FAILURE)
    */
int manual_input(void)
{
        FILE* config_file;
        char* config_file_loc;

        config_file_loc = getenv("HOME");
        strncat(config_file_loc, "/.screenfetchc", MAX_STRLEN);

        if (!FILE_EXISTS(config_file_loc))
        {
                if (OS == CYGWIN)
                {
                        printf("%s\n", TBLU "WARNING: There is currenly a bug involving manual mode on Windows." TNRM);
                        printf("%s\n", TBLU "Only continue if you are ABSOLUTELY sure." TNRM);
                }

                printf("%s\n", "This appears to be your first time running screenfetch-c in manual mode.");
                printf("%s", "Would you like to continue? (y/n) ");

                char choice = getchar();
                getchar(); /* flush the newline */

                if (choice == 'y' || choice == 'Y')
                {
                        config_file = fopen(config_file_loc, "w");

                        printf("%s\n", "We are now going to begin the manual mode input process.");
                        printf("%s\n", "Please enter exactly what is asked for.");
                        printf("%s\n", "If you are unsure about format, please consult the manpage.");

                        printf("%s", "Please enter the name of your distribution/OS: ");
                        fgets(distro_str, MAX_STRLEN, stdin);
                        fputs(distro_str, config_file);

                        printf("%s", "Please enter your architecture: ");
                        fgets(arch_str, MAX_STRLEN, stdin);
                        fputs(arch_str, config_file);

                        printf("%s", "Please enter your username@hostname: ");
                        fgets(host_str, MAX_STRLEN, stdin);
                        fputs(host_str, config_file);

                        printf("%s", "Please enter your kernel: ");
                        fgets(kernel_str, MAX_STRLEN, stdin);
                        fputs(kernel_str, config_file);

                        printf("%s", "Please enter your CPU name: ");
                        fgets(cpu_str, MAX_STRLEN, stdin);
                        fputs(cpu_str, config_file);

                        printf("%s", "Please enter your GPU name: ");
                        fgets(gpu_str, MAX_STRLEN, stdin);
                        fputs(gpu_str, config_file);

                        printf("%s", "Please enter your shell name and version: ");
                        fgets(shell_str, MAX_STRLEN, stdin);
                        fputs(shell_str, config_file);

                        printf("%s", "Please enter your monitor resolution: ");
                        fgets(res_str, MAX_STRLEN, stdin);
                        fputs(res_str, config_file);

                        printf("%s", "Please enter your DE name: ");
                        fgets(de_str, MAX_STRLEN, stdin);
                        fputs(de_str, config_file);

                        printf("%s", "Please enter your WM name: ");
                        fgets(wm_str, MAX_STRLEN, stdin);
                        fputs(wm_str, config_file);

                        printf("%s", "Please enter your WM Theme name: ");
                        fgets(wm_theme_str, MAX_STRLEN, stdin);
                        fputs(wm_theme_str, config_file);

                        printf("%s", "Please enter any GTK info: ");
                        fgets(gtk_str, MAX_STRLEN, stdin);
                        fputs(gtk_str, config_file);

                        printf("%s\n", "That concludes the manual input.");
                        printf("%s\n", "From now on, screenfetch-c will use this information when called with -m.");

                        fclose(config_file);

                        /* i am deeply ashamed of this solution */
                        distro_str[strlen(distro_str) - 1] = '\0';
                        arch_str[strlen(arch_str) - 1] = '\0';
                        host_str[strlen(host_str) - 1] = '\0';
                        kernel_str[strlen(kernel_str) - 1] = '\0';
                        cpu_str[strlen(cpu_str) - 1] = '\0';
                        gpu_str[strlen(gpu_str) - 1] = '\0';
                        shell_str[strlen(shell_str) - 1] = '\0';
                        res_str[strlen(res_str) - 1] = '\0';
                        de_str[strlen(de_str) - 1] = '\0';
                        wm_str[strlen(wm_str) - 1] = '\0';
                        wm_theme_str[strlen(wm_theme_str) - 1] = '\0';
                        gtk_str[strlen(gtk_str) - 1] = '\0';

                        return EXIT_SUCCESS;
                }

                else
                {
                        printf("%s\n", "Exiting manual mode and screenfetch-c.");
                        printf("%s\n", "If you wish to run screenfetch-c normally, do not use the -m (--manual) flag next time.");

                        return EXIT_FAILURE;
                }
        }

        else
        {
                if (verbose)
                        VERBOSE_OUT("Found config file. Reading...", "");

                config_file = fopen(config_file_loc, "r");

                fgets(distro_str, MAX_STRLEN, config_file);
                fgets(arch_str, MAX_STRLEN, config_file);
                fgets(host_str, MAX_STRLEN, config_file);
                fgets(kernel_str, MAX_STRLEN, config_file);
                fgets(cpu_str, MAX_STRLEN, config_file);
                fgets(gpu_str, MAX_STRLEN, config_file);
                fgets(shell_str, MAX_STRLEN, config_file);
                fgets(res_str, MAX_STRLEN, config_file);
                fgets(de_str, MAX_STRLEN, config_file);
                fgets(wm_str, MAX_STRLEN, config_file);
                fgets(wm_theme_str, MAX_STRLEN, config_file);

                fgets(gtk_str, MAX_STRLEN, config_file);

                fclose(config_file);

                /* i am deeply ashamed of this solution */
                distro_str[strlen(distro_str) - 1] = '\0';
                arch_str[strlen(arch_str) - 1] = '\0';
                host_str[strlen(host_str) - 1] = '\0';
                kernel_str[strlen(kernel_str) - 1] = '\0';
                cpu_str[strlen(cpu_str) - 1] = '\0';
                gpu_str[strlen(gpu_str) - 1] = '\0';
                shell_str[strlen(shell_str) - 1] = '\0';
                res_str[strlen(res_str) - 1] = '\0';
                de_str[strlen(de_str) - 1] = '\0';
                wm_str[strlen(wm_str) - 1] = '\0';
                wm_theme_str[strlen(wm_theme_str) - 1] = '\0';
                gtk_str[strlen(gtk_str) - 1] = '\0';

                return EXIT_SUCCESS;
        }
}

/*  safe_strncpy
    calls strncpy with the given params, then inserts a null char at the last position
    returns a pointer to a string containing the copied data (same as destination)
    */
char* safe_strncpy(char* destination, const char* source, size_t num)
{
        char* ret = strncpy(destination, source, num);
        ret[num - 1] = '\0';
        return ret;
}

/*  split_uptime
    splits param uptime into individual time-units
    argument long uptime: the uptime, in seconds, to be split
    arguments int* secs...days: pointers to ints where the split uptime will be stored
    --
CAVEAT: uptime MUST be in seconds
--
*/
void split_uptime(long uptime, int* secs, int* mins, int* hrs, int* days)
{
        *secs = (int) uptime % 60;
        *mins = (int) (uptime / 60) % 60;
        *hrs = (int) (uptime / 3600) % 24;
        *days = (int) (uptime / 86400);

        return;
}

void output_logo_only(char* distro)
{
        // FIXME This is a temporary macro to make it easier to read the ridiculous boilerplate
        #define SF_DISPLAY_LOGO(condition, length, logo) \
        if (condition) \
                for (int i = 0; i < length; i++) \
                        printf("%s\n", logo[i]);

        SF_DISPLAY_LOGO(STRCMP(distro, "Windows"), 16, windows_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "OS X"), 16, macosx_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "Arch Linux - Old"), 18, oldarch_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "Arch Linux"), 18, arch_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "LinuxMint"), 18, mint_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "LMDE"), 18, lmde_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "Ubuntu" ) || STRCMP(distro, "Lubuntu") || STRCMP(distro, "Xubuntu"), 18, ubuntu_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "Debian"), 18, debian_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "CrunchBang"), 18, crunchbang_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "Gentoo"), 18, gentoo_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "Funtoo"), 18, funtoo_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "Fedora"), 18, fedora_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "Mandriva") || STRCMP(distro, "Mandrake"), 18, mandriva_mandrake_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "OpenSUSE"), 18, opensuse_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "Slackware"), 21, slackware_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "Red Hat Linux"), 18, redhat_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "Frugalware"), 23, frugalware_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "Peppermint"), 19, peppermint_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "SolusOS"), 18, solusos_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "Mageia"), 18, mageia_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "ParabolaGNU/Linux-libre"), 18, parabolagnu_linuxlibre_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "Viperr"), 18, viperr_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "LinuxDeepin"), 18, linuxdeepin_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "Chakra"), 18, chakra_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "Fuduntu"), 21, fuduntu_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "Trisquel"), 18, trisquel_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "Manjaro"), 18, manjaro_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "elementary OS"), 18, elementaryos_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "Scientific Linux"), 20, scientificlinux_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "Backtrack Linux"), 21, backtracklinux_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "Sabayon"), 18, sabayon_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "FreeBSD"), 18, freebsd_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "OpenBSD"), 23, openbsd_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "NetBSD"), 20, netbsd_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "DragonFly BSD"), 18, dragonflybsd_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "Android"), 16, android_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "Solaris"), 17, solaris_logo);
        SF_DISPLAY_LOGO(STRCMP(distro, "Angstrom"), 16, angstrom_logo););

        /* if (STRCMP(distro_str, "Linux")) */
        for (i = 0; i < 16; i++)
                printf("%s\n", linux_logo[i]);
}
