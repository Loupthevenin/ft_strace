#include "../includes/ft_strace.h"

int	get_max_syscall(const char **syscalls)
{
	int	count;

	count = 0;
	while (syscalls[count])
		count++;
	return (count);
}

static int	compare_stats(const void *a, const void *b)
{
	t_syscall_stat	*s1;
	t_syscall_stat	*s2;

	s1 = (t_syscall_stat *)a;
	s2 = (t_syscall_stat *)b;
	return (s2->total_time_ns - s1->total_time_ns);
}

void	print_stats(t_args *args)
{
	int			i;
	long long	total_time_ns;
	int			total_calls;
	int			total_errors;
	double		sec;
	double		pct;
	int			usec_per_call;

	total_time_ns = 0;
	for (i = 0; i < args->stats_size; i++)
		total_time_ns += args->stats[i].total_time_ns;
	qsort(args->stats, args->stats_size, sizeof(t_syscall_stat), compare_stats);
	printf("%% time     seconds  usecs/call     calls    errors syscall\n");
	printf("------ ----------- ----------- --------- --------- ----------------\n");
	for (i = 0; i < args->stats_size; i++)
	{
		sec = (double)args->stats[i].total_time_ns / 1e9;
		pct = total_time_ns ? ((double)args->stats[i].total_time_ns * 100
				/ total_time_ns) : 0;
		usec_per_call = 0;
		if (args->stats[i].count > 0)
			usec_per_call = args->stats[i].total_time_ns / args->stats[i].count
				/ 1000;
		printf("%6.2f %11.6f %11d %9d %9d %s\n",
				pct,
				sec,
				usec_per_call,
				args->stats[i].count,
				args->stats[i].errors,
				args->stats[i].name);
	}
	total_calls = 0;
	total_errors = 0;
	i = 0;
	while (i < args->stats_size)
	{
		total_calls += args->stats[i].count;
		total_errors += args->stats[i].errors;
		i++;
	}
	printf("------ ----------- ----------- --------- --------- ----------------\n");
	printf("100.00 %11.6f %11s %9d %9d total\n", (double)total_time_ns / 1e9,
			"", total_calls, total_errors);
}

void	clean(t_args *args)
{
	if (args->path_bin)
		free(args->path_bin);
	if (args->stats)
		free(args->stats);
}
