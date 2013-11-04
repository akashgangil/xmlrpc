CC = gcc

XMLRPCSRCDIR = ./xmlrpc-c-1.25.26
EXAMPLESDIR = ./code
UTILSDIR = ./utils

subsystem:
	cd $(XMLRPCSRCDIR) && $(MAKE)
	$(shell make_install.sh)
	cd $(EXAMPLESDIR) && $(MAKE)
	$(CC) -o service $(UTILSDIR)/service.cpp
clean:
	$(shell configure.sh)
	make --directory=$(XMLRPCSRCDIR) clean
	make --directory=$(EXAMPLESDIR) clean
