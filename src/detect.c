/* NOTE detect_* functions seems to depend on their own platorms...
 * A better solution would be to put them in platform/<plat>/detect.c */

#include "detect.h"
#include "platform.h"
#include "utils.h"

void detect_distro(char* str)
{
        if (STRCMP(str, "Unknown") || STRCMP(str, "*")) /* if distro_str was NOT set by the -D flag or manual mode */
        {
                FILE* distro_file;

                char distro_name_str[MAX_STRLEN];

#ifdef PLATFORM_WINDOWS
                // TODO Get rid of NTDDI_* defines
#if defined(NTDDI_WIN7)
                safe_strncpy(str, "Microsoft Windows 7", MAX_STRLEN);
#elif defined(NTDDI_WIN8)
                safe_strncpy(str, "Microsoft Windows 8", MAX_STRLEN);
#elif defined(NTDDI_WINBLUE)
                safe_strncpy(str, "Microsoft Windows 8.1", MAX_STRLEN);
#elif defined(NTDDI_VISTA) || defined(NTDDI_VISTASP1)
                safe_strncpy(str, "Microsoft Windows Vista", MAX_STRLEN);
#elif defined(NTDDI_WINXP) || defined(NTDDI_WINXPSP1) || defined(NTDDI_WINXPSP2) || defined(NTDDI_WINXPSP3)
                safe_strncpy(str, "Microsoft Windows XP", MAX_STRLEN);
#elif defined(_WIN32_WINNT_WS03)
                safe_strncpy(str, "Microsoft Windows Server 2003", MAX_STRLEN);
#elif defined(_WIN32_WINNT_WS08)
                safe_strncpy(str, "Microsoft Windows Server 2008", MAX_STRLEN);
#else
                safe_strncpy(str, "Microsoft Windows", MAX_STRLEN);
#endif
#elif defined(PLATFORM_OSX)
                distro_file = popen("sw_vers | grep ProductVersion | tr -d 'ProductVersion: \\t\\n'", "r");
                fgets(distro_name_str, MAX_STRLEN, distro_file);
                pclose(distro_file);

                snprintf(str, MAX_STRLEN, "%s%s", "Mac OS X ", distro_name_str);
#elif defined(PLATFORM_LINUX)
                if (FILE_EXISTS("/system/bin/getprop"))
                {
                        safe_strncpy(str, "Android", MAX_STRLEN);
                }

                else
                {
                        bool detected = false;

                        /* Note: this is a very bad solution, as /etc/issue contains junk on some distros */
                        distro_file = fopen("/etc/issue", "r");

                        if (distro_file != NULL)
                        {
                                /* get the first 4 chars, that's all we need */
                                fscanf(distro_file, "%4s", distro_name_str);
                                fclose(distro_file);

                                if (STRCMP(distro_name_str, "Back"))
                                {
                                        safe_strncpy(str, "Backtrack Linux", MAX_STRLEN);
                                        detected = true;
                                }

                                else if (STRCMP(distro_name_str, "Crun"))
                                {
                                        safe_strncpy(str, "CrunchBang", MAX_STRLEN);
                                        detected = true;
                                }

                                else if (STRCMP(distro_name_str, "LMDE"))
                                {
                                        safe_strncpy(str, "LMDE", MAX_STRLEN);
                                        detected = true;
                                }

                                else if (STRCMP(distro_name_str, "Debi") || STRCMP(distro_name_str, "Rasp"))
                                {
                                        safe_strncpy(str, "Debian", MAX_STRLEN);
                                        detected = true;
                                }
                        }

                        if (!detected)
                        {
                                if (FILE_EXISTS("/etc/fedora-release"))
                                {
                                        safe_strncpy(str, "Fedora", MAX_STRLEN);
                                }

                                else if (FILE_EXISTS("/etc/SuSE-release"))
                                {
                                        safe_strncpy(str, "OpenSUSE", MAX_STRLEN);
                                }

                                else if (FILE_EXISTS("/etc/arch-release"))
                                {
                                        safe_strncpy(str, "Arch Linux", MAX_STRLEN);
                                }

                                else if (FILE_EXISTS("/etc/gentoo-release"))
                                {
                                        safe_strncpy(str, "Gentoo", MAX_STRLEN);
                                }

                                else if (FILE_EXISTS("/etc/angstrom-version"))
                                {
                                        safe_strncpy(str, "Angstrom", MAX_STRLEN);
                                }

                                else if (FILE_EXISTS("/etc/lsb-release"))
                                {
                                        distro_file = popen("head -1 < /etc/lsb-release | tr -d '\\\"\\n'", "r");
                                        fgets(distro_name_str, MAX_STRLEN, distro_file);
                                        pclose(distro_file);

                                        snprintf(str, MAX_STRLEN, "%s", distro_name_str + 11);
                                }

                                else
                                {
                                        safe_strncpy(str, "Linux", MAX_STRLEN);

                                        if (error)
                                        {
                                                ERROR_OUT("Error: ", "Failed to detect specific Linux distro.");
                                        }
                                }
                        }
                }
        }
#elif (defined(PLATFORM_BSD) || defined(PLATFORM_SOLARIS))
        distro_file = popen("uname -sr | tr -d '\\n'", "r");
        fgets(str, MAX_STRLEN, distro_file);
        pclose(distro_file);
#endif

        if (verbose)
                VERBOSE_OUT("Found distro as ", str);
}

