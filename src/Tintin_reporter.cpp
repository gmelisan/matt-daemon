#include "Matt_daemon.h"
#include "Tintin_reporter.h"

const char *Tintin_reporter::get_time()
{
    static char buffer[100];

    memset(buffer, 0, 100);
    time_t t = time(nullptr);
    struct tm *ti = localtime(&t);
    strftime(buffer, 100, "%d/%m/%Y-%H:%M:%S", ti);
    return buffer;
}

Tintin_reporter::Tintin_reporter()
{}
Tintin_reporter::~Tintin_reporter()
{
    clear();
}
Tintin_reporter::Tintin_reporter(const Tintin_reporter &ttr)
{}
Tintin_reporter& Tintin_reporter::operator=(const Tintin_reporter &ttr)
{
    return *this;
}

bool Tintin_reporter::init(const char *filename)
{
    system("mkdir -p " DEFAULT_LOG_DIR);
    f = fopen(filename, "a+");
    if (!f) {
        fprintf(stderr, "Can't open log file: %s\n", strerror(errno));
        return false;
    }
    setvbuf(f, nullptr, _IONBF, 0);
    return true;
}
bool Tintin_reporter::clear()
{
    if (f)
        fclose(f);
    f = nullptr;
}
void Tintin_reporter::info(const char *format, ...)
{
    va_list ap;
    if (f) {
        va_start(ap, format);
        fprintf(f, "[%s] [ INFO ] - %s: ", get_time(), PROGRAM_NAME);
        vfprintf(f, format, ap);
        fprintf(f, "\n");
        va_end(ap);
    }
}
void Tintin_reporter::log(const char *format, ...)
{
    va_list ap;
    if (f) {
        va_start(ap, format);
        fprintf(f, "[%s] [ LOG ] - %s: ", get_time(), PROGRAM_NAME);
        vfprintf(f, format, ap);
        fprintf(f, "\n");
        va_end(ap);
    }
}
void Tintin_reporter::error(const char *format, ...)
{
    va_list ap;
    if (f) {
        va_start(ap, format);
        fprintf(f, "[%s] [ ERROR ] - %s: ", get_time(), PROGRAM_NAME);
        vfprintf(f, format, ap);
        fprintf(f, "\n");
        va_end(ap);
    }
}

void Tintin_reporter::perror(const char *msg)
{
    error("%s: %s", format, strerror(errno));
}