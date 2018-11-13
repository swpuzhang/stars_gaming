#*************************************************
#  by xuhu 2011-01-08
#*************************************************

DEBUG	:= yes

#*************************************************

FLAG_DEBUG = -g -D _DEBUG -D TIXML_USE_STL -DMONGOCXX_STATIC -DBSONCXX_STATIC -DMONGOC_STATIC -DBSON_STATIC -fpic -std=c++14
FLAG_RELEASE =  -D TIXML_USE_STL -DMONGOCXX_STATIC -DBSONCXX_STATIC -DMONGOC_STATIC -DBSON_STATIC -fpic -std=c++14

TARGET_DEBUG = testmongo
TARGET_RELEASE = testmongo

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
-I../../3rd/ \
-I../../message/cpp \
-I../../3rd/protobuf \
-I../../3rd/amqp \
-I../../3rd/mongoc \
-I../../3rd/bson \
-I../../3rd/mongocxx/v_noabi \
-I../../3rd/bsoncxx/v_noabi \
-I../../common/liblog

LIBS := -llog -lserver -lmessage -lpbmsg -ltask_manager -lresource -ltools  -lprotobuf -lprotobuf-lite -lprotoc \
-lmongocxx-static -lbsoncxx-static -lmongoc-static -lsasl2 -lssl -lcrypto -lresolv -lbson-static -lm \
-lcpp_redis -ltacopie \
-lboost_system -lboost_thread -lboost_log -lboost_filesystem -lamqpcpp -lpthread  -ldl  -lrt -lz

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
