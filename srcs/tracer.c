#include "../includes/ft_strace.h"

static void	handle_signal(pid_t pid, t_args *args)
{
	siginfo_t	siginfo;

	// Récupère les infos sur le signal
	if (ptrace(PTRACE_GETSIGINFO, pid, NULL, &siginfo) == -1)
	{
		perror("ptrace GETSIGINFO");
		return ;
	}
	if (!args->enable_stats)
		printf("--- %s {si_signo=%d, si_code=%d, si_pid=%d, si_uid=%d} ---\n",
				strsignal(siginfo.si_signo),
				siginfo.si_signo,
				siginfo.si_code,
				siginfo.si_pid,
				siginfo.si_uid);
}

static int	loop_trace(pid_t child_pid, int *status, t_args *args)
{
	int	in_syscall;
	int	sig;

	in_syscall = 0;
	while (1)
	{
		// Intercepte un syscall;
		if (ptrace(PTRACE_SYSCALL, child_pid, NULL, NULL) == -1)
		{
			perror("ptrace SYSCALL");
			break ;
		}
		// Attend que ca s'arrete;
		if (safe_waitpid(child_pid, status) == -1)
		{
			perror("waitpid");
			break ;
		}
		// execution terminé;
		if (WIFEXITED(*status))
		{
			if (!args->enable_stats)
				printf("\n");
			return (WEXITSTATUS(*status));
		}
		if (WIFSTOPPED(*status))
		{
			sig = WSTOPSIG(*status);
			// Si c'est un syscall
			if (sig == (SIGTRAP | 0x80))
				handle_syscall(child_pid, &in_syscall, args);
			// Autre type -> signal
			else
			{
				handle_signal(child_pid, args);
				// Reprend avec le signal
				if (ptrace(PTRACE_SYSCALL, child_pid, NULL, sig) == -1)
				{
					perror("ptrace SYSCALL with signal");
					break ;
				}
				// Attend le signal suivant pour vérifier si le processus est mort
				if (safe_waitpid(child_pid, status) == -1)
				{
					perror("waitpid after signal");
					break ;
				}
				// Si le processus est terminé à cause du signal,
				if (WIFSIGNALED(*status))
				{
					sig = WTERMSIG(*status);
					fprintf(stderr, "+++ killed by signal %d (%s) +++\n", sig,
							strsignal(sig));
					return (128 + sig);
				}
				// Sinon, continuer la boucle
				continue ;
			}
		}
	}
	return (EXIT_FAILURE);
}

int	tracer(pid_t child_pid, t_args *args)
{
	int	status;

	// ON attache au child
	if (ptrace(PTRACE_SEIZE, child_pid, NULL, PTRACE_O_TRACESYSGOOD) == -1)
	{
		perror("ptrace SEIZE");
		return (EXIT_FAILURE);
	}
	// On met en pause pour le tracing
	if (ptrace(PTRACE_INTERRUPT, child_pid, NULL, NULL) == -1)
	{
		perror("ptrace INERRUPT");
		return (EXIT_FAILURE);
	}
	// Attendre que le process soit stop
	if (safe_waitpid(child_pid, &status) == -1)
	{
		perror("waitpid");
		return (EXIT_FAILURE);
	}
	return (loop_trace(child_pid, &status, args));
}
