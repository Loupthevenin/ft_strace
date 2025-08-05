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

static void	loop_trace(pid_t child_pid, int *status)
{
	int						in_syscall;
	struct user_regs_struct	regs;
	struct iovec			iov;
	const char				**syscalls;
	int						max_syscall;
	const char				*name;

	in_syscall = 0;
	if (is_32_bit(child_pid))
		syscalls = get_syscall_names_32();
	else
		syscalls = get_syscall_names_64();
	max_syscall = get_max_syscall(syscalls);
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
				name = get_syscall_name(syscalls, max_syscall, regs.orig_rax);
				printf("%s(%#llx, %#llx, %#llx", name, regs.rdi, regs.rsi,
						regs.rdx);
				fflush(stdout);
				in_syscall = 1;
			}
			else
			{
				printf(") = %lld\n", regs.rax);
				in_syscall = 0;
			}
		}
	}
}

void	tracer(pid_t child_pid, t_args *args)
{
	int	status;

	// ON attache au child
	if (ptrace(PTRACE_SEIZE, child_pid, NULL, PTRACE_O_TRACESYSGOOD) == -1)
	{
		perror("ptrace SEIZE");
		clean(args);
		exit(EXIT_FAILURE);
	}
	// On met en pause pour le tracing
	if (ptrace(PTRACE_INTERRUPT, child_pid, NULL, NULL) == -1)
	{
		perror("ptrace INERRUPT");
		clean(args);
		exit(EXIT_FAILURE);
	}
	// Attendre que le process soit stop
	if (waitpid(child_pid, &status, 0) == -1)
	{
		perror("waitpid");
		clean(args);
		exit(EXIT_FAILURE);
	}
	loop_trace(child_pid, &status);
}
