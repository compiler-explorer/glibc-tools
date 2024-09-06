#include "signal-safe-trace.hpp"
#include "config.hpp"

#include <cpptrace/cpptrace.hpp>

#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include <csignal>
#include <cstring>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include <errno.h>

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

void warmup_cpptrace()
{
    cpptrace::frame_ptr buffer[10];
    std::size_t count = cpptrace::safe_generate_raw_trace(buffer, 10);
    cpptrace::safe_object_frame frame;
    cpptrace::get_safe_object_frame(buffer[0], &frame);
}

std::string tracer_program = [] {
    const char *value = getenv("LIBSEGFAULT_TRACER");
    return value ? value : "";
} ();

std::vector<std::string> tracer_env;
std::vector<char*> tracer_env_buffer;

extern char **environ;

[[gnu::constructor]] void init()
{
    warmup_cpptrace();
    for (char **s = environ; *s; s++) {
        std::string var = *s;
        if(!var.starts_with("LD_PRELOAD=")) {
            tracer_env.emplace_back(std::move(var));
            tracer_env_buffer.emplace_back(tracer_env.back().data());
        }
        printf("%s\n", *s);
    }
    tracer_env_buffer.emplace_back(nullptr);
    // printf("init() setting tracer_program: %s\n", tracer_program.c_str());
    fflush(stdout);
    sleep(5);
}

const auto fork_failure_message = "fork() failed, unable to collect trace\n"sv;
const auto no_tracer_message = "exec(signal_tracer) failed: Please supply the environment variable "
                                "LIBSEGFAULT_TRACER.\n"sv;

const auto exec_failure_message = "exec(signal_tracer) failed: Make sure the signal_tracer exists and the executable "
                                "permissions are correct.\n"sv;

void do_signal_safe_trace()
{
    cpptrace::frame_ptr buffer[100];
    std::size_t count = cpptrace::safe_generate_raw_trace(buffer, 100);

    pipe_t input_pipe;
    std::ignore = pipe(input_pipe.data);

    fprintf(stderr, "pre-fork: tracer_program: %s\n", tracer_program.c_str()); fflush(stderr);

    const pid_t pid = fork();
    if (pid == -1)
    {
        std::ignore = write(STDERR_FILENO, fork_failure_message.data(), fork_failure_message.size());
        return;
    }

    if (pid == 0)
    { // child
        fprintf(stderr, "post-fork: tracer_program: %s\n", tracer_program.c_str()); fflush(stderr);
        dup2(input_pipe.read_end, STDIN_FILENO);
        close(input_pipe.read_end);
        close(input_pipe.write_end);

        bool is_debug = false; isDebugMode();
        // const char *tracer_program = getTracerProgram();
        if (tracer_program.size() == 0) // (strlen(tracer_program) == 0)
        {
            std::ignore = write(STDERR_FILENO, no_tracer_message.data(), no_tracer_message.size());
        }
        else
        {
            execle(tracer_program.c_str(), tracer_program.c_str(), nullptr, tracer_env_buffer.data());

            if (is_debug)
            {
                auto errcode = errno;
                fprintf(stderr, "errno: %d\n", errcode);
                fprintf(stderr, "tried to execute: %s\n", tracer_program.c_str());
            }

            // https://linux.die.net/man/3/execl - execl() only returns when an error has occured
            //  otherwise this basically exits out of this code
            // so this write() is only executed when a failure has occured

            std::ignore = write(STDERR_FILENO, exec_failure_message.data(), exec_failure_message.size());
        }
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
