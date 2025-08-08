#ifndef FT_STRACE_H
# define FT_STRACE_H

# include "syscall_names.h"
# include <elf.h>
# include <fcntl.h>
# include <signal.h>
# include <stdint.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/ptrace.h>
# include <sys/types.h>
# include <sys/uio.h>
# include <sys/wait.h>
# include <time.h>
# include <unistd.h>

# ifdef IS_32_BIT

// Représentation des registres pour un processus 32 bits.
typedef struct s_user_regs_32
{
	uint32_t ebx, ecx, edx, esi, edi, ebp, eax;
	uint16_t ds, __ds, es, __es, fs, __fs, gs, __gs;
	uint32_t orig_eax, eip;
	uint16_t cs, __cs;
	uint32_t eflags, esp;
	uint16_t ss, __ss;
}								t_user_regs;

#  define SYSCALL_NUM regs->orig_eax
#  define SYSCALL_RET regs->eax

# else
#  include <sys/user.h>

typedef struct user_regs_struct	t_user_regs;

#  define SYSCALL_NUM regs->orig_rax // Numéro de syscall
#  define SYSCALL_RET regs->rax      // Valeur de retour du syscall

# endif

typedef struct s_syscall_stat
{
	const char *name;        // Nom du syscall;
	int count;               // Nombre total d'appel;
	int errors;              // Nombre d'appel échoué;
	long long total_time_ns; // temps cumulé passé dans syscall;
}								t_syscall_stat;

typedef struct s_args
{
	char						*path_bin;
	char						**argv_exec;
	int enable_stats; // -c
	const char					**syscalls;
	int							max_syscall;
	t_syscall_stat				*stats;
	int							stats_size;
	long long					current_syscall_num;
	struct timespec				stats_start_time;
	struct timespec				current_start_time;
}								t_args;

static volatile sig_atomic_t	g_sigint_received;

// Main
int								tracer(pid_t child_pid, t_args *args);
void							handle_syscall(pid_t pid, int *in_syscall,
									t_args *args);

// Utils
const char						**get_syscall_names(void);
int								get_max_syscall(const char **syscalls);
int								safe_waitpid(pid_t pid, int *status);
void							print_stats(t_args *args);
int								is_32_bit(pid_t pid);
void							clean(t_args *args);

#endif
