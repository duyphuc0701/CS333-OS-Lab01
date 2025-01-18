#include "kernel/types.h"
#include "user/user.h"

int sendToRightNeighbor(int read_end, int prime) {
    int num;
    int p[2];
    if(pipe(p) < 0) {
        printf("pipe failed\n");
        exit(1);
    }

    if(!fork()) {
        close(p[0]);
        while(read(read_end, &num, sizeof(num))) {
            if(num % prime != 0) {
                write(p[1], &num, sizeof(num));
            }
        }
        close(p[1]);
        exit(0);
    }

    close(p[1]);
    close(read_end);
    return p[0];
} 

int createFirstPipe(int start, int end) {
    int p[2]; 
    if(pipe(p) < 0) {
        printf("pipe failed\n");
        exit(1);
    }
    if(!fork()) {
        close(p[0]);
        while (start <= end) {
            write(p[1], &start, sizeof(start));
            start++;
        }
        
        close(p[1]);
        exit(0);
    }
    close(p[1]);
    return p[0];
}

int main() {
    int read_end = createFirstPipe(2, 280);
    int prime;
    while(read(read_end, &prime, sizeof(prime))) {
        printf("prime %d\n", prime);
        read_end = sendToRightNeighbor(read_end, prime);
    }

    exit(0);
}