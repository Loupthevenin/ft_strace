#include "../includes/ft_strace.h"

static pid_t					g_child_pid = -1;
static volatile sig_atomic_t	g_sigint_received = 0;

static void	check_perm(t_args *args)
{
	if (access(args->path_bin, F_OK) == -1)
	{
		fprintf(stderr, "%s: No such file or directory\n", args->path_bin);
		free(args->path_bin);
		exit(EXIT_FAILURE);
	}
	if (access(args->path_bin, X_OK) == -1)
	{
		fprintf(stderr, "%s: Permission denied or not executable\n",
				args->path_bin);
		free(args->path_bin);
		exit(EXIT_FAILURE);
	}
}

static char	*resolve_exec_path(const char *cmd)
{
	char	*path;
	char	*paths;
	char	*dir;
	char	fullpath[2048];

	if (strchr(cmd, '/'))
		return (strdup(cmd));
	path = getenv("PATH");
	if (!path)
		return (NULL);
	paths = strdup(path);
	dir = strtok(paths, ":");
	while (dir)
	{
		snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, cmd);
		if (access(fullpath, X_OK) == 0)
		{
			free(paths);
			return (strdup(fullpath));
		}
		dir = strtok(NULL, ":");
	}
	free(paths);
	return (NULL);
}

static t_args	parse_args(int argc, char **argv)
{
	t_args	result;
	int		i;
	char	*str;

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
	str = resolve_exec_path(argv[i]);
	if (!str)
	{
		fprintf(stderr, "%s: command not found\n", argv[i]);
		exit(EXIT_FAILURE);
	}
	result.path_bin = str;
	check_perm(&result);
	result.argv_exec = &argv[i];
	result.stats = calloc(1024, sizeof(t_syscall_stat));
	result.stats_size = 0;
	result.syscalls = NULL;
	result.max_syscall = -1;
	return (result);
}

static void	sig_handler(int sig)
{
	g_sigint_received = 1;
	if (g_child_pid > 0)
		kill(g_child_pid, sig);
}

static void	setup_signal(t_args *args)
{
	struct sigaction	sa;

	sa.sa_handler = sig_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGINT, &sa, NULL) == -1)
	{
		perror("sigaction SIGINT");
		clean(args);
		exit(EXIT_FAILURE);
	}
	if (sigaction(SIGTERM, &sa, NULL) == -1)
	{
		perror("sigaction SIGTERM");
		clean(args);
		exit(EXIT_FAILURE);
	}
	if (sigaction(SIGQUIT, &sa, NULL) == -1)
	{
		perror("sigaction SIGQUIT");
		clean(args);
		exit(EXIT_FAILURE);
	}
}

static void	exec_cmd(t_args *args)
{
	execvp(args->path_bin, args->argv_exec);
	perror("execvp");
	clean(args);
	exit(EXIT_FAILURE);
}

void	fork_command(t_args *args)
{
	pid_t	child_pid;
	int		exit_code;

	child_pid = fork();
	if (child_pid == -1)
	{
		perror("fork");
		exit(EXIT_FAILURE);
	}
	if (child_pid == 0)
		exec_cmd(args);
	else
	{
		g_child_pid = child_pid;
		setup_signal(args);
		exit_code = tracer(child_pid, args);
		if (args->enable_stats)
			print_stats(args);
		clean(args);
		exit(exit_code);
	}
}

int	main(int argc, char **argv)
{
	t_args	args;

	args = parse_args(argc, argv);
	fork_command(&args);
	clean(&args);
	return (0);
}
