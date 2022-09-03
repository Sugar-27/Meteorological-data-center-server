// #include <stdlib.h>
// #include <unistd.h>
// #include "_public.h"
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <iostream>

struct st_pid {
    int pid;
    char name[51];
};

int main(int argc, char* argv[]) {

    int shmid;

    if ((shmid = shmget(0x5005, sizeof (st_pid), 0640 | IPC_CREAT)) == -1) {
        printf("shmget failed\n");
        return -1;
    }

    st_pid* stpid = nullptr;
    if ((stpid = (st_pid*)shmat(shmid, nullptr, 0)) == (void*)-1) {
        printf("shmat failed\n");
        return -1;
    }

    stpid->pid = 10010;
    strcpy(stpid->name, "JOJO");

    return 0;
}