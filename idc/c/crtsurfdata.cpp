/**
 * @file crtsurfdata1.cpp
 * @brief 本程序用于生成全国气象站点观测的分钟数据
 *        1. 增加生成历史数据文件的功能，为压缩数据和清理文件模块准备历史数据文件
 *        2. 增加信号处理函数，处理信号2和信号15
 *        3. 解决调用exit()函数退出时局部对象没有执行析构函数的问题
 *        4. 把心跳信息写入共享内存
 * @author Sugar (hzzou@dhu.edu.cn)
 * @date 2022-08-28
 */

#include "_public.h"

CLogFile logFile(10);
char strDateTime[21];   // 观测数据的时间
CFile file;             // file定义为全局变量（因为exit函数不会执行局部变量的析构函数）
CPActive PActive;       // 进程心跳

void EXIT(int sig) {
    logFile.Write("程序退出，sig=%d\n", sig);
    exit(0);
}

/*
省   站号  站名 纬度   经度  海拔高度
安徽,58015,砀山,34.27,116.2,44.2
*/
// 每个气象站的数据结构
struct st_stcode {
    char provName[31];  // 省
    char obtid[11];     // 站号
    char obtName[31];   // 站名
    double lat;         // 纬度
    double lon;         // 经度
    double height;      // 海拔高度
};

// 全国气象站点分钟观测数据结构
struct st_surfdata {
    char obtid[11];     // 站点代码
    char dateTime[21];  // 数据时间，格式-yyyymmddhh24miss
    int t;              // 气温：单位0.1摄氏度
    int p;              // 气压：单位0.1百帕
    int u;              // 相对湿度：0-100之间的值，本质就是百分比
    int wd;             // 风向：0-360之间的值，单位度
    int wf;             // 风速：单位0.1m/s
    int r;              // 降雨量：单位0.1mm
    int vis;            // 能见度：单位0.1m
};

// 存放全国气象站点参数的容器
vector<struct st_stcode> vstcode;

// 存放全国气象站点分钟观测数据的容器
vector<struct st_surfdata> vsurfdata;

// 把站点参数文件中的信息加载到容器中
bool loadSTCode(const char* iniFile);

// 模拟生成观测数据存入vsurfdata容器
void creatSurfData();

// 把容器vsurfdata中的所有全国气象观测数据写入文件
bool creatSurfFile(const char* outpath, const char* datafmt);

int main(int argc, char* argv[]) {
    // inifile: 全国气象站点参数文件
    // outpath: 生成的测试数据存放目录
    // logfile: 生成的日志
    if ((argc != 5) && (argc != 6)) {
        printf("Using: ./crtsurfdata inifile outpath logfile [datetime]\n");
        printf(
            "Example:/home/sugar/project/DataCenter/idc/bin/crtsurfdata "
            "/home/sugar/project/DataCenter/idc/ini/stcode.ini "
            "/home/sugar/project/DataCenter/tmp/surfdata " 
            "/home/sugar/project/DataCenter/log/idc/crtsurfdata.log "
            "xml,json,csv "
            "20220903013455\n\n");

        printf("inifile 全国气象站点参数文件名。\n");
        printf("outpath 全国气象站点数据文件存放的目录。\n");
        printf("logfile 本程序运行的日志文件名。\n");
        printf("datafmt 生成数据文件的格式，支持xml,json,csv三种格式，中间用逗号分隔\n\n");

        return -1;
    }

    CloseIOAndSignal(true);
    signal(SIGINT, EXIT);
    signal(SIGTERM, EXIT);

    if (!logFile.Open(argv[3], "a+", false)) {
        printf("logFile.Open(%s) failed, please recheck.", argv[3]);
        return -1;
    }

    PActive.AddPInfo(20, "crtsurfdata");
    logFile.Write("crtsurfdata 开始运行\n");

    // TODO: @sugar 业务代码
    // 将站点参数文件加载到容器中
    if (!loadSTCode(argv[1])) {
        return -1;  // 加载失败
    }

    // 获取当前时间作为观测时间
    memset(strDateTime, 0, sizeof strDateTime);
    if (argc == 5) {
        LocalTime(strDateTime, "yyyymmddhh24miss");
    } else {
        STRCPY(strDateTime, sizeof(strDateTime), argv[5]);
    }

    // 模拟生成全国气象站点分钟观测数据，存放在vsurfdata容器中
    creatSurfData();

    // 打开程序的日志文件
    if (strstr(argv[4], "json") != 0) {
        creatSurfFile(argv[2], "json");
        logFile.Write("生成json数据\n");
    }
    if (strstr(argv[4], "xml") != 0) {
        creatSurfFile(argv[2], "xml");
        logFile.Write("生成xml数据\n");
    }
    if (strstr(argv[4], "csv") != 0) {
        creatSurfFile(argv[2], "csv");
        logFile.Write("生成csv数据\n");
    }

    logFile.Write("crtsurfdata 运行结束\n");

    return 0;
}

