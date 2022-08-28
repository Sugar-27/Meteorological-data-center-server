/**
 * @file crtsurfdata1.cpp
 * @brief 本程序用于生成全国气象站点观测的分钟数据
 * @author Sugar (hzzou@dhu.edu.cn)
 * @date 2022-08-28
 */

#include "_public.h"

CLogFile logFile(10);

int main(int argc, char* argv[]) {
    // inifile: 全国气象站点参数文件
    // outpath: 生成的测试数据存放目录
    // logfile: 生成的日志
    if (argc != 4) {
        printf("Using: ./crtsurfdata1 inifile outpath logfile\n");
        printf(
            "Example:/home/sugar/project/DataCenter/idc/bin/crtsurfdata1 "
            "/home/sugar/project/DataCenter/idc/ini/stcode.ini "
            "/home/sugar/project/DataCenter/tmp/surfdata " 
            "/home/sugar/project/DataCenter/log/idc/crtsurfdata1.log\n\n");

        printf("inifile 全国气象站点参数文件名。\n");
        printf("outpath 全国气象站点数据文件存放的目录。\n");
        printf("logfile 本程序运行的日志文件名。\n\n");

        return -1;
    }

    if (!logFile.Open(argv[3])) {
        printf("logFile.Open(%s) failed, please recheck.", argv[3]);
        return -1;
    }

    logFile.Write("crtsurfdata1 开始运行\n");

    // TODO: @sugar 业务代码
    for (size_t i = 0; i < 1e9; ++i) {
        logFile.Write("这是第%d次执行\n", i);
    }

    logFile.Write("crtsurfdata1 运行结束\n");

    return 0;
}

