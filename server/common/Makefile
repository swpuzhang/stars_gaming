#*************************************************
#  by xuhu 2011-01-08
#*************************************************

DEBUG	:= no

#*************************************************

all :
	cd ./liblog && $(MAKE) -j10 DEBUG=$(DEBUG)
	cd ./libtools && $(MAKE) -j10 DEBUG=$(DEBUG)
	cd ./libmessage && $(MAKE) -j10 DEBUG=$(DEBUG)
	cd ./libserver && $(MAKE) -j10 DEBUG=$(DEBUG)
	cd ./libresource && $(MAKE) -j10 DEBUG=$(DEBUG)

.PHONY : clean
clean :
	cd ./liblog && $(MAKE) clean
	cd ./libmessage && $(MAKE) clean
	cd ./libserver && $(MAKE) clean
	cd ./libtools && $(MAKE) clean
	cd ./libresource && $(MAKE) clean

	
	
