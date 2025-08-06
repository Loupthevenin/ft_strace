#include "../includes/ft_strace.h"

static const char	*get_syscall_name(const char **syscalls, int max, long id)
{
	if (id < max && syscalls[id])
		return (syscalls[id]);
	return ("unknown");
}

static int	is_32_bit(pid_t pid)
{
	char			path[128];
	int				fd;
	unsigned char	elf_header[5];

	snprintf(path, sizeof(path), "/proc/%d/exe", pid);
	// Lecture binaire;
	fd = open(path, O_RDONLY);
	if (fd == -1)
		return (-1);
	// Lit les 5 premiers octets du fichier ELF;
	if (read(fd, elf_header, 5) != 5)
	{
		close(fd);
		return (-1);
	}
	close(fd);
	// Verfie si c'est bien signature ELF;
	if (elf_header[0] != 0x7f || elf_header[1] != 'E' || elf_header[2] != 'L'
		|| elf_header[3] != 'F')
		return (-1);
	// Le 4 octet indique le format de classe 1 -> 32, 2 -> 64;
	return (elf_header[4] == 1);
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

static void	handle_syscall(pid_t pid, int *in_syscall, int is_32,
		const char **syscalls, int max_syscall, t_args *args)
{
	struct user_regs_structregs;
	struct ioveciov;
	const char		*name;
	struct timespec	end;
	int				i;
	long long		duration_ns;

	intfound;
	intsyscall_index = -1;
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
		name = get_syscall_name(syscalls, max_syscall, regs.orig_rax);
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
			if (is_32)
				print_syscall_args_32(&regs);
			else
				print_syscall_args_64(&regs);
			fflush(stdout);
			args->current_syscall_num = regs.orig_rax;
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
			name = get_syscall_name(syscalls, max_syscall,
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
	int			in_syscall;
	int			is_32;
	const char	**syscalls;
	int			max_syscall;
	int			sig;

	in_syscall = 0;
	is_32 = -1;
	if (is_32_bit(child_pid))
	{
		syscalls = get_syscall_names_32();
		is_32 = 1;
	}
	else
	{
		syscalls = get_syscall_names_64();
		is_32 = 0;
	}
	max_syscall = get_max_syscall(syscalls);
	while (1)
	{
		// Intercepte un syscall;
		if (ptrace(PTRACE_SYSCALL, child_pid, NULL, NULL) == -1)
		{
			perror("ptrace SYSCALL");
			break ;
		}
		// Attend que ca s'arrete;
		if (waitpid(child_pid, status, 0) == -1)
		{
			perror("waitpid");
			break ;
		}
		// execution terminé;
		if (WIFEXITED(*status))
			return (WEXITSTATUS(*status));
		if (WIFSTOPPED(*status))
		{
			sig = WSTOPSIG(*status);
			// Si c'est un syscall
			if (sig == (SIGTRAP | 0x80))
				handle_syscall(child_pid, &in_syscall, is_32, syscalls,
						max_syscall, args);
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
				if (waitpid(child_pid, status, 0) == -1)
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
	if (waitpid(child_pid, &status, 0) == -1)
	{
		perror("waitpid");
		return (EXIT_FAILURE);
	}
	return (loop_trace(child_pid, &status, args));
}
