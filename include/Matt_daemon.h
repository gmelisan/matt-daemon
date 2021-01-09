#ifndef MATT_DAEMON_H
# define MATT_DAEMON_H

# define PROGRAM_NAME           "Matt_daemon"
# define PROGRAM_VERSION        "0.1.0"
# define DEFAULT_LOG_DIR        "/var/log/matt_daemon"
# define DEFAULT_LOG_PATH       "/var/log/matt_daemon/matt_daemon.log"

# define DEFAULT_LOCK_FILE_PATH "/var/lock/matt_daemon.lock"

# define DEFAULT_PORT_NUMBER    "4242"
# define LISTEN_BACKLOG         1

# define HELP_TEXT \
"Usage: " PROGRAM_NAME " -[hnuv]\n" \
"Available options:\n" \
"    -h          This help\n" \
"    -n          No fork at startup\n" \
"    -p [N]      Listen to port N (default " DEFAULT_PORT_NUMBER ")\n" \
"    -v          Print program version and exit\n" \
""

struct s_options
{
    bool no_fork;
    char *port;
};

extern s_options g_options;
extern bool exit_flag;

void matt_die(int code, bool unl = true);

#endif
