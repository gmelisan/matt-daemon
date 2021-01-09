#include <sys/stat.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <cstdlib>
#include <sys/wait.h>
#include <cstdio>
#include <cstring>
#include <poll.h>

#include <string>

#define PROGRAM_NAME    "Ben_AFK"
#define PORT_NUMBER     "4242"

#define HELP_TEXT \
"Usage: " PROGRAM_NAME " -[ahp]\n" \
"Options:\n" \
"    -a [address]   Address name (default 'localhost')\n" \
"    -h             This help\n" \
"    -p [port]      Port number (default " PORT_NUMBER ")\n" \
"" 

#define BUFSIZE     128

static char g_host[BUFSIZE];
static char g_port[BUFSIZE];

static void parse_options(int argc, char **argv)
{
    int opt;

    strncpy(g_host, "localhost", BUFSIZE - 1);
    strncpy(g_port, PORT_NUMBER, BUFSIZE - 1);
    while ((opt = getopt(argc, argv, "a:hp:")) != -1) {
        switch (opt) {
        case 'a':
            strncpy(g_host, optarg, BUFSIZE - 1);
            break ;
        case 'h':
            printf("%s", HELP_TEXT);
            exit(EXIT_SUCCESS);
        case 'p':
            strncpy(g_port, optarg, BUFSIZE - 1);
            break ;
        case '?':
            printf("%s", HELP_TEXT);
            exit(EXIT_FAILURE);
        }
    }
}

int socket_open()
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int ret;
    int sfd;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_NUMERICSERV;
    if ((ret = getaddrinfo(g_host, g_port, &hints, &result)) != 0) {
        fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(ret));
        return -1;
    }
    for (rp = result; rp != nullptr; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue ;
        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break ;
        close(sfd);
    }
    if (!rp) {
        fprintf(stderr, "Could not connect socket to any address.\n");
        return -1;
    }
    freeaddrinfo(result);
    return sfd;
}

int main(int argc, char **argv)
{
    int sfd;
    int ret;
    char buf[1000] = {};

    parse_options(argc, argv);
    sfd = socket_open();
    if (sfd == -1)
        return 1;
    printf("Connected to Matt_daemon (%s:%s)\n", g_host, g_port);

    struct pollfd fds[2];

    int timeout_ms = 100;

    fds[0].fd = 0;
    fds[0].events = POLLIN;
    fds[1].fd = sfd;
    fds[1].events = POLLIN;

    while (42) {
        ret = poll(fds, 2, timeout_ms);
        if (ret > 0) {
            for (int i = 0; i < 2; ++i) {
                if (i == 0 && (fds[i].revents & POLLIN)) {
                    // read stdin + write to socket
                    memset(buf, 0, 1000);
                    ret = read(0, buf, 999);
                    if (ret < 0)
                        perror("read()");
                    ret = write(sfd, buf, strlen(buf));
                    if (ret < 0)
                        perror("write()");
                }
                if (i == 1 && (fds[i].revents & POLLIN)) {
                    // read socket + write to stdout
                    memset(buf, 0, 1000);
                    ret = read(sfd, buf, 999);
                    if (ret == 0) {
                        printf("Socket disconnected\n");
                        close(sfd);
                        exit(0);
                    }
                    if (ret < 0)
                        perror("read()");
                    if (write(1, buf, strlen(buf)) < 0)
                        perror("write()");
                }
            }
        } else if (ret < 0) {
            perror("poll()");
        }
    }
    close(sfd);
    return 0;
}