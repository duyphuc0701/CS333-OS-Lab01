#include "kernel/types.h"
#include "user/user.h"

void pingpong() {
    int p[2];
    char buf[1];

    if(pipe(p)) {
        fprintf(2, "pipe failed\n");
        exit(1);
    }

    if (fork() == 0) {
        read(p[0], buf, 1);
        printf("%d: received ping\n", getpid());
        write(p[1], "b", 1);
    } else {
        write(p[1], "a", 1);
        wait(0);
        read(p[0], buf, 1);
        printf("%d: received pong\n", getpid());
    }

    close(p[0]);
    close(p[1]);
}

int main()
{
    pingpong();
    return 0;
}