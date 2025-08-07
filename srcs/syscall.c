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

void	handle_syscall(pid_t pid, int *in_syscall, t_args *args)
{
	t_user_regs		regs;
	struct iovec	iov;
	const char		*name;
	struct timespec	end;
	int				i;
	long long		duration_ns;
	long			ret;
	int				found;

	iov.iov_base = &regs;
	iov.iov_len = sizeof(regs);
	// Récupère les registres -> GETREGSET
	if (ptrace(PTRACE_GETREGSET, pid, (void *)NT_PRSTATUS, &iov) == -1)
	{
		perror("ptrace GETREGSET");
		return ;
	}
	// Si on entre dans le syscall
	if (!*in_syscall)
	{
		// Récupère le nom
		name = get_syscall_name(args->syscalls, args->max_syscall, SYSCALL_NUM);
		args->current_syscall_num = SYSCALL_NUM;
		clock_gettime(CLOCK_MONOTONIC, &args->current_start_time);
		if (args->enable_stats)
		{
			found = 0;
			i = 0;
			while (i < args->stats_size)
			{
				if (strcmp(args->stats[i].name, name) == 0)
				{
					args->stats[i].count++;
					found = 1;
					break ;
				}
				i++;
			}
			if (!found)
			{
				args->stats[args->stats_size].name = name;
				args->stats[args->stats_size].count = 1;
				args->stats[args->stats_size].errors = 0;
				args->stats[args->stats_size].total_time_ns = 0;
				args->stats_size++;
			}
		}
		else
		{
			printf("%s(", name);
			print_syscall_args(&regs);
			fflush(stdout);
		}
		*in_syscall = 1;
	}
	else
	{
		// Sortie de syscall : code de retour
		clock_gettime(CLOCK_MONOTONIC, &end);
		if (args->enable_stats)
		{
			duration_ns = (end.tv_sec - args->current_start_time.tv_sec)
				* 1000000000LL + (end.tv_nsec
					- args->current_start_time.tv_nsec);
			name = get_syscall_name(args->syscalls, args->max_syscall,
					args->current_syscall_num);
			i = 0;
			while (i < args->stats_size)
			{
				if (strcmp(args->stats[i].name, name) == 0)
				{
					args->stats[i].total_time_ns += duration_ns;
					ret = (long)SYSCALL_RET;
					if (ret < 0)
						args->stats[i].errors++;
					break ;
				}
				i++;
			}
		}
		else
			printf(") = %lld\n", (long long)SYSCALL_RET);
		*in_syscall = 0;
	}
}
