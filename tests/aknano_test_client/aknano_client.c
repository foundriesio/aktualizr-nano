#include <stdlib.h>
#include <sys/random.h>
#include <time.h>

#include "aknano_platform.h"

time_t get_current_epoch()
{
    return time(NULL);
}

status_t aknano_gen_random_bytes(char *output, size_t size)
{
        getrandom(output, size, 0);
        return 0;
}
