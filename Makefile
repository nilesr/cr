S=$(wildcard *.s)
S_O=$(patsubst %.s,%.o,$(S))

test: $(S_O) cr.o
	gcc $(CFLAGS) $(S_O) cr.o test.c -o test

cr.o:
	gcc $(CFLAGS) cr.c -o cr.o

$(S_O): %.o : %.s
	as -o $@ $<

clean:
	rm $(S_O) cr.o test

.PHONY: clean
