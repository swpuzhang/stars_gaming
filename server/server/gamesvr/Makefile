#*************************************************
#  by xuhu 2011-01-08
#*************************************************

DEBUG	:= yes

#*************************************************

FLAG_DEBUG = -g -D _DEBUG -D TIXML_USE_STL -fpic -std=c++14
FLAG_RELEASE =  -D TIXML_USE_STL -fpic -std=c++14

TARGET_DEBUG = testgamesvr
TARGET_RELEASE = testgamesvr

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


INCLUDE = -I../../common \
-I../../message/cpp \
-I../../3rd/protobuf \
-I../../common/liblog

LIBS := -llog -lserver -lmessage -lpbmsg -ltask_manager -ltools  -lprotobuf -lprotobuf-lite -lprotoc -lboost_system -lboost_thread -lboost_log -lboost_filesystem -lpthread  -ldl  -lrt -lz

LIB_DIR := -L../../3rd/lib \
-L../../common/lib \
-L../../message/cpp  \


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
