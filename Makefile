CC = gcc

XMLRPCSRCDIR = ./xmlrpc-c-1.25.26
EXAMPLESDIR = ./code
UTILSDIR = ./utils

subsystem:
	$(shell ./xmlrpc-c-1.25.26/configure --prefix=$(PWD)/rpc)
	cd $(XMLRPCSRCDIR) && $(MAKE)
	cd $(EXAMPLESDIR) && $(MAKE)
	$(CC) -o service $(UTILSDIR)/service.cpp
clean:
	$(shell ./xmlrpc-c-1.25.26/configure --prefix=$(PWD)/rpc)
	make --directory=$(XMLRPCSRCDIR) clean
	make --directory=$(EXAMPLESDIR) clean