void detect_arch(char* str)
{
        FILE* arch_file;

#ifdef PLATFORM_WINDOWS
        SYSTEM_INFO arch_info;
        GetNativeSystemInfo(&arch_info);

        if (arch_info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
        {
                safe_strncpy(str, "AMD64", MAX_STRLEN);
        }

        else if (arch_info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ARM)
        {
                safe_strncpy(str, "ARM", MAX_STRLEN);
        }

        else if (arch_info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
        {
                safe_strncpy(str, "IA64", MAX_STRLEN);
        }

        else if (arch_info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
        {
                safe_strncpy(str, "x86", MAX_STRLEN);
        }

        else
        {
                safe_strncpy(str, "Unknown", MAX_STRLEN);
        }
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_OSX) || defined(PLATFORM_SOLARIS)
        struct utsname arch_info;
        uname(&arch_info);
        safe_strncpy(str, arch_info.machine, MAX_STRLEN);
#elif defined(PLATFORM_BSD) 
        arch_file = popen("uname -m | tr -d '\\n'", "r");
        fgets(str, MAX_STRLEN, arch_file);
        pclose(arch_file);
#endif
}

/* NOTE Accesses environment variable */
void detect_host(char* str)
{
        char* given_user = "Unknown"; /* has to be a pointer for getenv()/GetUserName(), god knows why */
        char given_host[MAX_STRLEN] = "Unknown";

#ifdef PLATFORM_WINDOWS
        given_user = malloc(sizeof(char) * MAX_STRLEN);
        if (given_user == NULL)
        {
                ERROR_OUT("Error: ", "Failed to allocate sufficient memory in detect_host.");
                exit(1);
        }
        /* why does the winapi require a pointer to a long? */
        unsigned long len = MAX_STRLEN;
        GetUserName(given_user, &len);
        gethostname(given_host, MAX_STRLEN);

        free(given_user);
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_OSX) || defined(PLATFORM_SOLARIS)
        given_user = getlogin(); /* getlogin is apparently buggy on linux, so this might be changed */
        gethostname(given_host, MAX_STRLEN);
#elif defined(PLATFORM_BSD) 
        given_user = getenv("USER");

        FILE* host_file = popen("hostname | tr -d '\\r\\n '", "r");
        fgets(given_host, MAX_STRLEN, host_file);
        pclose(host_file);
#endif

        snprintf(str, MAX_STRLEN, "%s@%s", given_user, given_host);
}

// TODO Double check macros
void detect_kernel(char* str)
{
#ifdef PLATFORM_WINDOWS
        OSVERSIONINFO kern_info;
        ZeroMemory(&kern_info, sizeof(OSVERSIONINFO));
        kern_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        GetVersionEx(&kern_info);
        snprintf(str, MAX_STRLEN, "Windows NT %d.%d build %d", (int) kern_info.dwMajorVersion, (int) kern_info.dwMinorVersion, (int) kern_info.dwBuildNumber);
#elif defined(PLATFORM_BSD) 
        FILE* kernel_file = popen("uname -sr | tr -d '\\n'", "r");

        fscanf(kernel_file, "%s", str);

        pclose(kernel_file);
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_OSX) || defined(PLATFORM_SOLARIS)
        struct utsname kern_info;
        uname(&kern_info);
#endif
}

