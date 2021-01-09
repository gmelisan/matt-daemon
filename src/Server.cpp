#include <sys/stat.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <cstdlib>
#include <sys/wait.h>

#include "Matt_daemon.h"
#include "Tintin_reporter.h"
#include "Server.h"

Server::Server()
{
    ttr.info("Creating server.");
    if (!init()) {
        matt_die(1);
    }
    ttr.info("Server created.");
}

Server::~Server()
{
}

Server::Server(const Server &s)
{
    (void)s;
}

Server& Server::operator=(const Server &s)
{
    (void)s;
    return *this;
}

void Server::clear()
{
    close(sfd);
}

bool Server::init()
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int ret, optval;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;
    if ((ret = getaddrinfo(nullptr, g_options.port, &hints, &result)) != 0) {
        ttr.error("getaddrinfo(): %s", gai_strerror(ret));
        return false;
    }
    optval = 1;
    for (rp = result; rp != nullptr; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue ;
        if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
            ttr.perror("setsockopt()");
            return false;
        }
        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break ;
        close(sfd);
    }
    if (!rp) {
        ttr.error("Could not bind socket to any address.");
        return false;
    }
    freeaddrinfo(result);
    return true;
}

static void spawn_shell()
{
    pid_t pid;
    int status;
    const char *cmd[] = {"sh", "-i", nullptr};

    pid = fork();
    if (pid < 0) {
        ttr.perror("fork()");
        return ;
    }
    if (pid == 0) {
        execv("/bin/sh", const_cast<char *const *>(cmd));
    }
    waitpid(pid, &status, 0);
    ttr.info("Quitting shell mode.");
}

bool Server::start()
{
    struct sockaddr_storage claddr;
    int cfd, ret;
    socklen_t addrlen;
    char c;
    std::string income;
    std::vector<std::string> v;

    daemon();
    if (listen(sfd, LISTEN_BACKLOG) == -1) {
        ttr.error("listen()");
        clear();
        return false;
    }
    addrlen = sizeof(struct sockaddr_storage);
    while (42) {
        cfd = accept(sfd, reinterpret_cast<struct sockaddr *>(&claddr), &addrlen);
        if (cfd == -1) {
            if (errno != EINTR) {
                ttr.perror("accept()");
                continue ;
            } else
                break ;
        }
        if (!g_options.dont_ask_passwd) {
            if (write(cfd, "Password:\n", 10))
                ttr.perror("write()");
            auth_mode = true;
            ttr.info("Asking for password.");
        }
        income.clear();
        while ((ret = read(cfd, &c, 1)) > 0) {
            if (c != '\n') {
                income.push_back(c);
            } else {
                if (income == "quit") {
                    ttr.info("Requested quit");
                    exit_flag = true;
                    break ;
                } else if (auth_mode) {
                    if (income == "2142") {
                        write(cfd, "Welcome\n", 8);
                        auth_mode = false;
                        ttr.info("Access granted.");
                    }
                } else if (income == "shell") {
                    ttr.info("Entering shell mode");
                    dup2(cfd, 0);
                    dup2(cfd, 1);
                    dup2(cfd, 2);
                    spawn_shell();
                } else if (income.size()) {
                    ttr.log("User input: '%s'", income.c_str());
                }
                income.clear();
            }
        }
        if (ret < 0) {
            ttr.perror("read()");
        }
        //split(income, v);
        
        close(cfd);
        if (exit_flag)
            break ;
    }
    clear();
    return exit_flag;
}


/*
void Server::split(std::string str, std::vector<std::string> &v)
{
    std::string msg;
    size_t pos = 0;

    if (str.empty())
        return ;
    v.clear();
    while ((pos = str.find('\n')) != std::string::npos) {
        v.push_back(str.substr(0, pos));
        str.erase(0, pos + 1);
    }
    v.push_back(str);
}
*/

void Server::daemon()
{
    pid_t pid;
    
    if (g_options.no_fork)
        return ;
    ttr.info("Entering Daemon mode.");
    pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);
    if (pid > 0)
        exit(EXIT_SUCCESS);
    if (setsid() < 0)
        exit(EXIT_FAILURE);
    signal(SIGCHLD, SIG_IGN);
    pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);
    if (pid > 0)
        exit(EXIT_SUCCESS);
    umask(0);
    ttr.info("started. PID: %d.", getpid());
}