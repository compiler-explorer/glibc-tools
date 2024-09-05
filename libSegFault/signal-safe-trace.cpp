#include "signal-safe-trace.hpp"

#include <cpptrace/cpptrace.hpp>

#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <string_view>

#include <csignal>
#include <cstring>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace std::literals;

struct pipe_t
{
    union
    {
        struct
        {
            int read_end;
            int write_end;
        };
        int data[2];
    };
};
static_assert(sizeof(pipe_t) == 2 * sizeof(int), "Unexpected struct packing");

std::string tracer_program = "tracer";

void warmup_cpptrace()
{
    cpptrace::frame_ptr buffer[10];
    std::size_t count = cpptrace::safe_generate_raw_trace(buffer, 10);
    cpptrace::safe_object_frame frame;
    cpptrace::get_safe_object_frame(buffer[0], &frame);
}

[[gnu::constructor]] void init()
{
    warmup_cpptrace();
    if (const char *value = getenv("LIBSEGFAULT_TRACER"))
    {
        tracer_program = value;
    }
}

constexpr auto fork_failure_message = "fork() failed, unable to collect trace\n"sv;
auto exec_failure_message = "exec(signal_tracer) failed: Make sure the signal_tracer executable is in the current "
                            "working directory and the binary's permissions are correct.\n"sv;

void do_signal_safe_trace()
{
    cpptrace::frame_ptr buffer[100];
    std::size_t count = cpptrace::safe_generate_raw_trace(buffer, 100);

    pipe_t input_pipe;
    std::ignore = pipe(input_pipe.data);

    const pid_t pid = fork();
    if (pid == -1)
    {
        std::ignore = write(STDERR_FILENO, fork_failure_message.data(), fork_failure_message.size());
        return;
    }

    if (pid == 0)
    { // child
        dup2(input_pipe.read_end, STDIN_FILENO);
        close(input_pipe.read_end);
        close(input_pipe.write_end);
        execl(tracer_program.c_str(), tracer_program.c_str(), nullptr);
        std::ignore = write(STDERR_FILENO, exec_failure_message.data(), exec_failure_message.size());
        _exit(1);
    }

    for (std::size_t i = 0; i < count; i++)
    {
        cpptrace::safe_object_frame frame;
        cpptrace::get_safe_object_frame(buffer[i], &frame);
        std::ignore = write(input_pipe.write_end, &frame, sizeof(frame));
    }

    close(input_pipe.read_end);
    close(input_pipe.write_end);
    waitpid(pid, nullptr, 0);
}
