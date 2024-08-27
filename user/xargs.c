#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"


#define STDIN 0
#define BUF_SZ 512

static char buf[1024];
static char *args[MAXARG];
static int arg_num_init = 0;
static int argnum = 0;

int readline(int fd) {
    char readbuf[2];
    argnum = arg_num_init;
    int offset = 0;
    while (1) {
        // 找到第一个非空字符
        while (1) {
            if (read(fd, readbuf, 1) != 1) {
                return 0;
            }
            if (readbuf[0] == '\n') {
                return 1;
            }
            if (readbuf[0] != ' ' && readbuf[0] != '\t') {
                args[argnum++] = buf + offset;
                buf[offset++] = readbuf[0];
                break;
            }
        }
        // 找到一个 arg
        while (1) {
            if (read(fd, buf + offset, 1) != 1) {
                buf[offset] = '\0';
                return 0;
            }
            if (buf[offset] == '\n') {
                buf[offset] = '\0';
                return 1;
            }
            if (buf[offset] == ' ' || buf[offset] == '\t') {
                buf[offset++] = '\0';
                break;
            }
            ++offset; 
        }
    }
}


int
main(int argc, char *argv[])
{
    int pid, status;
    if (argc < 2) {
        fprintf(2, "Usage: xargs command ...\n");
        exit(1);
    }
    char *command = argv[1];
    arg_num_init = argc - 1;
    args[0] = command;
    for (int i = 1; i < argc; i++) {
        args[i] = argv[i + 1];
    }
    int flag = 1;
    while (flag) {
        flag = readline(STDIN);
        args[argnum] = 0;
        if (flag == 0 && argnum == arg_num_init) {
            exit(0);
        }
        pid = fork();
        if (pid == 0) {
            exec(command, args);
            printf("exec failed!\n");
            exit(1);
        } else {
            wait(&status);
        }
    }
    exit(0);
}
