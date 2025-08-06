#include "../includes/ft_strace.h"

int	get_max_syscall(const char **syscalls)
{
	int	count;

	count = 0;
	while (syscalls[count])
		count++;
	return (count);
}

void	print_stats(t_args *args)
{
	printf("\n%-30s %s\n", "Syscall", "Calls");
	printf("%-30s %s\n", "-----------------------", "-----");
	for (int i = 0; i < args->stats_size; i++)
	{
		printf("%-30s %d\n", args->stats[i].name, args->stats[i].count);
	}
}

void	clean(t_args *args)
{
	if (args->path_bin)
		free(args->path_bin);
	if (args->stats)
		free(args->stats);
}
