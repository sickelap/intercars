#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <err.h>
#include <errno.h>

extern char *__progname;
extern int errno;
extern char *optarg;

#define BUFFER_SIZE 8192
#define PATH_MAX 1024

void help()
{
    printf("usage: %s [-h] -f <filename.dbf>\n", __progname);
}

int main(int argc, char *argv[])
{
    int fd, fd2, iteration, mask, index, bytes_read, bytes_written;
    char *buffer, *ptr, source[PATH_MAX], destination[PATH_MAX], ch;
    struct stat st;

    iteration = mask = 0;

    while ((ch = getopt(argc, argv, "m:f:h")) != -1) {
        switch (ch) {
            case 'm':
                if (strstr(optarg, "x") != NULL) {
                    mask = (int)strtol(optarg, NULL, 16);
                } else {
                    mask = atoi(optarg);
                }
                break;

            case 'f':
                (void)strncpy(source, optarg, PATH_MAX);
                break;

            case 'h':
            default:
                help();
                exit(1);
                break;
        }
    }

    if (strnlen(source, 1) == 0) {
      help();
      exit(1);
    }

    if (stat(source, &st) != 0) {
        err(errno, "source stat");
    }

    if ((fd = open(source, O_RDONLY)) == -1) {
        err(errno, "source open");
    }

    snprintf(destination, PATH_MAX, "decoded_%s", source);
    if ((fd2 = open(destination, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)) == -1) {
        close(fd);
        err(errno, "destination open");
    }

    printf("Processing %s ... ", source);
    
    if ((buffer = malloc(BUFFER_SIZE)) == NULL) {
        err(errno, "malloc");
    }

    ptr = buffer;
    
    do {
        iteration++;

        if ((bytes_read = read(fd, buffer, BUFFER_SIZE)) == -1) {
            free(buffer);
            err(errno, "read");
        }

        /**
         * Find correct bit mask for the DBF file
         * We know what we want, it's 0x30 for the first byte in the dbf
         */
        if (iteration == 1 && mask == 0) {
            mask = *buffer ^ 0x30;
            if (mask == 0) {
                close(fd);
                close(fd2);
                free(buffer);
                unlink(destination);
                printf("already decoded\n");
                exit(0);
            }
        }

        for (index = 0; index < BUFFER_SIZE; index++) {
            *buffer++ ^= mask;
        }

        buffer = ptr;

        if ((bytes_written = write(fd2, buffer, bytes_read)) == -1) {
            close(fd);
            close(fd2);
            free(buffer);
            err(errno, "write");
        }

    } while (bytes_read > 0);

    printf("done\n");

    close(fd);
    close(fd2);
    free(buffer);

    return 0;
}

