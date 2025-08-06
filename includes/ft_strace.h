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
# include <time.h>
# include <unistd.h>

typedef struct s_syscall_stat
{
	const char					*name;
	int							count;
	int							errors;
	long long					total_time_ns;
}								t_syscall_stat;

typedef struct s_args
{
	char						*path_bin;
	char						**argv_exec;
	int							enable_stats;
	t_syscall_stat				*stats;
	int							stats_size;
	long long					current_syscall_num;
	struct timespec				stats_start_time;
	struct timespec				current_start_time;
}								t_args;

static volatile sig_atomic_t	g_sigint_received;

// Main
int								tracer(pid_t child_pid, t_args *args);
void							handle_syscall(pid_t pid, int *in_syscall,
									int is_32, const char **syscalls,
									int max_syscall, t_args *args);

// Utils
int								get_max_syscall(const char **syscalls);
void							print_stats(t_args *args);
int								is_32_bit(pid_t pid);
void							clean(t_args *args);

#endif
