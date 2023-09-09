 #!/usr/bin/make -f

include configure.mk

all:
	make -C Programas

clean:
	make -C Programas clean

install:
	make -C Script install
	make -C Programas install

uninstall:
	make -C Script uninstall
	make -C Programas uninstall

installer:
	rm -rf $(RUN_HOME)
	mkdir -p $(RUN_HOME)
	mkdir -p $(RUN_HOME)/cgi
	mkdir -p $(RUN_HOME)/html

	cp Programas/Clientes/ioconfig.cgi/ioconfig.cgi $(RUN_HOME)/cgi/
	cp Programas/Clientes/iostatus.cgi/iostatus.cgi $(RUN_HOME)/cgi/
	cp Programas/Clientes/ioswitch.cgi/ioswitch.cgi $(RUN_HOME)/cgi/
	cp Programas/Clientes/wifi.cgi/wifi.cgi $(RUN_HOME)/cgi/
	
	tar cvzf $(UPDATE_FILE) --files-from=update-files.lst

	cp Script/gmond $(RUN_HOME)/
	cp Script/install.sh $(RUN_HOME)/
	cp Config/dompiio.config $(RUN_HOME)/
	cp Config/funcion.tab $(RUN_HOME)/
	cp Config/funcion_parametro.tab $(RUN_HOME)/
	cp Config/server.tab $(RUN_HOME)/
	cp Config/server_parametro.tab $(RUN_HOME)/
	
	tar cvzf $(INSTALL_FILE) --files-from=install-files.lst

	#
	# *******************************************************************************"
	# * Para instalar el sistema:
	# * 
	# * copiar gmonitor_dompiio_install.tar.gz a /home/gmonitor
	# * ejecutar con el usuario root:
	# * 
	# * cd /
	# * tar xvzf /home/gmonitor/gmonitor_dompiio_install.tar.gz
	# * cd $(RUN_HOME)
	# * ./install.sh
	# * 
	# *******************************************************************************"
	# * Para actualizar el sistema:
	# * 
	# * Copiar gmonitor_dompiio_update.tar.gz a /home/gmonitor
	# * Reiniciar el sistema (/etc/init.d/gmond restart)
	# * 
	# *******************************************************************************"
	#
