#ifdef __linux
#include <sys/stat.h>

int stat(const char *file_name, struct stat *buf) 
{
    return __xstat( 3, file_name, buf );
}
#endif