// TODO Double check macros
void detect_uptime(char* str)
{
        FILE* uptime_file;

        long uptime = 0; 
        long currtime = 0, boottime = 0; /* may or may not be used depending on OS */
        int secs = 0;
        int mins = 0;
        int hrs = 0;
        int days = 0;

#ifdef PLATFORM_WINDOWS
        uptime = GetTickCount(); /* known problem: will rollover after 49.7 days */
        uptime /= 1000;

#elif defined(PLATFORM_LINUX)
        struct sysinfo si_upt;
        sysinfo(&si_upt);

        uptime = si_upt.uptime;

#elif defined(PLATFORM_OSX) || defined(BSD_DRAGONFLY) || defined(BSD_FREE) 
        uptime_file = popen("sysctl -n kern.boottime | cut -d '=' -f 2 | cut -d ',' -f 1", "r");
        fscanf(uptime_file, "%ld", &boottime); /* get boottime in secs */
        pclose(uptime_file);

        currtime = time(NULL);

        uptime = currtime - boottime;

#elif defined(BSD_NET)
        uptime_file = popen("cut -d ' ' -f 1 < /proc/uptime", "r");
        fscanf(uptime_file, "%ld", &uptime);
        pclose(uptime_file);

#elif defined(BSD_OPEN)
        uptime_file = popen("sysctl -n kern.boottime", "r");
        fscanf(uptime_file, "%ld", &boottime); /* get boottime in secs */
        pclose(uptime_file);

        currtime = time(NULL);

        uptime = currtime - boottime;

#elif defined(PLATFORM_SOLARIS)
        currtime = time(NULL);
        struct utmpx* ent;

        while (ent = getutxent())
        {
                if (STRCMP("system boot", ent->ut_line))
                {
                        boottime = ent->ut_tv.tv_sec;
                }
        }

        uptime = currtime - boottime;
#endif

        split_uptime(uptime, &secs, &mins, &hrs, &days);

        if (days > 0)
                snprintf(str, MAX_STRLEN, "%dd %dh %dm %ds", days, hrs, mins, secs);
        else
                snprintf(str, MAX_STRLEN, "%dh %dm %ds", hrs, mins, secs);

        if (verbose)
                VERBOSE_OUT("Found uptime as ", str);

        return;
}

