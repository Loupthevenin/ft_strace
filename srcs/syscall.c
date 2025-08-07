#include "../includes/ft_strace.h"

static const char	*get_syscall_name(const char **syscalls, int max, long id)
{
	if (id < max && syscalls[id])
		return (syscalls[id]);
	return ("unknown");
}

static void	print_syscall_args(t_user_regs *regs)
{
#if IS_32_BIT
	printf("%#x, %#x, %#x, %#x, %#x, %#x",
			regs->ebx,
			regs->ecx,
			regs->edx,
			regs->esi,
			regs->edi,
			regs->ebp);
#else
	printf("%#llx, %#llx, %#llx, %#llx, %#llx, %#llx", regs->rdi, regs->rsi,
			regs->rdx, regs->r10, regs->r8, regs->r9);
#endif
}

static int	get_regs(pid_t pid, t_user_regs *regs)
{
	struct iovec	iov;

	iov.iov_base = regs;
	iov.iov_len = sizeof(*regs);
	// Récupère les registres -> GETREGSET
	if (ptrace(PTRACE_GETREGSET, pid, (void *)NT_PRSTATUS, &iov) == -1)
	{
		perror("ptrace GETREGSET");
		return (-1);
	}
	return (0);
}

static t_syscall_stat	*get_or_create_stat(t_args *args, const char *name)
{
	int				i;
	t_syscall_stat	*stat;

	i = 0;
	while (i < args->stats_size)
	{
		if (strcmp(args->stats[i].name, name) == 0)
			return (&args->stats[i]);
		i++;
	}
	// Max stats;
	if (args->stats_size < 1024)
	{
		stat = &args->stats[args->stats_size++];
		stat->name = name;
		stat->count = 0;
		stat->errors = 0;
		stat->total_time_ns = 0;
		return (stat);
	}
	// Overflow;
	return (NULL);
}

static void	handle_enter_syscall(t_user_regs *regs, t_args *args)
{
	const char		*name;
	t_syscall_stat	*stat;

	name = get_syscall_name(args->syscalls, args->max_syscall, SYSCALL_NUM);
	args->current_syscall_num = SYSCALL_NUM;
	clock_gettime(CLOCK_MONOTONIC, &args->current_start_time);
	if (args->enable_stats)
	{
		stat = get_or_create_stat(args, name);
		if (stat)
			stat->count++;
	}
	else
	{
		printf("%s(", name);
		print_syscall_args(regs);
		fflush(stdout);
	}
}

static void	handle_exit_syscall(t_user_regs *regs, t_args *args)
{
	struct timespec	end;
	long long		duration_ns;
	long			ret;
	const char		*name;
	t_syscall_stat	*stat;

	clock_gettime(CLOCK_MONOTONIC, &end);
	if (args->enable_stats)
	{
		duration_ns = (end.tv_sec - args->current_start_time.tv_sec)
			* 1000000000LL + (end.tv_nsec - args->current_start_time.tv_nsec);
		name = get_syscall_name(args->syscalls, args->max_syscall,
				args->current_syscall_num);
		stat = get_or_create_stat(args, name);
		if (stat)
		{
			stat->total_time_ns += duration_ns;
			ret = (long)SYSCALL_RET;
			if (ret < 0)
				stat->errors++;
		}
	}
	else
		printf(") = %lld\n", (long long)SYSCALL_RET);
}

void	handle_syscall(pid_t pid, int *in_syscall, t_args *args)
{
	t_user_regs	regs;

	if (get_regs(pid, &regs) == -1)
		return ;
	if (!*in_syscall)
		handle_enter_syscall(&regs, args);
	else
		handle_exit_syscall(&regs, args);
	*in_syscall = !(*in_syscall);
}
