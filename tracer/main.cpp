#include <unistd.h>

#include <cstdio>
#include <iostream>

#include <cpptrace/cpptrace.hpp>

int main(int argc, char **argv)
{
    if (argc > 1)
    {
        std::cout << "tracer program part of glibc-tools and libSegFault\n";
        return 0;
    }

    cpptrace::object_trace trace;
    while (true)
    {
        cpptrace::safe_object_frame frame;
        std::size_t res = fread(&frame, sizeof(frame), 1, stdin);
        if (res == 0)
        {
            break;
        }
        else if (res != 1)
        {
            std::cerr << "Oops, size mismatch " << res << " " << sizeof(frame) << "\n";
            break;
        }
        else
        {
            trace.frames.push_back(frame.resolve());
        }
    }
    auto resolved = trace.resolve();
    // while(!resolved.empty() && resolved.frames.front().filename.find("libSegFault/") != std::string::npos) {
    //     resolved.frames.erase(resolved.frames.begin());
    // }
    resolved.print();
}
