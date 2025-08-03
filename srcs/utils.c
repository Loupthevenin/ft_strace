#include "../includes/ft_strace.h"

void	clean(t_args *args)
{
	if (args->path_bin)
		free(args->path_bin);
}
