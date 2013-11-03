
XMLRPCSRCDIR = ./xmlrpc-c-1.25.26
EXAMPLESDIR = ./code

subsystem:
	cd $(XMLRPCSRCDIR) && $(MAKE)
	cd $(EXAMPLESDIR) && $(MAKE)

clean:
	make --directory=$(XMLRPCSRCDIR) clean
	make --directory=$(EXAMPLESDIR) clean
