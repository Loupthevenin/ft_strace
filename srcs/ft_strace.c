#include "../includes/ft_strace.h"

static t_args	parse_args(int argc, char **argv)
{
	t_args	result;
	int		i;

	result.enable_stats = 0;
	i = 1;
	while (i < argc && argv[i][0] == '-')
	{
		if (strcmp(argv[i], "-c") == 0)
			result.enable_stats = 1;
		else
		{
			fprintf(stderr, "Unknown option: %s\n", argv[i]);
			exit(EXIT_FAILURE);
		}
		i++;
	}
	if (i >= argc)
	{
		fprintf(stderr, "Usage: %s <command> [args...]\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	result.path_bin = strdup(argv[1]);
	result.argv_exec = &argv[i];
	return (result);
}

static void	exec_cmd(t_args *args)
{
	if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) != -1)
	{
		perror("ptrace(TRACEME)");
		clean(args);
		exit(EXIT_FAILURE);
	}
	// Met en pause le child (il attend le tracer);
	kill(getpid(), SIGSTOP);
	execvp(args->path_bin, args->argv_exec);
	perror("execvp");
	clean(args);
	exit(EXIT_FAILURE);
}

void	fork_command(t_args *args)
{
	pid_t	child_pid;

	child_pid = fork();
	if (child_pid == -1)
	{
		perror("fork");
		exit(EXIT_FAILURE);
	}
	if (child_pid == 0)
		exec_cmd(args);
	else
		tracer(child_pid, args);
}

int	main(int argc, char **argv)
{
	t_args	args;

	args = parse_args(argc, argv);
	fork_command(&args);
	clean(&args);
	return (0);
}
