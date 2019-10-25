#需要生成可执行程序的Makefile
		#程序根目录

		top_srcdir	=	.

	

		#目标程序名

		TARGET = flashApp0
		
		#TARGETFILE = $(addsuffix .c, $(TARGET))

		#CPP_FILES = $(shell ls *.cpp)

		C_FILES = $(shell ls *.c)
		

		#SRCS = $(CPP_FILES) $(C_FILES)
		SRCS = $(C_FILES)
		
		BASE = $(basename $(SRCS))

		TEMP = $(addprefix obj/, $(BASE))
		
		OBJS = $(addsuffix .o, $(TEMP))

		DEPS = $(addsuffix .d, $(addprefix dep/, $(BASE)))
		#包含公共Make规则

        include  $(top_srcdir)/makeinclude/Make.rules
		

		#额外需要包含的头文件的目录位置

		INCLUDEDIR := $(INCLUDEDIR)\
										-I $(top_srcdir)/common/inc\
	

		#所有要包含的静态库的名称

#		LIBS := -lfa_common

	
		#设置目标程序依赖的.o文件
# $(TARGET):$(OBJS) $(LIBS)
#				-rm -f $@.
#					$(CXX) -o $(TARGET) $(INCLUDEDIR) $(LDFLAGS) $(OBJS) $(LIBS)
        
$(TARGET):$(OBJS)
		@echo $(OBJS)
		-rm -f $@
		$(CXX) $(CFLAGS) -o $(TARGET) $(INCLUDEDIR) $(OBJS) 
			
