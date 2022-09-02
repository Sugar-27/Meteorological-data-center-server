#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include "_public.h"

int main(int argc, char* argv[]) {
    printf("输入参数数量为：%d\n", argc);

    for (int i = 0; i < argc; ++i) {
        printf("参数%d为：%s\n", i, argv[i]);
    }
    printf("测试程序开始执行\n");
    CFile file;
    if (!file.Open("/home/sugar/project/DataCenter/tools/c/testLog.txt", "w")) {
        // 打开失败，日志记录函数返回
        printf("打开失败\n");
        return -1;
    }
    file.Fprintf("测试开始\n");
    sleep(5);
    file.Fprintf("测试结束，历时5秒\n");
    sleep(2);
    printf("测试程序执行结束\n");

    return 0;
}