#ifndef FT_STRACE_H
# define FT_STRACE_H

# include <signal.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/ptrace.h>
# include <sys/types.h>
# include <unistd.h>

typedef struct s_args
{
	char	*path_bin;
	char	**argv_exec;
	int		enable_stats;
}			t_args;

// Main
void		tracer(pid_t child_pid, t_args *args);

// Utils
void		clean(t_args *args);

#endif
