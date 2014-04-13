OPTS=-DPALM -Wall -g3 
CC=m68k-palmos-gcc

SRC = hydra-ftp hydra-pop3 hydra-telnet hydra-vnc hydra-imap hydra-cisco hydra-nntp hydra-vmauthd d3des
OBJ = hydra-ftp.o hydra-pop3.o hydra-telnet.o hydra-vnc.o hydra-imap.o hydra-cisco.o hydra-nntp.o hydra-vmauthd.o d3des.o

hydra.prc: code0000.a.out.grc tver0001.bin
	build-prc hydra.prc "Hydra" Hydr *.a.out.grc *.bin

tver0001.bin: hydra.rcp
	pilrc hydra.rcp

code0000.a.out.grc: hydra-mod.o hydra.o $(SRC)
	$(CC) hydra.o hydra-mod.o $(OBJ)
#	m68k-palmos-strip a.out
	m68k-palmos-obj-res a.out

hydra.o: hydra.c
	$(CC) -c -o hydra.o -O2 hydra.c

hydra-mod.o: hydra-mod.c
	$(CC) -c -o hydra-mod.o -O2 hydra-mod.c

$(SRC):
	$(CC) $(OPTS) -c -o $@.o ../$@.c

clean:
	rm -f a.out *.a.out* *.bin *~ *.o *.prc
