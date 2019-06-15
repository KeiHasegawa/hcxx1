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

SRCS =	cxx_l.cpp \
	cxx_y.cpp \
	classes.cpp \
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
	record_type.cpp \
	scope.cpp \
	statements.cpp \
	type.cpp \
	vars.cpp \
	$(WARNING_CPP) \

error_euc.cpp:error.cpp
	sjis2euc.exe < $< > $@

error_utf.cpp:error_euc.cpp
	euc2utf.exe < $< > $@

warning_euc.cpp:warning.cpp
	sjis2euc.exe < $< > $@

warning_utf.cpp:warning_euc.cpp
	euc2utf.exe < $< > $@

OBJS = $(SRCS:.cpp=.o)

PATCHS = patch.00.p patch.01.p patch.02.p patch.03.p patch.04.p \
	 patch.05.p patch.05.2.p patch.06.p patch.07.p patch.08.p \
	 patch.09.p patch.10.p patch.10.p2 patch.11.p

PATCHS_HEADER = patch.03.q patch.04.q patch.10.q

parse.o : $(PATCHS_HEADER)
expr01.o : $(PATCHS_HEADER)

$(OBJS) : cxx_y.h

cxx_y.o : $(PATCHS)

cxx_l.cpp:flex_script cxx.l
	./flex_script cxx.l

cxx_y.cpp:bison_script bison_conv.pl cxx.y
	./bison_script cxx.y

cxx_y.h:bison_script bison_conv.pl cxx.y
	./bison_script cxx.y

BISON_REP_FILE = cxx_y.output

$(BISON_REP_FILE):bison_script bison_conv.pl cxx.y
	./bison_script cxx.y

$(PATCHS) : $(BISON_REP_FILE)

%.p : %.pl
	perl $< $(BISON_REP_FILE) > $@

%.p2 : %.pl
	perl $< -2 $(BISON_REP_FILE) > $@

%.q : %.pl
	perl $< -h $(BISON_REP_FILE) > $@

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
	$(RM) $(PATCHS) $(PATCHS_HEADER)
	$(RM) $(BISON_REP_FILE)
	$(RM) error_euc.cpp warning_euc.cpp error_utf.cpp warning_utf.cpp
	$(RM) $(PROG) *.o *.stackdump *~
	$(RM) .vs Debug Release x64
