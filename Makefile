S=$(wildcard *.s)
S_O=$(patsubst %.s,%.o,$(S))
CFLAGS=-ggdb3 -std=c11 -Wall

test: $(S_O) cr.o
	gcc $(CFLAGS) $(S_O) cr.o test.c -o test cr_yield_do.c

cr.o: cr.c
	gcc $(CFLAGS) -c cr.c -o cr.o

$(S_O): %.o : %.s
	as -o $@ $<

clean:
	rm -f $(S_O) cr.o test

rlogin_build: clean
	ssh nilesr@rlogin.cs.vt.edu rm -rf cr-auto
	ssh nilesr@rlogin.cs.vt.edu mkdir cr-auto
	tar c . | ssh nilesr@rlogin.cs.vt.edu tar xv -C cr-auto
	ssh nilesr@rlogin.cs.vt.edu make -C cr-auto

gdb: rlogin_build
	ssh nilesr@rlogin.cs.vt.edu -t gdb cr-auto/test

.PHONY: clean rlogin_build rlogin gdb