// NOTE relies on distro_str or something to be defined first
void detect_pkgs(char* str)
{
        FILE* pkgs_file;

        int packages = 0;

#if defined(PLATFORM_WINDOWS)
        pkgs_file = popen("cygcheck -cd | wc -l", "r");
        fscanf(pkgs_file, "%d", &packages);
        packages -= 2;
        pclose(pkgs_file);

        snprintf(str, MAX_STRLEN, "%d", packages);

#elif defined(PLATFORM_OSX)
        pkgs_file = popen("ls /usr/local/bin 2> /dev/null | wc -w", "r");
        fscanf(pkgs_file, "%d", &packages);
        pclose(pkgs_file);

        if (FILE_EXISTS("/usr/local/bin/brew"))
        {
                int brew_pkgs = 0;
                pkgs_file = popen("brew list -1 | wc -l", "r");
                fscanf(pkgs_file, "%d", &brew_pkgs);
                pclose(pkgs_file);

                packages += brew_pkgs;
        }

        if (FILE_EXISTS("/opt/local/bin/port"))
        {
                int port_pkgs = 0;
                pkgs_file = popen("port installed | wc -l", "r");
                fscanf(pkgs_file, "%d", &port_pkgs);
                pclose(pkgs_file);

                packages += port_pkgs;
        }

        if (FILE_EXISTS("/sw/bin/fink"))
        {
                int fink_pkgs = 0;
                pkgs_file = popen("/sw/bin/fink list -i | wc -l", "r");
                fscanf(pkgs_file, "%d", &fink_pkgs);
                pclose(pkgs_file);

                packages += fink_pkgs;
        }

#elif defined(PLATFORM_LINUX)
        if (STRCMP(distro_str, "Arch Linux") || STRCMP(distro_str, "ParabolaGNU/Linux-libre") || STRCMP(distro_str, "Chakra") || STRCMP(distro_str, "Manjaro"))
        {
                pkgs_file = popen("pacman -Qq | wc -l", "r");
                fscanf(pkgs_file, "%d", &packages);
                pclose(pkgs_file);
        }

        else if (STRCMP(distro_str, "Frugalware"))
        {
                pkgs_file = popen("pacman-g2 -Q | wc -l", "r");
                fscanf(pkgs_file, "%d", &packages);
                pclose(pkgs_file);
        }

        else if (STRCMP(distro_str, "Ubuntu") || STRCMP(distro_str, "Lubuntu") || STRCMP(distro_str, "Xubuntu") || STRCMP(distro_str, "LinuxMint") || STRCMP(distro_str, "SolusOS") || STRCMP(distro_str, "Debian") || STRCMP(distro_str, "LMDE") || STRCMP(distro_str, "CrunchBang") || STRCMP(distro_str, "Peppermint") || STRCMP(distro_str, "LinuxDeepin") || STRCMP(distro_str, "Trisquel") || STRCMP(distro_str, "elementary OS") || STRCMP(distro_str, "Backtrack Linux"))
        {
                pkgs_file = popen("dpkg --get-selections | wc -l", "r");
                fscanf(pkgs_file, "%d", &packages);
                pclose(pkgs_file);
        }

        else if (STRCMP(distro_str, "Slackware"))
        {
                pkgs_file = popen("ls -l /var/log/packages | wc -l", "r");
                fscanf(pkgs_file, "%d", &packages);
                pclose(pkgs_file);
        }

        else if (STRCMP(distro_str, "Gentoo") || STRCMP(distro_str, "Sabayon") || STRCMP(distro_str, "Funtoo"))
        {
                pkgs_file = popen("ls -d /var/db/pkg/*/* | wc -l", "r");
                fscanf(pkgs_file, "%d", &packages);
                pclose(pkgs_file);
        }

        else if (STRCMP(distro_str, "Fuduntu") || STRCMP(distro_str, "Fedora") || STRCMP(distro_str, "OpenSUSE") || STRCMP(distro_str, "Red Hat Linux") || STRCMP(distro_str, "Mandriva") || STRCMP(distro_str, "Mandrake") || STRCMP(distro_str, "Mageia") || STRCMP(distro_str, "Viperr"))
        {
                pkgs_file = popen("rpm -qa | wc -l", "r");
                fscanf(pkgs_file, "%d", &packages);
                pclose(pkgs_file);
        }

        else if (STRCMP(distro_str, "Angstrom"))
        {
                pkgs_file = popen("opkg list-installed | wc -l", "r");
                fscanf(pkgs_file, "%d", &packages);
                pclose(pkgs_file);
        }

        /* if linux disto detection failed */
        else if (STRCMP(distro_str, "Linux"))
        {
                safe_strncpy(str, "Not Found", MAX_STRLEN);

                if (error)
                        ERROR_OUT("Error: ", "Packages cannot be detected on an unknown Linux distro.");
        }

#elif defined(BSD_FREE) || defined(BSD_OPEN)
        pkgs_file = popen("pkg_info | wc -l", "r");
        fscanf(pkgs_file, "%d", &packages);
        pclose(pkgs_file);

#elif defined(BSD_NET) || defined(BSD_DRAGONFLY)
        safe_strncpy(str, "Not Found", MAX_STRLEN);

        if (error)
                ERROR_OUT("Error: ", "Could not find packages on current OS.");

#elif defined(PLATFORM_SOLARIS)
        pkgs_file = popen("pkg list | wc -l", "r");
        fscanf(pkgs_file, "%d", &packages);
        pclose(pkgs_file);
#endif

        snprintf(str, MAX_STRLEN, "%d", packages);

        if (verbose)
                VERBOSE_OUT("Found package count as ", str);
}

