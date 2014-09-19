#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "detect.h"
#include "platform.h"
#include "screenfetch.h"
#include "thread.h"
#include "utils.h"
#include "version.h"

/* other definitions */
static bool manual = false;
static bool logo = true;
static bool error = true;
static bool verbose = false;
static bool screenshot = false;

static const char* screenfetch_unknown = "Unknown";

int main(int argc, char** argv)
{
        /* Set up */
        struct screenfetch_t sf = {
                .distro   = {.title = "OS"         , .value = screenfetch_unknown},
                .arch     = {.title = "Arch"       , .value = screenfetch_unknown},
                .host     = {.title = "Host"       , .value = screenfetch_unknown},
                .kernel   = {.title = "Kernel"     , .value = screenfetch_unknown},
                .uptime   = {.title = "Uptime"     , .value = screenfetch_unknown},
                .pkgs     = {.title = "Packages"   , .value = screenfetch_unknown},
                .cpu      = {.title = "CPU"        , .value = screenfetch_unknown},
                .gpu      = {.title = "GPU"        , .value = screenfetch_unknown},
                .disk     = {.title = "Disk"       , .value = screenfetch_unknown},
                .mem      = {.title = "Memory"     , .value = screenfetch_unknown},
                .shell    = {.title = "Shell"      , .value = screenfetch_unknown},
                .res      = {.title = "Resolution" , .value = screenfetch_unknown},
                .de       = {.title = "DE"         , .value = screenfetch_unknown},
                .wm       = {.title = "WM"         , .value = screenfetch_unknown},
                .wm_theme = {.title = "WM Theme"   , .value = screenfetch_unknown},
                .gtk      = {.title = "GTK"        , .value = screenfetch_unknown}
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

        signed char c; int index = 0;
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

        /* Program */

        detect_uptime (sf.uptime);
        detect_pkgs   (sf.pkgs);
        detect_disk   (sf.disk);
        detect_mem    (sf.mem);

        if (manual)
                int stat = manual_input();
        else
        {
                detect_distro   (sf.distro);
                detect_arch     (sf.arch);
                detect_host     (sf.host);
                detect_kernel   (sf.kernel);
                detect_cpu      (sf.cpu);
                detect_gpu      (sf.gpu);
                detect_shell    (sf.shell);
                detect_res      (sf.res);
                detect_de       (sf.de);
                detect_wm       (sf.wm);
                detect_wm_theme (sf.wm_theme);
                detect_gtk      (sf.gtk);
        }

        if (logo)
                main_ascii_output(detected_arr, detected_arr_names);
        else
                main_text_output(detected_arr, detected_arr_names);

        if (screenshot)
                take_screenshot();

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

        #ifdef PLATFORM_WINDOWS
        /* terrible hack, the printscreen key is simulated */
        keybd_event(VK_SNAPSHOT, 0, 0, 0);

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

        return;
        #else
        #if defined(PLATFORM_OSX)
        system("screencapture -x ~/screenfetch_screenshot.png 2> /dev/null");	
        #elif (defined(PLATFORM_LINUX) || defined(PLATFORM_BSD))
        system("scrot ~/screenfetch_screenshot.png 2> /dev/null");
        #endif

        char* loc = getenv("HOME");
        strncat(loc, "/screenfetch_screenshot.png", MAX_STRLEN);

        return;
        #endif
}

/*  manual_input
    generates (or reads) the ~/.screenfetchc file based upon user input
    returns an int indicating status (EXIT_SUCCESS or EXIT_FAILURE)
    */
int manual_input(struct screenfetch_t sf)
{
        FILE* config_file;
        char *home = getenv("HOME");
        char config_file_loc[MAX_STRLEN] = {0};

        strncat(config_file_loc, home, MAX_STRLEN);
        strncat(config_file_loc, "/.screenfetchc", MAX_STRLEN);

        if (!FILE_EXISTS(config_file_loc))
        {
                printf("Use * to detect normally.\n");

                config_file = fopen(config_file_loc, "w");

                /* Makes the very bold assumption that all members of screenfetch_t will be char_pair_t */
                struct char_pair_t list[] = (struct char_pair_t *)&sf;
                
                for(int i = 0, i < sizeof(struct screenfetch_t)/sizeof(struct char_pair_t); ++i)
                {
                        char *line = calloc(MAX_STRLEN, 1);
                        printf("%s: ", list[i].title);
                        fgets(line, MAX_STRLEN, stdin);
                        list[i].value = line;
                }

                printf("%s\n", "Saving...");
                /* TODO Save config. */

                fclose(config_file);

                return EXIT_SUCCESS;
        } else
        {
                config_file = fopen(config_file_loc, "r");

                /* TODO Parse config. */
                fclose(config_file);

                return EXIT_SUCCESS;
        }

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
                if (condition) { for (int i = 0; i < length; i++) { printf("%s\n", logo[i]); } return; }

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
