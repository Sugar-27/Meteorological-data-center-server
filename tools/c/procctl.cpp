/**
 * @file procctl.cpp
 * @brief 用于执行自定义的辅助程序
 * @author Sugar (hzzou@dhu.edu.cn)
 * @date 2022-09-02
 */
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Using:./procctl timetvl program argv ...\n");
        printf(
            "Example:/home/sugar/project/DataCenter/tools/bin/procctl 5 "
            "/home/sugar/project/DataCenter/idc/bin/crtsurfdata "
            "/home/sugar/project/DataCenter/idc/ini/stcode.ini "
            "/home/sugar/project/DataCenter/tmp/surfdata "
            "/home/sugar/project/DataCenter/log/idc/crtsurfdata.log "
            "xml,json,csv\n\n");

        printf("本程序是服务程序的调度程序，周期性启动服务程序或shell脚本。\n");
        printf(
            "timetvl "
            "运行周期，单位：秒。被调度的程序运行结束后，在timetvl秒后会被procc"
            "tl重新启动。\n");
        printf("program 被调度的程序名，必须使用全路径。\n");
        printf("argvs   被调度的程序的参数。\n");
        printf("注意，本程序不会被kill杀死，但可以用kill -9强行杀死。\n\n\n");

        return -1;
    }

    // 先忽略所有信号并关闭IO，因为程序不希望被打扰
    for (int i = 1; i < 65; ++i) {
        signal(i, SIG_IGN);
        close(i);
    }

    // 目的：让进程在后台运行
    // 方法：生成子进程，父进程关闭，将子进程托管给1号进程
    if (fork() != 0) exit(0);

    // 启用SIGCHLD信号，让父进程能够回收子进程
    signal(SIGCHLD, SIG_DFL);

    char* paragv[argc - 1];
    // argv[0]父进程程序名，argv[1]周期时间，argv[2]要执行的进程路径
    // paragv[0]应该是进程名称
    for (int i = 2; i < argc; ++i) {
        paragv[i - 2] = argv[i];
    }
    paragv[argc - 2] = nullptr;

    // 先执行fork函数，创建一个子进程，让子进程调用execv函数执行对应程序
    // 新程序将替换子进程，不会影响父进程
    // 在父进程中可以，可以调用wait函数等待新进程运行的结果，这样就可以实现调度的功能
    while (true) {
        if (fork() == 0) {
            // 这个程序执行完后会自动退出
            execv(argv[2], paragv);
            // 这行代码是因为如果argv[2]有误无法执行，则会继续向下执行，会出现反复fork的情况
            exit(0);  
        } else {
            int status;
            wait(&status);
            sleep(atoi(argv[1]));   // 按照要求定时执行
        }
    }
    
    return 0;
}