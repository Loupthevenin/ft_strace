#include "../includes/ft_strace.h"

int	get_max_syscall(const char **syscalls)
{
	int	count;

	count = 0;
	while (syscalls[count])
		count++;
	return (count);
}

void	clean(t_args *args)
{
	if (args->path_bin)
		free(args->path_bin);
}
