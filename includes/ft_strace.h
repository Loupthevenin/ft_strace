#ifndef FT_STRACE_H
# define FT_STRACE_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>

typedef struct s_args
{
	char	*path_bin;
	char	**argv_exec;
	int		enable_stats;
}			t_args;

// Main

// Utils
void		clean(t_args *args);

#endif