// TODO Double check macros
void detect_cpu(char* str)
{
        FILE* cpu_file;

#ifdef PLATFORM_WINDOWS
        HKEY hkey;
        DWORD str_size = MAX_STRLEN;
        RegOpenKey(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", &hkey);
        RegQueryValueEx(hkey, "ProcessorNameString", 0, NULL, (BYTE*) str, &str_size);

#elif defined(PLATFORM_OSX)
        cpu_file = popen("sysctl -n machdep.cpu.brand_string | sed 's/(\\([Tt][Mm]\\))//g;s/(\\([Rr]\\))//g;s/^//g' | tr -d '\\n' | tr -s ' '", "r");
        fgets(str, MAX_STRLEN, cpu_file);
        pclose(cpu_file);

#elif defined(PLATFORM_LINUX) || defined(BSD_NET)
        cpu_file = popen("awk 'BEGIN{FS=\":\"} /model name/ { print $2; exit }' /proc/cpuinfo | sed -e 's/ @/\\n/' -e 's/^ *//g' -e 's/ *$//g' | head -1 | tr -d '\\n'", "r");
        fgets(str, MAX_STRLEN, cpu_file);
        pclose(cpu_file);

        if (STRCMP(str, "ARMv6-compatible processor rev 7 (v6l)"))
        {
                safe_strncpy(str, "BCM2708 (Raspberry Pi)", MAX_STRLEN); /* quick patch for Raspberry Pi machines */
        }

#elif defined(BSD_DRAGONFLY) || defined(BSD_FREE) || defined(BSD_OPEN)
        cpu_file = popen("sysctl -n hw.model | tr -d '\\n'", "r");
        fgets(str, MAX_STRLEN, cpu_file);
        pclose(cpu_file);

#elif defined(PLATFORM_SOLARIS)
        cpu_file = popen("psrinfo -pv | tail -1 | tr -d '\\t\\n'", "r");
        fgets(str, MAX_STRLEN, cpu_file);
        pclose(cpu_file);
#endif

        if (verbose)
                VERBOSE_OUT("Found CPU as ", str);
}

void detect_gpu(char* str)
{
        FILE* gpu_file;

#ifdef PLATFORM_WINDOWS
        HKEY hkey;
        DWORD str_size = MAX_STRLEN;
        RegOpenKey(HKEY_LOCAL_MACHINE, "SYSTEM\\ControlSet001\\Control\\Class\\{4D36E968-E325-11CE-BFC1-08002BE10318}\\0000\\Settings", &hkey);
        RegQueryValueEx(hkey, "Device Description", 0, NULL, (BYTE*) str, &str_size);

#elif defined(PLATFORM_OSX)
        gpu_file = popen("system_profiler SPDisplaysDataType | awk -F': ' '/^\\ *Chipset Model:/ {print $2}' | tr -d '\\n'", "r");
        fgets(str, MAX_STRLEN, gpu_file);
        pclose(gpu_file);

#elif defined(PLATFORM_LINUX) || defined(PLATFORM_BSD) || defined(PLATFORM_SOLARIS)
        gpu_file = popen("detectgpu 2>/dev/null", "r");
        fgets(str, MAX_STRLEN, gpu_file);
        pclose(gpu_file);
#endif

        if (verbose)
                VERBOSE_OUT("Found GPU as ", str);
}

void detect_disk(char* str)
{
        FILE* disk_file;

        int disk_total = 0;
        int disk_used = 0;
        int disk_percentage = 0;

#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_LINUX) || defined(PLATFORM_OSX)
        /* Cygwin -- GetDiskFreeSpaceEx? */

        disk_file = popen("df -H 2> /dev/null | grep -vE '^[A-Z]\\:\\/|File' | awk '{ print $2 }' | head -1 | tr -d '\\r\\n G'", "r");
        fscanf(disk_file, "%d", &disk_total);
        pclose(disk_file);

        disk_file = popen("df -H 2> /dev/null | grep -vE '^[A-Z]\\:\\/|File' | awk '{ print $3 }' | head -1 | tr -d '\\r\\n G'", "r");
        fscanf(disk_file, "%d", &disk_used);
        pclose(disk_file);

#elif defined(BSD_FREE)
        disk_file = popen("df -h -c 2> /dev/null | grep -vE '^[A-Z]\\:\\/|File' | awk '{ print $2 }' | tail -1 | tr -d '\\r\\n G'", "r");
        fscanf(disk_file, "%d", &disk_total);
        pclose(disk_file);

        disk_file = popen("df -h -c 2> /dev/null | grep -vE '^[A-Z]\\:\\/|File' | awk '{ print $3 }' | tail -1 | tr -d '\\r\\n G'", "r");
        fscanf(disk_file, "%d", &disk_used);
        pclose(disk_file);

#elif defined(PLATFORM_BSD)
        disk_file = popen("df -h 2> /dev/null | grep -vE '^[A-Z]\\:\\/|File' | awk '{ print $2 }' | head -1 | tr -d '\\r\\n G'", "r");
        fscanf(disk_file, "%d", &disk_total);
        pclose(disk_file);

        disk_file = popen("df -h 2> /dev/null | grep -vE '^[A-Z]\\:\\/|File' | awk '{ print $3 }' | head -1 | tr -d '\\r\\n G'", "r");
        fscanf(disk_file, "%d", &disk_used);
        pclose(disk_file);

#elif defined(PLATFORM_SOLARIS)
        /* not yet implemented */

#endif

        if (disk_total > disk_used)
        {
                disk_percentage = (((float) disk_used / disk_total) * 100);

                snprintf(str, MAX_STRLEN, "%dG / %dG (%d%%)", disk_used, disk_total, disk_percentage);
        }

        else /* when disk_used is in a smaller unit */
        {
                disk_percentage = ((float) disk_used / (disk_total * 1024) * 100);

                snprintf(str, MAX_STRLEN, "%dM / %dG (%d%%)", disk_used, disk_total, disk_percentage);
        }


        if (verbose)
                VERBOSE_OUT("Found disk usage as ", str);
}

/* Uses format 'nMB / nMB', where n is a number */
void detect_mem(char* str)
{
        FILE* mem_file;

        long long total_mem = 0; /* each of the following MAY contain bytes/kbytes/mbytes/pages */
        long long free_mem = 0;
        long long used_mem = 0;

#ifdef PLATFORM_WINDOWS
        MEMORYSTATUSEX mem_stat;
        mem_stat.dwLength = sizeof(mem_stat);
        GlobalMemoryStatusEx(&mem_stat);

        total_mem = (long long) mem_stat.ullTotalPhys / MB;
        used_mem = total_mem - ((long long) mem_stat.ullAvailPhys / MB);

#elif defined(PLATFORM_OSX)
        mem_file = popen("sysctl -n hw.memsize", "r");
        fscanf(mem_file, "%lld", &total_mem);
        pclose(mem_file);

        mem_file = popen("vm_stat | head -2 | tail -1 | tr -d 'Pages free: .'", "r");
        fscanf(mem_file, "%lld", &free_mem);
        pclose(mem_file);

        total_mem /= (long) MB;

        free_mem *= 4096; /* 4KiB is OS X's page size */
        free_mem /= (long) MB;

        used_mem = total_mem - free_mem;

#elif defined(PLATFORM_LINUX)
        /* known problem: because linux utilizes free ram extensively in caches/buffers,
           the amount of memory sysinfo reports as free is very small.
           */
        struct sysinfo si_mem;
        sysinfo(&si_mem);

        total_mem = (long long) (si_mem.totalram * si_mem.mem_unit) / MB;
        free_mem = (long long) (si_mem.freeram * si_mem.mem_unit) / MB;
        used_mem = (long long) total_mem - free_mem;

#elif defined(PLATFORM_BSD)
        mem_file = popen("sysctl -n hw.physmem", "r");
        fscanf(mem_file, "%lld", &total_mem);
        pclose(mem_file);

        total_mem /= (long) MB;

#elif defined(PLATFORM_SOLARIS)
        total_mem = sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGE_SIZE);
        total_mem /= (long) MB;

        /* sar -r 1 to get free pages? */
#endif

#if defined(PLATFORM_BSD) || defined(PLATFORM_SOLARIS)
        snprintf(str, MAX_STRLEN, "%lld%s", total_mem, "MB");
#else
        snprintf(str, MAX_STRLEN, "%lld%s / %lld%s", used_mem, "MB", total_mem, "MB");
#endif
}

