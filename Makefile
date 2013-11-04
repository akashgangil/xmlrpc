CC = gcc

XMLRPCSRCDIR = ./xmlrpc-c-1.25.26
EXAMPLESDIR = ./code
UTILSDIR = ./utils

subsystem:
	cd $(XMLRPCSRCDIR) && $(MAKE)
	$(shell cd xmlrpc-c-1.25.26 && make install)
	cd $(EXAMPLESDIR) && $(MAKE)
	$(CC) -o service $(UTILSDIR)/service.cpp
clean:
	$(shell cd xmlrpc-c-1.25.26 && ./configure --prefix=$(PWD)/rpc && cd ..)
	make --directory=$(XMLRPCSRCDIR) clean
	make --directory=$(EXAMPLESDIR) clean
