CC     = g++
CFLAGS = -std=c++11 -Wall -g
LFLAGS = -lboost_unit_test_framework.dll
SRCS   = $(wildcard *.cpp)
OBJS   = $(patsubst %.cpp, %.o, $(SRCS))
BINS   = $(patsubst %.cpp, %, $(SRCS))

test : $(BINS)
	./$< --log_level=test_suite --run_test="$(TESTS)"

% : %.cpp
	$(CC) $(CFLAGS) $< $(LFLAGS) -o $@

clobber :
	rm -f *.bak
	rm -f *.exe

.PHONY : test clobber