/* CAVEAT: shell version detection relies on the standard versioning format for
 * each shell. If any shell's older (or newer versions) suddenly begin to use a
 * new scheme, the version may be displayed incorrectly.  */
void detect_shell(char* str)
{
        FILE* shell_file;

        char shell_name[MAX_STRLEN];
        char vers_str[MAX_STRLEN];

        shell_file = popen("echo $SHELL | awk -F \"/\" '{print $NF}' 2> /dev/null | tr -d '\\r\\n'", "r");
        fgets(shell_name, MAX_STRLEN, shell_file);
        pclose(shell_file);

        if (STRCMP(shell_name, "bash"))
        {
                shell_file = popen("bash --version | head -1", "r");
                fgets(vers_str, MAX_STRLEN, shell_file);
                /* evil pointer arithmetic */
                snprintf(str, MAX_STRLEN, "%s %.*s", shell_name, 17, vers_str + 10);
                pclose(shell_file);
        }

        else if (STRCMP(shell_name, "zsh"))
        {
                shell_file = popen("zsh --version", "r");
                fgets(vers_str, MAX_STRLEN, shell_file);	
                /* evil pointer arithmetic */
                snprintf(str, MAX_STRLEN, "%s %.*s", shell_name, 5, vers_str + 4);
                pclose(shell_file);
        }

        else if (STRCMP(shell_name, "csh"))
        {
                shell_file = popen("csh --version | head -1", "r");
                fgets(vers_str, MAX_STRLEN, shell_file);
                /* evil pointer arithmetic */
                snprintf(str, MAX_STRLEN, "%s %.*s", shell_name, 7, vers_str + 5);
                pclose(shell_file);
        }

        else if (STRCMP(shell_name, "fish"))
        {
                shell_file = popen("fish --version", "r");
                fgets(vers_str, MAX_STRLEN, shell_file);
                /* evil pointer arithmetic */
                snprintf(str, MAX_STRLEN, "%s %.*s", shell_name, 13, vers_str + 6);
                pclose(shell_file);
        }

        else if (STRCMP(shell_name, "dash") || STRCMP(shell_name, "ash") || STRCMP(shell_name, "ksh"))
        {
                /* i don't have a version detection system for these, yet */
                safe_strncpy(str, shell_name, MAX_STRLEN);
        }

        if (verbose)
                VERBOSE_OUT("Found shell as ", str);
}

