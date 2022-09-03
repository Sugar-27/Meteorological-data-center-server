/**
 * @file deletefiles.cpp
 * @brief 
 * @author Sugar (hzzou@dhu.edu.cn)
 * @date 2022-09-03
 */

#include "_public.h"

void EXIT(int sig) {
    printf("程序退出，sig=%d\n", sig);
    exit(0);
}

int main(int argc, char* argv[]) {
    // 程序的帮助
    if (argc != 4) {
        printf("\n");
        printf("Using:./tools/bin/deletefiles pathname matchstr timeout\n\n");

        printf(R"(
Example:/tools/bin/deletefiles /log/idc "*.log.20*" 0.02
        /tools/bin/deletefiles /tmp/idc/surfdata "*.xml,*.json" 0.01
        /tools/bin/procctl 300 /tools/bin/deletefiles /log/idc "*.log.20*" 0.02
        /tools/bin/procctl 300 /tools/bin/deletefiles /tmp/idc/surfdata "*.xml,*.json" 0.01
        )");

        printf("\n\n这是一个工具程序，用于删除历史的数据文件或日志文件。\n");
        printf("本程序把pathname目录及子目录中timeout天之前的匹配matchstr文件全部删除，timeout可以是小数。\n");
        printf("本程序不写日志文件，也不会在控制台输出任何信息。\n\n\n");

        return -1;
    }

    // 关闭全部的信号和输入输出
    CloseIOAndSignal(true);
    signal(SIGINT, EXIT);
    signal(SIGTERM, EXIT);

    // 获取文件的超时的时间点
    char strTimeOut[21];
    LocalTime(strTimeOut, "yyyy-mm-dd hh24:mi:ss", 0 - static_cast<int>(stof(argv[3]) * 24 * 60 * 60));

    // 打开目录，CDir.OpenDir()
    CDir Dir;
    if (!Dir.OpenDir(argv[1], argv[2], 10000, true)) {
        printf("Dir.OpenDir(%s) failed\n", argv[1]);
        return -1;
    }
    // 遍历目录中的文件名
    while (true) {
        // 得到每一个文件的信息，用CDir.ReadDir()方法
        if (!Dir.ReadDir()) break;  // 读取失败则说明没有文件了
        // 与超时的时间点比较，如果更早，则说明需要压缩
        printf("Ful1FileName=%s\n",Dir.m_FullFileName);
        if ((strcmp(Dir.m_ModifyTime, strTimeOut) < 0)) {
            // 压缩命令，调用系统的gzip命令
            if (REMOVE(Dir.m_FullFileName)) {
                printf("Remove %s OK\n", Dir.m_FullFileName);
            } else {
                printf("Remove %s failed\n", Dir.m_FullFileName);
            }
        }
        
    }


    return 0;
}