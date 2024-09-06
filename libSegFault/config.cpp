#include <cstdlib>
#include <cstring>

bool isDebugMode()
{
    if (const char *value = getenv("LIBSEGFAULT_DEBUG"))
    {
        if (strcmp(value, "1") == 0)
            return true;
    }

    return false;
}

bool useAlternativeStack()
{
    return getenv("SEGFAULT_USE_ALTSTACK") != 0;
}

bool dumpRegisters()
{
    if (const char *value = getenv("LIBSEGFAULT_REGISTERS"))
    {
        if (strcmp(value, "1") == 0)
            return true;
    }

    return false;
}

bool dumpMemory()
{
    if (const char *value = getenv("LIBSEGFAULT_MEMORY"))
    {
        if (strcmp(value, "1") == 0)
            return true;
    }

    return false;
}