void detect_res(char* str)
{
        FILE* res_file;

        int width = 0;
        int height = 0;

#ifdef PLATFORM_WINDOWS
        width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
        snprintf(str, MAX_STRLEN, "%dx%d", width, height);

#elif defined(PLATFORM_OSX)
        res_file = popen("system_profiler SPDisplaysDataType | awk '/Resolution:/ {print $2\"x\"$4}' | tr -d '\\n'", "r");
        fgets(str, MAX_STRLEN, res_file);
        pclose(res_file);

#elif defined(PLATFORM_LINUX) || defined(PLATFORM_SOLARIS)
        Display* disp = XOpenDisplay(NULL);
        if (disp != NULL)
        {
                Screen* screen = XDefaultScreenOfDisplay(disp);
                width = WidthOfScreen(screen);
                height = HeightOfScreen(screen);
                snprintf(str, MAX_STRLEN, "%dx%d", width, height);
        }
        else
        {
                safe_strncpy(str, "No X Server", MAX_STRLEN);

                if (error)
                        ERROR_OUT("Error: ", "Problem detecting X display resolution.");
        }

#elif defined(PLATFORM_BSD)
        res_file = popen("xdpyinfo 2> /dev/null | sed -n 's/.*dim.* \\([0-9]*x[0-9]*\\) .*/\\1/pg' | tr '\\n' ' '", "r");
        fgets(str, MAX_STRLEN, res_file);
        pclose(res_file);

        if (STRCMP(str, "Unknown"))
        {
                safe_strncpy(str, "No X Server", MAX_STRLEN);
        }

#endif

        if (verbose)
                VERBOSE_OUT("Found resolution as ", str);
}

/*  CAVEAT: On *BSDs and Linux distros, this function relies on the presence of
 *  'detectde', a shell script. If it isn't present in the working directory,
 *  the DE will be set as 'Unknown' */
void detect_de(char* str)
{
        FILE* de_file;

#if defined(PLATFORM_WINDOWS)
        int version;

        de_file = popen("wmic os get version | grep -o '^[0-9]'", "r");
        fscanf(de_file, "%d", &version);
        pclose(de_file);

        if (version == 6 || version == 7)
        {
                safe_strncpy(str, "Aero", MAX_STRLEN);
        }
        else
        {
                safe_strncpy(str, "Luna", MAX_STRLEN);
        }

#elif defined(PLATFORM_OSX)
        safe_strncpy(str, "Aqua", MAX_STRLEN);

#elif defined(PLATFORM_LINUX) || defined(PLATFORM_BSD)
        de_file = popen("detectde 2> /dev/null", "r");
        fgets(str, MAX_STRLEN, de_file);
        pclose(de_file);

#elif defined(PLATFORM_SOLARIS)
        /* detectde needs to be made compatible with Solaris's AWK */
#endif

        if (verbose)
                VERBOSE_OUT("Found DE as ", str);
}

/* CAVEAT: On *BSDs and Linux distros, this function relies on the presence of
 * 'detectwm', a shell script. If it isn't present in the working directory,
 * the WM will be set as 'Unknown' */
