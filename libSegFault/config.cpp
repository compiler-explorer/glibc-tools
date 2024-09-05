#include <cstdlib>
#include <cstring>

const char *getTracerProgram()
{
    static const char *emptystr = "";
    if (const char *value = getenv("LIBSEGFAULT_TRACER"))
    {
        return value;
    }

    return emptystr;
}

bool isDebugMode()
{
    if (const char *value = getenv("LIBSEGFAULT_DEBUG"))
    {
        if (strcmp(value, "1") == 0)
            return true;
    }

    return false;
}