bool loadSTCode(const char* iniFile) {
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
        cmdStr.GetValue(1, stcode.obtid, 10);       // 站号
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

void creatSurfData() {
    // 设置随机数种子
    srand(time(0));

    struct st_surfdata st_surfdata;
    // 遍历气象站点参数容器
    for (int i = 0; i < vstcode.size(); ++i) {
        memset(&st_surfdata, 0, sizeof st_surfdata);
        // 用随机数填充分钟观测数据的结构体
        strncpy(st_surfdata.obtid, vstcode[i].obtid, 10);   // 站点代码。
        strncpy(st_surfdata.dateTime, strDateTime, 14);     // 数据时间：格式yyyymmddhh24miss
        st_surfdata.t = rand() % 351;                       // 气温：单位，0.1摄氏度
        st_surfdata.p = rand() % 265 + 10000;               // 气压：0.1百帕
        st_surfdata.u = rand() % 100 + 1;                   // 相对湿度，0-100之间的值。
        st_surfdata.wd = rand() % 360;                      // 风向，0-360之间的值。
        st_surfdata.wf = rand() % 150;                      // 风速：单位0.1m/s
        st_surfdata.r = rand() % 16;                        // 降雨量：0.1mm
        st_surfdata.vis = rand() % 5001 + 100000;           // 能见度：0.1米
        // 把观测数据的结构体放入容器中
        vsurfdata.push_back(st_surfdata);
    }
}

// 把容器vsurfdata中的所有全国气象观测数据写入文件
bool creatSurfFile(const char* outpath, const char* datafmt) {
    CFile file;
    
    // 拼接生成数据的文件名，例如：SURF_ZH_20220829092200_2222.csv
    char strFileName[301];
    sprintf(strFileName, "%s/SURF_ZH_%s_%d.%s", outpath, strDateTime, getpid(), datafmt);
    // 打开文件
    if (!file.OpenForRename(strFileName, "w")) {
        logFile.Write("file.OpenForRename(%s) failed\n", strFileName);
        return false;
    }
    // 写入第一行标题
    if (strcmp(datafmt, "csv") == 0) {
        file.Fprintf("站点代码,数据时间,气温,气压,相对湿度,风向,风速,降雨量,能见度\n");
    } else if (strcmp(datafmt, "xml") == 0) {
        file.Fprintf("<data>\n");
    } else if (strcmp(datafmt, "json") == 0) {
        file.Fprintf(R"({"data":[)");
    }
    
    // 遍历存放观测数据的vsurfdata容器
    for (int i = 0; i < vsurfdata.size(); ++i) {
        // 写入一条记录
        if (strcmp(datafmt, "csv") == 0) {
            file.Fprintf("%s,%s,%.1f,%.1f,%d,%d,%.1f,%.1f,%.1f\n",
                         vsurfdata[i].obtid, vsurfdata[i].dateTime,
                         vsurfdata[i].t / 10.0, vsurfdata[i].p / 10.0,
                         vsurfdata[i].u, vsurfdata[i].wd,
                         vsurfdata[i].wf / 10.0, vsurfdata[i].r / 10.0,
                         vsurfdata[i].vis / 10.0);
        } else if (strcmp(datafmt, "xml") == 0) {
            file.Fprintf(
                "<obtid>%s</obtid><ddatetime>%s</ddatetime><t>%.1f</t><p>%.1f</p>"
                "<u>%d</u><wd>%d</wd><wf>%.1f</wf><r>%.1f</r><vis>%.1f</vis><endl/>\n",
                vsurfdata[i].obtid, vsurfdata[i].dateTime,
                vsurfdata[i].t / 10.0, vsurfdata[i].p / 10.0, vsurfdata[i].u,
                vsurfdata[i].wd, vsurfdata[i].wf / 10.0, vsurfdata[i].r / 10.0,
                vsurfdata[i].vis / 10.0);
        } else if (strcmp(datafmt, "json") == 0) {
            file.Fprintf(
                "{\"obtid\":\"%s\",\"ddatetime\":\"%s\",\"t\":\"%.1f\",\"p\":"
                "\"%.1f\","
                "\"u\":\"%d\",\"wd\":\"%d\",\"wf\":\"%.1f\",\"r\":\"%.1f\","
                "\"vis\":\"%.1f\"}",
                vsurfdata[i].obtid, vsurfdata[i].dateTime, vsurfdata[i].t / 10.0,
                vsurfdata[i].p / 10.0, vsurfdata[i].u, vsurfdata[i].wd,
                vsurfdata[i].wf / 10.0, vsurfdata[i].r / 10.0,
                vsurfdata[i].vis / 10.0);
            if (i != vsurfdata.size() - 1) {
                file.Fprintf(",");
            }
        } 
    }

    if (strcmp(datafmt, "xml") == 0) {
        file.Fprintf("</data>\n");
    } else if (strcmp(datafmt, "json") == 0) {
        file.Fprintf("]}\n");
    }

    // 关闭文件
    file.CloseAndRename();

    UTime(strFileName, strDateTime);    // 修改文件的时间属性

    logFile.Write("生成数据文件%s成功，数据时间%s，记录数%d\n", strFileName, strDateTime, vsurfdata.size());

    return true;
}