void detect_wm(char* str)
{
        FILE* wm_file;

#if defined(PLATFORM_WINDOWS)
        /* wm_file = popen("tasklist | grep -o 'bugn' | tr -d '\\r\\n'", "r"); */
        /* test for bugn */
        /* pclose(wm_file); */

        /* wm_file = popen("tasklist | grep -o 'Windawesome' | tr -d '\\r \\n'", "r"); */
        /* test for Windawesome */
        /* pclose(wm_file); */

        /* else */
        safe_strncpy(str, "DWM", MAX_STRLEN);

#elif defined(PLATFORM_OSX)
        safe_strncpy(str, "Quartz Compositor", MAX_STRLEN);

#elif defined(PLATFORM_LINUX) || defined(PLATFORM_BSD) || defined(PLATFORM_SOLARIS)
        wm_file = popen("detectwm 2> /dev/null", "r");
        fgets(str, MAX_STRLEN, wm_file);
        pclose(wm_file);
#endif

        if (verbose)
                VERBOSE_OUT("Found WM as ", str);
}

/* CAVEAT: On *BSDs and Linux distros, this function relies on the presence of
 * 'detectwmtheme', a shell script. If it isn't present in the working
 * directory, the WM Theme will be set as 'Unknown' */
void detect_wm_theme(char* str)
{
        FILE* wm_theme_file;

#if defined(PLATFORM_WINDOWS)
        /* nasty one-liner */
        wm_theme_file = popen("reg query 'HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes' /v 'CurrentTheme' | grep -o '[A-Z]:\\\\.*' | awk -F\"\\\\\" '{print $NF}' | grep -o '[0-9A-z. ]*$' | grep -o '^[0-9A-z ]*' | tr -d '\\r\\n'", "r");
        fgets(str, MAX_STRLEN, wm_theme_file);
        pclose(wm_theme_file);

#elif defined(PLATFORM_OSX)
        safe_strncpy(str, "Aqua", MAX_STRLEN);

#elif defined(PLATFORM_LINUX) || defined(PLATFORM_BSD) || defined(PLATFORM_SOLARIS)
        wm_theme_file = popen("detectwmtheme 2> /dev/null", "r");
        fgets(str, MAX_STRLEN, wm_theme_file);
        pclose(wm_theme_file);
#endif

        if (verbose)
                VERBOSE_OUT("Found WM theme as ", str);
}

/* detects the theme, icon(s), and font(s) associated with a GTK DE (if
 * present) CAVEAT: On *BSDs and Linux distros, this function relies on the
 * presence of 'detectgtk', a shell script. If it isn't present in the working
 * directory, the GTK will be set as 'Unknown' */
void detect_gtk(char* str)
{
        FILE* gtk_file;

        char gtk2_str[MAX_STRLEN] = "Unknown";
        char gtk3_str[MAX_STRLEN] = "Unknown";
        char gtk_icons_str[MAX_STRLEN] = "Unknown";
        char font_str[MAX_STRLEN] = "Unknown";

#if defined(PLATFORM_WINDOWS)
        /* get the terminal's font */
        gtk_file = popen("grep '^Font=.*' < $HOME/.minttyrc | grep -o '[0-9A-z ]*$' | tr -d '\\r\\n'", "r");
        fgets(font_str, MAX_STRLEN, gtk_file);
        pclose(gtk_file);

        snprintf(str, MAX_STRLEN, "%s (Font)", font_str);

#elif defined(PLATFORM_OSX)
        safe_strncpy(str, "Not Applicable", MAX_STRLEN);

#elif defined(PLATFORM_LINUX) || defined(PLATFORM_BSD)
        gtk_file = popen("detectgtk 2> /dev/null", "r");
        fscanf(gtk_file, "%s%s%s%s", gtk2_str, gtk3_str, gtk_icons_str, font_str);
        pclose(gtk_file);

        if (STRCMP(gtk3_str, "Unknown"))
                snprintf(str, MAX_STRLEN, "%s (GTK2), %s (Icons)", gtk2_str, gtk_icons_str);
        else if (STRCMP(gtk2_str, "Unknown"))
                snprintf(str, MAX_STRLEN, "%s (GTK3), %s (Icons)", gtk3_str, gtk_icons_str);
        else
                snprintf(str, MAX_STRLEN, "%s (GTK2), %s (GTK3)", gtk2_str, gtk3_str);

#elif defined(PLATFORM_SOLARIS)
        /* detectgtk needs to be made compatible with Solaris's awk */
#endif

        if (verbose)
                VERBOSE_OUT("Found GTK as ", str);
}
