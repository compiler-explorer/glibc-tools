/* Catch segmentation faults and print backtrace.

   Copyright (C) 1998-2021 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify it
   under the terms of the GNU Lesser General Public License as published
   by the Free Software Foundation; either version 2.1 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <common.h>
#include <ctype.h>
#include <errno.h>
#include <execinfo.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include <unistd.h>

#include "_itoa.h"
#include "intprops.h"
#include <register-dump.h>
#include <sigcontextinfo.h>

#include "config.hpp"
#include "signal-safe-trace.hpp"

#ifdef SA_SIGINFO
#define SIGCONTEXT siginfo_t *info, void *
#endif

/* Get code to possibly dump the content of all registers.  */

/* We'll use this a lot.  */
#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY(exp)            \
  ({                                       \
    decltype(exp) _rc;                     \
    do {                                   \
      _rc = (exp);                         \
    } while (_rc == -1 && errno == EINTR); \
    _rc;                                   \
  })
#endif

#define WRITE_STRING(s) TEMP_FAILURE_RETRY(write(fd, s, strlen(s)))

/* Name of the output file.  */
static const char *fname;


/* Print the signal number SIGNAL.  Either strerror or strsignal might
   call local internal functions and these in turn call far too many
   other functions and might even allocate memory which might fail.  */
static void write_strsignal(int fd, int signal)
{
    char buf[INT_BUFSIZE_BOUND(int)];
    char *ptr = _itoa_word(signal, &buf[sizeof(buf)], 10, 0);
    WRITE_STRING("signal ");
    TEMP_FAILURE_RETRY(write(fd, ptr, &buf[sizeof(buf)] - ptr));
}

const auto dump_registers = dumpRegisters();
const auto dump_memory = dumpMemory();

/* This function is called when a segmentation fault is caught.  The system
   is in an unstable state now.  This means especially that malloc() might
   not work anymore.  */
static void catch_segfault(int signal, SIGCONTEXT ctx)
{
    int fd, cnt, i;
    void **arr;
    struct sigaction sa;
    uintptr_t pc;

    /* This is the name of the file we are writing to.  If none is given
       or we cannot write to this file write to stderr.  */
    fd = 2;
    if (fname != NULL)
    {
        fd = open(fname, O_TRUNC | O_WRONLY | O_CREAT, 0666);
        if (fd == -1)
            fd = 2;
    }

    WRITE_STRING("*** ");
    write_strsignal(fd, signal);
    WRITE_STRING("\n");

#ifdef REGISTER_DUMP
    if (dump_registers)
    {
        REGISTER_DUMP;
    }
#endif

    WRITE_STRING("\n");

    do_signal_safe_trace();

#ifdef HAVE_PROC_SELF
    if (dump_memory)
    {
        /* Now the link map.  */
        int mapfd = open("/proc/self/maps", O_RDONLY);
        if (mapfd != -1)
        {
            WRITE_STRING("\nMemory map:\n\n");

            char buf[256];
            ssize_t n;

            while ((n = TEMP_FAILURE_RETRY(read(mapfd, buf, sizeof(buf)))) > 0)
                TEMP_FAILURE_RETRY(write(fd, buf, n));

            close(mapfd);
        }
    }
#endif

    /* Pass on the signal (so that a core file is produced).  */
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(signal, &sa, NULL);
    raise(signal);
}


static void __attribute__((constructor)) install_handler(void)
{
    struct sigaction sa;
    const char *sigs = getenv("SEGFAULT_SIGNALS");
    const char *name;

#ifdef SA_SIGINFO
    sa.sa_sigaction = catch_segfault;
    sa.sa_flags = SA_SIGINFO;
#else
    sa.sa_handler = (void *)catch_segfault;
    sa.sa_flags = 0;
#endif
    sigemptyset(&sa.sa_mask);
    sa.sa_flags |= SA_RESTART;

    if (useAlternativeStack())
    {
        void *stack_mem = malloc(2 * SIGSTKSZ);
        stack_t ss;

        if (stack_mem != NULL)
        {
            ss.ss_sp = stack_mem;
            ss.ss_flags = 0;
            ss.ss_size = 2 * SIGSTKSZ;

            if (sigaltstack(&ss, NULL) == 0)
                sa.sa_flags |= SA_ONSTACK;
        }
    }

    if (sigs == NULL)
        sigaction(SIGSEGV, &sa, NULL);
    else if (sigs[0] == '\0')
        /* Do not do anything.  */
        return;
    else
    {
        const char *where;
        int all = strcasecmp(sigs, "all") == 0;

#define INSTALL_FOR_SIG(sig, name)                                                                             \
    where = strcasestr(sigs, name);                                                                            \
    if (all || (where != NULL && (where == sigs || !isalnum(where[-1])) && !isalnum(where[sizeof(name) - 1]))) \
        sigaction(sig, &sa, NULL);

        INSTALL_FOR_SIG(SIGSEGV, "segv");
        INSTALL_FOR_SIG(SIGILL, "ill");
#ifdef SIGBUS
        INSTALL_FOR_SIG(SIGBUS, "bus");
#endif
#ifdef SIGSTKFLT
        INSTALL_FOR_SIG(SIGSTKFLT, "stkflt");
#endif
        INSTALL_FOR_SIG(SIGABRT, "abrt");
        INSTALL_FOR_SIG(SIGFPE, "fpe");
    }

    /* Preserve the output file name if there is any given.  */
    name = getenv("SEGFAULT_OUTPUT_NAME");
    if (name != NULL && name[0] != '\0')
    {
        int ret = access(name, R_OK | W_OK);

        if (ret == 0 || (ret == -1 && errno == ENOENT))
            fname = strdup(name);
    }
}
