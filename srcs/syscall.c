#include "../includes/ft_strace.h"

static const char	*get_syscall_name(const char **syscalls, int max, long id)
{
	if (id < max && syscalls[id])
		return (syscalls[id]);
	return ("unknown");
}

static void	print_syscall_args_64(struct user_regs_struct *regs)
{
	printf("%#llx, %#llx, %#llx, %#llx, %#llx, %#llx", regs->rdi, regs->rsi,
			regs->rdx, regs->r10, regs->r8, regs->r9);
}

static void	print_syscall_args_32(struct user_regs_struct *regs)
{
	printf("%#llx, %#llx, %#llx, %#llx, %#llx, %#llx", regs->rbx, regs->rcx,
			regs->rdx, regs->rsi, regs->rdi, regs->rbp);
}

static void	detect_archi(t_args *args, pid_t pid)
{
	if (args->is_32 != -1)
		return ;
	if (is_32_bit(pid))
	{
		args->syscalls = get_syscall_names_32();
		args->is_32 = 1;
	}
	else
	{
		args->syscalls = get_syscall_names_64();
		args->is_32 = 0;
	}
	args->max_syscall = get_max_syscall(args->syscalls);
}

void	handle_syscall(pid_t pid, int *in_syscall, t_args *args)
{
	struct user_regs_struct	regs;
	struct iovec			iov;
	const char				*name;
	struct timespec			end;
	int						i;
	long long				duration_ns;
	int						found;
	int						syscall_index;

	detect_archi(args, pid);
	syscall_index = -1;
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
		name = get_syscall_name(args->syscalls, args->max_syscall,
				regs.orig_rax);
		args->current_syscall_num = regs.orig_rax;
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
				syscall_index = args->stats_size;
				args->stats[args->stats_size].name = name;
				args->stats[args->stats_size].count = 1;
				args->stats[syscall_index].errors = 0;
				args->stats[args->stats_size].total_time_ns = 0;
				args->stats_size++;
			}
		}
		else
		{
			printf("%s(", name);
			if (args->is_32)
				print_syscall_args_32(&regs);
			else
				print_syscall_args_64(&regs);
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
					break ;
				}
				i++;
			}
		}
		else
			printf(") = %lld\n", regs.rax);
		*in_syscall = 0;
	}
}
