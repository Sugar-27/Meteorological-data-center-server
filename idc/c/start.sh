#############################
# 启动数据中心后台服务程序的脚本 #
#############################

#检查服务程序是否超时，配置在/etc/rc.local中由root用户执行
#/home/sugar/project/DataCenter/tools/bin/procctl 30 /home/sugar/project/DataCenter/tools1/bin/checkproc

# 压缩数据中心后台服务程序的备份日志
/home/sugar/project/DataCenter/tools/bin/procctl 300 /home/sugar/project/DataCenter/tools/bin/gzipfiles /log/idc "*.log.20*" 0.04

# 生成用于测试的全国气象站点观测的分钟数据
/home/sugar/project/DataCenter/tools/bin/procctl 60 /project/idc/bin/crtsurfdata /project/idc/ini/stcode.ini /tmp/idc/surfdata /log/idc/crtsurfdata.log xml,json,csv

# 清理原始的全国气象站点观测的分钟数据目录 /tmp/idc/surfdata中的历史数据文件
/home/sugar/project/DataCenter/tools/bin/procctl 300 /home/sugar/project/DataCenter/tools/bin/deletefiles /tmp/idc/surfdata "*" 0.04