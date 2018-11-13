DEBUG   := no


FLAG_DEBUG = -g -D _DEBUG  -fpic -std=c++14 -Wall
FLAG_RELEASE =   -fpic -std=c++14 -Wall

TARGET_DEBUG = liblog.a
TARGET_RELEASE = liblog.a

ifeq (yes, $(DEBUG))
	CFLAG       = $(FLAG_DEBUG)
	TARGET       = $(TARGET_DEBUG)
else
	CFLAG       = $(FLAG_RELEASE)
	TARGET = $(TARGET_RELEASE)
endif


CC = g++
SRC = $(wildcard *.cpp)
OBJ = $(patsubst %.cpp, %.o, $(SRC))

INCLUDE = -I../../common \
-I../../common/liblog


LIBS := -lboost_system -lpthread -lboost_thread

LIB_DIR := -L../lib

$(TARGET) : $(OBJ)
	ar crv $(TARGET) *.o
	cp -f $(TARGET) ../lib

$(OBJ): %.o: %.cpp
	$(CC) -c $(CFLAG) $(INCLUDE) $< -o $@

all :
	make

.PHONY : clean
clean :
	-rm -f $(TARGET) $(OBJ)
