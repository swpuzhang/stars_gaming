#*************************************************
#  by xuhu 2011-01-08
#*************************************************

DEBUG	:= yes

#*************************************************

FLAG_DEBUG = -g -D _DEBUG -D TIXML_USE_STL -fpic -std=c++11 
FLAG_RELEASE =  -D TIXML_USE_STL -fpic

TARGET_DEBUG = akgamesvr
TARGET_RELEASE = akgamesvr

ifeq (yes, $(DEBUG))
   CFLAG       = $(FLAG_DEBUG)
   TARGET	= $(TARGET_DEBUG)
else
   CFLAG       = $(FLAG_RELEASE)
   TARGET = $(TARGET_RELEASE)
endif


CC = g++

SRC = $(wildcard *.cpp)
OBJ = $(patsubst %.cpp, %.o, $(SRC))


INCLUDE = -I../../../3rd  \
-I../../../3rd/ACE_wrappers \
-I../../../3rd/include \
-I../../../common/include \
-I..\
-I../userproxy \
-I/usr/include/mysql++ \
-I/usr/include/mysql \

LIBS := -luserproxy -lmframe -lmredis -lhiredis -lmdbi -lmysqlpp -lmysqlclient -lz -llogger -lACE -lmtools -ljsoncpp -lrabbitmq -lcurl  -ltinyxml -lpthread -ldl  -lrt

LIB_DIR := -L../../../3rd/lib \
-L../../../common/lib \
-L../userproxy \
-L/usr/lib


$(TARGET) : $(OBJ)
	$(CC) -o $(TARGET) $(OBJ) $(LIBS) $(LIB_DIR)
	cp -f $(TARGET) ../bin

$(OBJ): %.o: %.cpp
	$(CC) -c $(CFLAG) $(INCLUDE) $< -o $@

all :
	make

.PHONY : clean
clean :
	rm -f $(TARGET) $(OBJ)
