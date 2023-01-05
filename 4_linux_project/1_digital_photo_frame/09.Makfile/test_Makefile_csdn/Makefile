
objects  = main.o kbd.o command.o display.o \

	  insert.o search.o files.o utils.o
	  
edit : $(objects)

	   cc -o edit $(objects)

main.o : defs.h


kbd.o : defs.h command.h


command.o : defs.h command.h



display.o : defs.h buffer.h



insert.o : defs.h buffer.h



search.o : defs.h buffer.h



files.o : defs.h buffer.h command.h


utils.o : defs.h


.PHONY : clean

clean :

	   rm edit $(objects)
	   

%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -M $(CPPFLAGS) $< >;$@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ >; $@; \
	rm -f $@.$$$$

ifndef CC
	CC=gcc
endif
ifndef CFLAGS
	CFLAGS=-MMD -O2 -Wall -g //-MMD告诉编译去生成.d文件，即依赖文件
endif

CFLAGS += -I../include
OBJS = chdir.o
LIBS = -L ../lib -lapue
all:chdir2
chdir2:$(OBJS)
	$(CC) $(LDFLAGS) -o chidr2 $(OBJS) $(LIBS) $(EXTRALIBS) 
-include $(OBJS:%.o=%.d)//把这些依赖文件包含进来，构成生成obj的target，每个target下面的命令是系统默认的（没有人为显式指定），CFLAGS就是被这些默认的命令调用



sources := foo.c bar.c baz.s ugh.h
foo: foo.c bar.c baz.s ugh.h
cc $(filter %.c %.s,foo.c bar.c baz.s ugh.h) -o foo

override CFLAGS += $(patsubst %,-I%,$(subst :, ,src:../headers))
override CFLAGS += $(patsubst %,-I%,src ../headers)

src -I../headers











	
	
	
