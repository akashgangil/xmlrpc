SHELL := /bin/bash
CC = gcc

XMLRPCSRCDIR = ./xmlrpc-c-1.25.26
EXAMPLESDIR = ./code
UTILSDIR = ./utils

subsystem:
	cd $(XMLRPCSRCDIR) && $(MAKE)
	make --directory=$(XMLRPCSRCDIR) install
	cd $(EXAMPLESDIR) && $(MAKE)
	$(CC) -o service $(UTILSDIR)/service.c
clean:
	bash -c "cd xmlrpc-c-1.25.26 && ./configure --prefix=$(PWD)/rpc && cd .."
	make --directory=$(XMLRPCSRCDIR) clean
	make --directory=$(EXAMPLESDIR) clean
