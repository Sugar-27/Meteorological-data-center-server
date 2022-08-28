/**
 * @file crtsurfdata1.cpp
 * @brief 本程序用于生成全国气象站点观测的分钟数据
 * @author Sugar (hzzou@dhu.edu.cn)
 * @date 2022-08-28
 */

#include "_public.h"

CLogFile logFile(10);
/*
省   站号  站名 纬度   经度  海拔高度
安徽,58015,砀山,34.27,116.2,44.2
*/
struct st_stcode {
    char provName[31];  // 省
    char obTid[11];     // 站号
    char obtName[31];   // 站名
    double lat;         // 纬度
    double lon;         // 经度
    double height;      // 海拔高度
};

// 存放全国气象站点参数的容器
vector<struct st_stcode> vstcode;

// 把站点参数文件中的信息加载到容器中
bool loadSTCode(const char* iniFile) {
    CFile file;
    if (file.Open(iniFile, "r") == false) {
        // 打开失败，日志记录函数返回
        logFile.Write("file.Open(%s) failed\n", iniFile);
        return false;
    }
    char strBuffer[301];
    CCmdStr cmdStr;
    struct st_stcode stcode;
    while (true) {
        // 从站点参数读取一行，如果已经读取完毕，跳出循环
        // memset(strBuffer, 0, sizeof strBuffer); // Fgets里会初始化
        if (!file.Fgets(strBuffer, sizeof(strBuffer) - 1, true)) {
            break;
        }

        // 把读取到的每一行拆分
        cmdStr.SplitToCmd(strBuffer, ",", true); 
        if (cmdStr.CmdCount() != 6) continue;   // 扔掉无效行         
        cmdStr.GetValue(0, stcode.provName, 30);    // 省
        cmdStr.GetValue(1, stcode.obTid, 10);       // 站号
        cmdStr.GetValue(2, stcode.obtName, 30);     // 站名         
        cmdStr.GetValue(3, &stcode.lat);            // 纬度
        cmdStr.GetValue(4, &stcode.lon);            // 经度 
        cmdStr.GetValue(5, &stcode.height);         // 海拔高度

        // 把结构体存入容器
        vstcode.push_back(stcode);
    }
    /* 测试代码
    for (auto& item : vstcode) {
        logFile.Write("%s, %s, %s, %.02f, %.02f, %.02f\n", item.provName, item.obTid, item.obtName, item.lat, item.lon, item.height);
    }
    */
    return true;
}

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

    logFile.Write("crtsurfdata 开始运行\n");

    // TODO: @sugar 业务代码
    if (!loadSTCode(argv[1])) {
        return -1;  // 加载失败
    }

    logFile.Write("crtsurfdata 运行结束\n");

    return 0;
}

