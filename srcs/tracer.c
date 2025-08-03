#include "../includes/ft_strace.h"

static void	loop_trace(pid_t child_pid, int *status)
{
	int						in_syscall;
	struct user_regs_struct	regs;
	struct iovec			iov;

	in_syscall = 0;
	while (1)
	{
		// Intercepte un syscall;
		if (ptrace(PTRACE_SYSCALL, child_pid, NULL, NULL) == -1)
			break ;
		// Attend que ca s'arrete;
		waitpid(child_pid, status, 0);
		// execution terminé;
		if (WIFEXITED(*status))
			break ;
		if (WIFSTOPPED(*status) && (WSTOPSIG(*status) & 0x80))
		{
			// Lecture registres;
			iov.iov_base = &regs;
			iov.iov_len = sizeof(regs);
			if (ptrace(PTRACE_GETREGSET, child_pid, (void *)NT_PRSTATUS,
					&iov) == -1)
			{
				perror("ptrace GETREGSET");
				return ;
			}
			if (!in_syscall)
			{
				printf("%lld(", regs.orig_rax);
				fflush(stdout);
				in_syscall = 1;
			}
			else
			{
				printf(") = %lld", regs.rax);
				in_syscall = 0;
			}
		}
	}
}

void	tracer(pid_t child_pid, t_args *args)
{
	int	status;

	(void)args;
	// Attendre le child
	waitpid(child_pid, &status, 0);
	if (WIFEXITED(status))
		return ;
	// Activer ptrace
	if (ptrace(PTRACE_SETOPTIONS, child_pid, 0, PTRACE_O_TRACESYSGOOD) == -1)
	{
		perror("ptrace SETOPTIONS");
		return ;
	}
	loop_trace(child_pid, &status);
}
