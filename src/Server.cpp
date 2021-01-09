#include <sys/stat.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <cstdlib>

#include "Matt_daemon.h"
#include "Tintin_reporter.h"
#include "Server.h"

Server::Server()
{
    ttr.info("Creating server.");
    if (!init()) {
        ttr.info("Quitting.");
        exit(EXIT_FAILURE);
    }
    ttr.info("Server created");
}

Server::~Server()
{}

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

bool Server::start()
{
    struct sockaddr_storage claddr;
    int cfd, ret;
    socklen_t addrlen;
    char buf[100];
    std::string income;

    daemon();
    if (listen(sfd, LISTEN_BACKLOG) == -1) {
        ttr.error("listen()");
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
        income.clear();
        while ((ret = read(cfd, buf, 99)) > 0) {
            income += buf;
            memset(buf, 0, 100);
        }
        if (income == "quit") {
            ttr.info("Requested quit");
            exit_flag = true;
            break ;
        }
        ttr.log("%s", income.c_str());
    }
    return exit_flag;
}

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
    ttr.info("started. PID: %d", getpid());
}