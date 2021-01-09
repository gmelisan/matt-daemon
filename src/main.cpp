#include <getopt.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>

#include "Matt_daemon.h"
#include "Tintin_reporter.h"
#include "Server.h"

Tintin_reporter ttr;
s_options g_options;
bool exit_flag;

static void sighandler(int n, siginfo_t *info, void */*ucontext*/)
{
    if (info)
        ttr.info("Signal handler: caught %d from pid %d.", n, info->si_pid);
    else
        ttr.info("Signal handler: caught %d.", n);
    exit_flag = true;
}

static void setup_signals()
{
    struct sigaction action;
    
    action.sa_sigaction = sighandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    for (int i = 1; i < 32; ++i) {
        sigaction(i, &action, nullptr);
    }
}

static void parse_options(int argc, char **argv)
{
    int opt;
    static char buf[100];

    strncpy(buf, DEFAULT_PORT_NUMBER, 99);
    g_options.port = buf;
    while ((opt = getopt(argc, argv, "hnp:uv"))) {
        switch (opt) {
        case 'h':
            printf("%s", HELP_TEXT);
            exit(EXIT_SUCCESS);
        case 'n':
            g_options.no_fork = true;
            break ;
        case 'p':
            g_options.port = optarg;
            break ;
        case 'u':
            g_options.udp = true;
            break ;
        case 'v':
            printf("%s, version %s\n", PROGRAM_NAME, PROGRAM_VERSION);
            exit(EXIT_SUCCESS);
        case '?':
            printf("%s", HELP_TEXT);
            exit(EXIT_FAILURE);
        }
    }
}

bool is_locked()
{
    struct stat st;

    if (stat(DEFAULT_LOCK_FILE_PATH, &st) == 0)
        return true;
    ttr.error("File locked.");
    return false;
}

bool lock()
{
    FILE *f = fopen(DEFAULT_LOCK_FILE_PATH, "w");
    if (!f) {
        ttr.error("Can't lock file: %s", strerror(errno));
        return false;
    }
    fclose(f);
    return true;
}

int main(int argc, char **argv)
{
    if (!ttr.init())
        return EXIT_FAILURE;
    ttr.info("Started.");
    if (is_locked() || !lock()) {
        ttr.info("Quitting.");
        return EXIT_FAILURE;
    }
    parse_options(argc, argv);
    setup_signals();
    Server server;
    server.start();
    server.clear();
    ttr.info("Quitting.");
    return EXIT_SUCCESS;
}