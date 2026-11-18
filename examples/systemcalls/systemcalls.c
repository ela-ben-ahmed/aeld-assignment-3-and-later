
#include "systemcalls.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdarg.h>

bool do_system(const char *cmd)
{
    int status = system(cmd);

    if (status == -1) {
        return false;
    }

    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        return true;
    }

    return false;
}

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    va_end(args);

    fflush(stdout);

    pid_t pid = fork();
    if (pid < 0) {
        return false;
    } else if (pid == 0) {
        execv(command[0], command);
        exit(-1);
    } else {
        int status;
        pid_t wait_result = waitpid(pid, &status, 0);
        if (wait_result == -1) {
            return false;
        }

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            return true;
        }
    }

    return false;
}

bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    va_end(args);

    int fd = open(outputfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        return false;
    }

    fflush(stdout);

    pid_t pid = fork();
    if (pid < 0) {
        close(fd);
        return false;
    } else if (pid == 0) {
        if (dup2(fd, STDOUT_FILENO) < 0) {
            close(fd);
            exit(-1);
        }
        close(fd);
        execv(command[0], command);
        exit(-1);
    } else {
        close(fd);
        int status;
        pid_t wait_result = waitpid(pid, &status, 0);
        if (wait_result == -1) {
            return false;
        }

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            return true;
        }
    }

    return false;
}
