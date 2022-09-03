/**
 * @file checkproc.cpp
 * @brief 守护进程
 * @author Sugar (hzzou@dhu.edu.cn)
 * @date 2022-09-03
 */

#include "_public.h"

// 程序运行日志
CLogFile logFIle;

int main(int argc, char* argv[]) {
    // 程序帮助文档
    if (argc != 2) {
        printf("\n");
        printf("Using:./checkproc logfilename\n");

        printf("./procctl 5 ./checkproc ../../log/checkproc.log\n\n");

        printf("本程序用于检查后台服务程序是否超时，如果已超时，就终止它。\n");
        printf("注意：\n");
        printf("  1）本程序由procctl启动，运行周期建议为10秒。\n");
        printf("  2）为了避免被普通用户误杀，本程序应该用root用户启动。\n");
        printf("  3）如果要停止本程序，只能用killall -9 终止。\n\n\n");

        return 0;
    }

    // 打开日志文档
    if (!logFIle.Open(argv[1], "a+")) {
        printf("lofFile.Open(%s) failed\n", argv[1]);
        return -1;
    }

    int shmid = 0;
    // 创建/获取共享内存
    if ((shmid = shmget(SHMKEYP, MAXNUMP * sizeof(struct st_procinfo), 0666|IPC_CREAT)) == -1) {
        logFIle.Write("创建/获取共享内存失败\n");
        return -1;
    }
    // 将共享内存连接到当前进程的地址空间
    st_procinfo* shm = (st_procinfo*)shmat(shmid, nullptr, 0);
    if (!shm) {
        logFIle.Write("共享内存连接到进程失败\n");
        return -1;
    }

    // 遍历共享内存中全部的记录
    for (int i = 0; i < MAXNUMP; ++i) {
        // 如果记录的pid == 0，表示是空记录，continue
        if (shm[i].pid == 0) continue;
        // 如果记录的pid != 0，表示是服务程序的心跳记录
        logFIle.Write("i=%d, pid=%d, pname=%s, timeout=%d, atime=%d\n", i, shm[i].pid, shm[i].pname, shm[i].timeout, shm[i].atime);
        // 向进程发送信号0，判断它是否还存在，如果不存在，从共享内存中删除该记录
        int iret = kill(shm[i].pid, 0);
        if (iret == -1) {
            logFIle.Write("进程pid=%d(%s)已经不存在\n", shm[i].pid, shm[i].pname);
            memset(&shm[i], 0, sizeof(st_procinfo));
        }

        time_t now = time(0);
        // 如果进程未超时，continue
        if (now - shm[i].atime <= shm[i].timeout) continue;
        // 如果进程已经超时
        logFIle.Write("进程pid=%d(%s)已经超时，开始终止\n", shm[i].pid, shm[i].pname);
        // 发送信号15，尝试正常终止进程
        kill(shm[i].pid, 15);
        // 发送信号15之后，每隔1秒判断一下是否关闭成功，一般5s足够关闭一个进程了
        for (size_t j = 0; j < 5; ++j) {
            iret = kill(shm[i].pid, 0);
            if (iret == -1) break; 
            sleep(1);
        }
        // 如果进程仍然存在，尝试发送信号9，强制终止进程
        if (iret != -1) {
            kill(shm[i].pid, 9);
            logFIle.Write("进程pid=%d(%s)非正常终止，使用信号9强制终止\n", shm[i].pid, shm[i].pname);
        } else {
            logFIle.Write("进程pid=%d(%s)已经正常终止\n", shm[i].pid, shm[i].pname);
        }
        // 从共享内存中删除已经超时进程的心跳记录
        memset(&shm[i], 0, sizeof(st_procinfo));
    }

    // 把共享内存从当前进程中分离
    shmdt(shm);

    return 0;
}