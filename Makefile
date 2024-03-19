# Makefile for the linux target.
#
#
# Denver project (NES emulator) (c) 2023 P. Santing
#
#

rwildcard=$(wildcard $1) $(foreach d,$1,$(call rwildcard,$(addsuffix /$(notdir $d),$(wildcard $(dir $d)*))))

CXX = c++

CXXFLAGS = -I imgui -I imgui/backends -c -O3 -Ofast -Wformat `sdl2-config --cflags`
LDFLAGS = -lSDL2 -lGL

SRCS := $(call rwildcard,./*.cpp) $(call rwildcard,./*.c)
BINSX := $(SRCS:%.cpp=%.o)
BINS := $(BINSX:%.c=%.o)

all: denver
denver: ${BINS}
	${CXX} -o denver ${BINS} ${LDFLAGS}
%.o: %.cpp
	${CXX} ${CXXFLAGS} $< -o $@

clean:
	@echo "Cleaning up..."
	rm -rvf ${BINS}
	rm ./denver

rebuild: clean denver

sources:
	@echo "Source files: ${SRCS}" 

showbins:
	@echo "Binary files: ${BINS}" 
