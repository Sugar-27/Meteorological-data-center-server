# 开发框架头文件路径
PUBINCL = -I/home/sugar/project/DataCenter/public

# 开发框架cpp文件名，这里直接包含进来，没有采用链接库，是为了方便调试
PUBCPP = /home/sugar/project/DataCenter/public/_public.cpp

# 编译参数
CFLAGS = -g

all: procctl test

procctl: procctl.cpp
		  g++ -o procctl procctl.cpp
		  cp procctl ../bin/.

test: test.cpp
	  g++ -o test test.cpp $(PUBINCL) $(PUBCPP) -lm -lc

clean: 
		rm -f procctl
		rm -f test