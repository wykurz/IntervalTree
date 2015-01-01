CC     = g++
CFLAGS = -std=c++11 -Wall -g
LFLAGS = -lboost_unit_test_framework.dll
SRCS   = $(wildcard *.cpp)
OBJS   = $(patsubst %.cpp, %.o, $(SRCS))
BINS   = $(patsubst %.cpp, %, $(SRCS))
PERF   = $(patsubst %.cpp, %.perf, $(SRCS))

test : $(BINS)
	./$< --log_level=test_suite --run_test="$(TESTS)"

perf : $(PERF)
	PUPROFILE=cpu.profile ./$< --log_level=test_suite --run_test="$(TESTS)"
	gprof ./$< gmon.out -Q | c++filt | less

% : %.cpp
	$(CC) $(CFLAGS) $< $(LFLAGS) -o $@

%.perf : %.cpp
	$(CC) $(CFLAGS) -pg $< $(LFLAGS) -o $@

clobber :
	rm -f *.bak
	rm -f *.exe
	rm -f *.perf
	rm -f *.out

.PHONY : test clobber
