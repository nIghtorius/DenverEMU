# Makefile for the linux target.
#
#
# Denver project (NES emulator) (c) 2023 P. Santing
#
#

rwildcard=$(wildcard $1) $(foreach d,$1,$(call rwildcard,$(addsuffix /$(notdir $d),$(wildcard $(dir $d)*))))

CC = g++

LINKERFLAG = -lSDL2
COMPILERFLAG = -I /home/peter/projects/sdl/SDL/include/ -c -g -O3

SRCS := $(call rwildcard,./*.cpp)
BINS := $(SRCS:%.cpp=%.o)

all: denver
denver: ${BINS}
	${CC} -o denver ${BINS} ${LINKERFLAG}
%.o: %.cpp
	${CC} ${COMPILERFLAG} $< -o $@

clean:
	@echo "Cleaning up..."
	rm -rvf ${BINS}
	rm ./denver
	

