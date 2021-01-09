#ifndef SERVER_H
# define SERVER_H

class Server
{
    int sfd; // server socket fd

    bool init_tcp();
    bool init_udp();
public:
    Server();
    ~Server();
    Server(const Server &s);
    Server& operator=(const Server &s);
    
    void daemon();
    bool init();
    void clear();
    bool start();
};


#endif