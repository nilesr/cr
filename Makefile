S=$(wildcard *.s)
S_O=$(patsubst %.s,%.o,$(S))
C=cr.c cr_yield_do.c
C_O=$(patsubst %.c,%.o,$(C))
CFLAGS=-ggdb3 -std=c11 -Wall
ASFLAGS=-ggdb3

test: $(S_O) $(C_O) test.c
	gcc $(CFLAGS) $(S_O) $(C_O) test.c -o test

$(S_O): %.o : %.s
	as $(ASFLAGS) -o $@ $<

$(C_O): %.o : %.c
	gcc $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(S_O) $(C_O) test

rlogin_build: clean
	ssh nilesr@rlogin.cs.vt.edu rm -rf cr-auto
	ssh nilesr@rlogin.cs.vt.edu mkdir cr-auto
	tar c . | ssh nilesr@rlogin.cs.vt.edu tar xv -C cr-auto
	ssh nilesr@rlogin.cs.vt.edu make -C cr-auto

gdb: rlogin_build
	ssh nilesr@rlogin.cs.vt.edu -t gdb cr-auto/test

.PHONY: clean rlogin_build rlogin gdb
