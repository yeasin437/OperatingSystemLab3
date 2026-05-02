#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Usage: ./parent directory exclude_file\n");
        return 1;
    }

    int numberPipe[2];
    int datePipe[2];

    if (pipe(numberPipe) == -1) {
        perror("number pipe error");
        return 1;
    }

    if (pipe(datePipe) == -1) {
        perror("date pipe error");
        return 1;
    }

    pid_t child2 = -1;
    pid_t child3 = -1;

    for (int i = 1; i <= 4; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork error");
            return 1;
        }

        if (pid == 0) {
            if (i == 1) {
                close(numberPipe[0]);
                close(numberPipe[1]);
                close(datePipe[0]);
                close(datePipe[1]);

                char *args[] = {"./child1", argv[1], argv[2], NULL};
                execv("./child1", args);

                perror("exec child1 failed");
                exit(1);
            }

            else if (i == 2) {
                close(numberPipe[0]);
                close(datePipe[0]);
                close(datePipe[1]);

                srand(time(NULL) ^ getpid());
                int num = rand() % 11;

                printf("Child 2 generated number: %d\n", num);
                write(numberPipe[1], &num, sizeof(num));

                close(numberPipe[1]);
                exit(0);
            }

            else if (i == 3) {
                close(numberPipe[0]);
                close(datePipe[0]);
                close(datePipe[1]);

                srand(time(NULL) ^ getpid());
                int num = rand() % 11;

                printf("Child 3 generated number: %d\n", num);
                write(numberPipe[1], &num, sizeof(num));

                close(numberPipe[1]);
                exit(0);
            }

            else if (i == 4) {
                close(numberPipe[0]);
                close(numberPipe[1]);
                close(datePipe[1]);

                char signal;
                read(datePipe[0], &signal, 1);
                close(datePipe[0]);

                execlp("date", "date", NULL);

                perror("exec date failed");
                exit(1);
            }
        }

        else {
            if (i == 2) {
                child2 = pid;
            }
            else if (i == 3) {
                child3 = pid;
            }
        }
    }

    close(numberPipe[1]);
    close(datePipe[0]);

    int num2, num3;

    waitpid(child2, NULL, 0);
    read(numberPipe[0], &num2, sizeof(num2));
    printf("Parent received number from Child 2: %d\n", num2);

    waitpid(child3, NULL, 0);
    read(numberPipe[0], &num3, sizeof(num3));
    printf("Parent received number from Child 3: %d\n", num3);

    if (num2 > num3) {
        printf("Winner: Child 2\n");
    }
    else if (num3 > num2) {
        printf("Winner: Child 3\n");
    }
    else {
        printf("Result: Tie\n");
    }

    write(datePipe[1], "x", 1);

    close(numberPipe[0]);
    close(datePipe[1]);

    return 0;
}
