/**
 * @file book.cpp
 * @brief 心跳程序测试，已集成到_public中
 * @author Sugar (hzzou@dhu.edu.cn)
 * @date 2022-09-03
 */
#include "_public.h"

#define MAXNUMP_ 1000   // 最多支持的心跳数量
#define SHMKEYP_ 0x9091 // 设定共享内存的KEY
#define SEMKEYP_ 0x9019 // 设定信号量的KEY

void EXIT(int sig) {
    printf("sig=%d\n", sig);
    exit(0);
}

// 进程心跳信息结构体
struct st_pinfo {
    int pid;        // 进程id
    char pname[51]; // 进程名称
    int timeout;    // 超时时间
    time_t atime;   // 最后一次心跳的时间，如果当前时间 - atime超过了timeout，则说明应重启进程
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Using: ./book procname timeout\n");
        return 0;
    }

    signal(9, EXIT);
    signal(15, EXIT);

    // 操作共享内存空间需要加锁
    CSEM m_sem;
    if (!(m_sem.init(SEMKEYP))) {
        printf("m_sem.init(%x) faild\n", SEMKEYP);
        return -1;
    }

    // 创建/获取共享内存，大小为 n * sizeof(struct st_pinfo)
    int m_shmid = 0;
    if ((m_shmid = shmget(SHMKEYP, MAXNUMP * sizeof(struct st_pinfo), 0640|IPC_CREAT)) == -1) {
        printf("shmget(%x) failed\n", SHMKEYP);
        return -1;
    }

    // 将共享内存连接到当前进程的地址空间
    struct st_pinfo* m_shm = nullptr;
    m_shm = (struct st_pinfo*)shmat(m_shmid, nullptr, 0);   // 这里获得的是首地址，换言之也可以当数组来用
    if (m_shm == (void*) -1) {
        printf("shmat failed\n");
        return -1;
    }

    // 创建当前进程心跳信息结构体变量，把本信息填进去
    struct st_pinfo stpinfo;
    memset(&stpinfo, 0, sizeof(stpinfo));
    stpinfo.pid = getpid();
    STRNCPY(stpinfo.pname, sizeof(stpinfo.pname), argv[1], 50);
    stpinfo.timeout = atoi(argv[2]);
    stpinfo.atime = time(0);

    int m_pos = -1;

    // 如果进程因为一些原因停止了，那么它在共享内存中的信息并不会被清理
    // 而此时如果又刚好有一个新进程的pid与遗留在共享内存中的信息相同
    // 那么就会出现这个新进程刚开就被守护进程杀死的情况
    // 解决办法：先查找一遍共享内存，看是否有相同的pid，如果有则直接复用
    for (int i = 0; i < MAXNUMP; ++i) {
        if (m_shm[i].pid == stpinfo.pid) {
            m_pos = i;
            break;
        }
    }

    m_sem.P();
    // 在共享内存中查找一个空位置，把当前进程的心跳信息存入共享内存中
    for (int i = 0; i < MAXNUMP && m_pos == -1; ++i) {
        if (m_shm[i].pid == 0) {
            // 这块内存还没用过
            m_pos = i;
        }
    }

    if (m_pos == -1) {
        // 没找到
        m_sem.V();
        printf("共享内存空间已经用完\n");
        return -1;
    }

    memcpy(&m_shm[m_pos], &stpinfo, sizeof(struct st_pinfo));
    m_sem.V();

    while (true) {
        // 更新共享内存中本进程的心跳时间
        // m_shm[m_pos].atime = time(0);

        sleep(10);
    }

    // 把当前进程从共享内存中移除
    // m_shm[m_pos].pid = 0;
    memset(&m_shm[m_pos], 0, sizeof(struct st_pinfo));

    // 把共享内存从当前进程中分离
    shmdt(m_shm);

    return 0;
}