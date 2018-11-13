#*************************************************
#  by xuhu 2011-01-08
#*************************************************

DEBUG	:= no

#*************************************************

FLAG_DEBUG = -g -D _DEBUG -D TIXML_USE_STL -fpic  -std=c++14 -Wall
FLAG_RELEASE =  -D TIXML_USE_STL -fpic -std=c++14 -Wall
 
TARGET_DEBUG = libpbmsg.a
TARGET_RELEASE =  libpbmsg.a

ifeq (yes, $(DEBUG))
   CFLAG       = $(FLAG_DEBUG)
   TARGET	= $(TARGET_DEBUG)
else
   CFLAG       = $(FLAG_RELEASE)
   TARGET = $(TARGET_RELEASE)
endif


CC = g++
SRC = $(wildcard *.cc)
OBJ = $(patsubst %.cc, %.o, $(SRC))
 
INCLUDE = -I../../3rd/protobuf

$(TARGET) : $(OBJ)
	ar crv $(TARGET) *.o

$(OBJ): %.o: %.cc
	$(CC) -c $(CFLAG) $(INCLUDE) $< -o $@

all :
	make

.PHONY : clean
clean :
	-rm -f $(TARGET) $(OBJ)


	
	
