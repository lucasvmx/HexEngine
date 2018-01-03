# 
#	Makefile
#

CPP = g++
SOURCE = version.cpp
CPPFLAGS =
TARGET =
RM = 

ifeq ($(OS),Windows_NT)
	CPPFLAGS += -DOS_WINDOWS
	TARGET += version.exe
	RM = del /f /q
else
	CPPFLAGS += -DOS_LINUX
	TARGET += version.elf
	RM = rm -f -vv
endif

all: version.o
	$(CPP) $(SOURCE) $(CPPFLAGS) -Wall -Wextra -o $(TARGET) -static -static-libgcc

clean:
	$(RM) *.o *.h *.db

