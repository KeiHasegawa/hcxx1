PROG = hcxx1.exe

all:$(PROG)

UNAME := $(shell uname)
ERROR_CPP = error.cpp
WARNING_CPP = warning.cpp
ifeq ($(LANG),ja_JP.eucJP)
	ERROR_CPP = error_euc.cpp
	WARNING_CPP = warning_euc.cpp
endif
ifeq ($(LANG),ja_JP.UTF-8)
	ERROR_CPP = error_utf.cpp
	WARNING_CPP = warning_utf.cpp
endif
debian = $(if $(wildcard /etc/debian_version),1,0)
ifeq ($(debian),1)
	ERROR_CPP = error_utf.cpp
	WARNING_CPP = warning_utf.cpp
endif
ifneq (,$(findstring Darwin,$(UNAME)))
	ERROR_CPP = error_utf.cpp
	WARNING_CPP = warning_utf.cpp
endif

SRCS =	classes.cpp \
	cmdline.cpp \
	conversion.cpp \
	declarations.cpp \
	declarators.cpp \
	dump.cpp \
	$(ERROR_CPP) \
	expr00.cpp \
	expr01.cpp \
	expr02.cpp \
	expr03.cpp \
	expr04.cpp \
	expr05.cpp \
	expr06.cpp \
	expr07.cpp \
	expr08.cpp \
	expr10.cpp \
	expr14.cpp \
	expr15.cpp \
	expr16.cpp \
	generator.cpp \
	initializers.cpp \
	main.cpp \
	optimize.cpp \
	parse.cpp \
	scope.cpp \
	statements.cpp \
	type.cpp \
	vars.cpp \
	$(WARNING_CPP) \
	cxx_l.cpp \
	cxx_y.cpp \

error_euc.cpp:error.cpp
	sjis2euc.exe < $< > $@

error_utf.cpp:error_euc.cpp
	euc2utf.exe < $< > $@

warning_euc.cpp:warning.cpp
	sjis2euc.exe < $< > $@

warning_utf.cpp:warning_euc.cpp
	euc2utf.exe < $< > $@

OBJS = $(SRCS:.cpp=.o)

RULES = rule.00 rule.01 rule.02 rule.03 rule.04 \
        rule.06 rule.07 rule.08 rule.09 \
        rule.10 rule.11 rule.12 \

$(OBJS) : cxx_y.h

cxx_y.o : $(RULES)

cxx_l.cpp:flex_script cxx.l
	./flex_script cxx.l

cxx_y.cpp:bison_script bison_conv.pl cxx.y
	./bison_script cxx.y

cxx_y.h:bison_script bison_conv.pl cxx.y
	./bison_script cxx.y

cxx_y.output:bison_script bison_conv.pl cxx.y
	./bison_script cxx.y

rule.00:cxx_y.output
	perl bison_rule.00.pl $< > $@

rule.01:cxx_y.output
	perl bison_rule.01.pl $< > $@

rule.02:cxx_y.output
	perl bison_rule.02.pl $< > $@

rule.03:cxx_y.output
	perl bison_rule.03.pl $< > $@

rule.04:cxx_y.output
	perl bison_rule.04.pl $< > $@

rule.06:cxx_y.output
	perl bison_rule.06.pl $< > $@

rule.07:cxx_y.output
	perl bison_rule.07.pl $< > $@

rule.08:cxx_y.output
	perl bison_rule.08.pl $< > $@

rule.09:cxx_y.output
	perl bison_rule.09.pl $< > $@

rule.10:cxx_y.output
	perl bison_rule.10.pl $< > $@

rule.11:cxx_y.output
	perl bison_rule.11.pl $< > $@

rule.12:cxx_y.output
	perl bison_rule.12.pl $< > $@

DEBUG_FLAG = -g
CXXFLAGS = -w $(DEBUG_FLAG) -DYYDEBUG

DYNAMIC_LOADING_LIBRARY_FLAGS = -ldl
ifneq (,$(findstring CYGWIN,$(UNAME)))
	DYNAMIC_LOADING_LIBRARY_FLAGS =
endif

$(PROG) : $(OBJS)
	$(CXX) $(DEBUG_FLAG) -o $(PROG) $(OBJS) $(DYNAMIC_LOADING_LIBRARY_FLAGS)

RM = rm -r -f

clean:
	$(RM) cxx_l.cpp* cxx_y.cpp* cxx_y.h
	$(RM) $(RULES)
	$(RM) cxx_y.out*
	$(RM) $(PROG) *.o *.stackdump *~
	$(RM) .vs Debug Release x64
