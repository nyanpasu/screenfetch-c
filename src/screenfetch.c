#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "screenfetch.h"
#include "detect.h"
#include "thread.h"

static const char* screenfetch_unknown = "Unknown";

/* other definitions */
bool manual = false;
bool logo = true;
bool error = true;
bool verbose = false;
bool screenshot = false;

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

        /* warn unknown OSes about using this program */
        if (OS == UNKNOWN)
        {
                ERROR_OUT("Warning: ", "This program isn't designed for your OS.");
                ERROR_OUT("Even if it did compile successfully, it will not execute correctly.", "");
                ERROR_OUT("It is HIGHLY recommended, therefore, that you use manual mode.", "");
        }

        struct option long_options[] =
        {
                {"manual", no_argument, 0, 'm'},
                {"verbose", no_argument, 0, 'v'},
                {"no-logo", no_argument, 0, 'n'},
                {"screenshot", no_argument, 0, 's'},
                {"distro", required_argument, 0, 'D'},
                {"suppress-errors", no_argument, 0, 'E'},
                {"version", no_argument, 0, 'V'},
                {"help", no_argument, 0, 'h'},
                {"logo-only", required_argument, 0, 'L'},
                {0, 0, 0, 0}
        };

        /* TOOO Move these characters to the scope of the loop. */
        signed char c;
        int index = 0;
        while ((c = getopt_long(argc, argv, "mvnsD:EVhL:", long_options, &index)) != -1)
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
                                display_help();
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

/*  display_version
    called if the -v flag is tripped, outputs the current version of screenfetch-c
    */
void display_version(void)
{
        printf("%s\n", "screenfetch-c - Version 1.2, revision 3");
        return;
}

/*  display_help
    called if the -h flag is tripped, tells the user where to find the manpage
    */
void display_help(void)
{
        printf("%s\n", "screenfetch-c");
        printf("%s\n", "A rewrite of screenFetch, the popular shell script, in C.");
        printf("%s\n", "Operating Systems currently supported:");
        printf("%s\n", "Windows (via Cygwin), Linux, *BSD, OS X, and Solaris.");
        printf("%s\n", "Using screenfetch-c on an OS not listed above may not work entirely or at all (and is disabled by default).");
        printf("%s\n", "Please access 'man screenfetch' for in-depth information on compatibility and usage.");
        return;
}

/*  take_screenshot
    takes a screenshot and saves it to $HOME/screenfetch_screenshot.jpg
    --
CAVEAT: THIS FUNCTION MAKES SYSTEM CALLS
--
*/
void take_screenshot(void)
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
                {
                        ERROR_OUT("Error: ", "Problem saving screenshot.");
                }
        }

        return;
}
