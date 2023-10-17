all:
	gcc wfc.c -owfc -g

run:
	./wfc

debug:
	gdb wfc
	