#ifndef FT_STRACE_H
# define FT_STRACE_H

# include "syscall_names.h"
# include <elf.h>
# include <fcntl.h>
# include <signal.h>
# include <stdint.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/ptrace.h>
# include <sys/types.h>
# include <sys/uio.h>
# include <sys/user.h>
# include <sys/wait.h>
# include <unistd.h>

typedef struct s_args
{
	char						*path_bin;
	char						**argv_exec;
	int							enable_stats;
}								t_args;

static volatile sig_atomic_t	g_sigint_received;

// Main
int								tracer(pid_t child_pid, t_args *args);

// Utils
int								get_max_syscall(const char **syscalls);
void							print_stats(void);
void							clean(t_args *args);

#endif
