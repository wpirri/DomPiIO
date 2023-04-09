 #!/usr/bin/make -f

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
