#ifndef TINTIN_REPORTER_H
# define TINTIN_REPORTER_H

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cerrno>
#include <libgen.h>

#include <string>

class Tintin_reporter
{
    FILE *f = nullptr;

    const char *get_time();
    
public:
    Tintin_reporter();
    ~Tintin_reporter();
    Tintin_reporter(const Tintin_reporter &ttr);
    Tintin_reporter& operator=(const Tintin_reporter &ttr);

    bool init(const char *filename = DEFAULT_LOG_PATH);
    void clear();
    void info(const char *format, ...) __attribute__ ((format (printf, 2, 3)));
    void log(const char *format, ...) __attribute__ ((format (printf, 2, 3)));
    void error(const char *format, ...) __attribute__ ((format (printf, 2, 3)));
    void perror(const char *msg);
};

extern Tintin_reporter ttr;

#endif