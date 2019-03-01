S=$(wildcard *.s)
S_O=$(patsubst %.s,%.o,$(S))
C=cr.c cr_yield_do.c cr_aio.c
C_O=$(patsubst %.c,%.o,$(C))
T=$(patsubst test/%,%,$(wildcard test/*))
T_CLEAN=$(patsubst %,clean_%,$(T))


CFLAGS=-std=c11 -Wall -Wextra --pedantic -Ofast -lrt
TEST_CFLAGS=-I../.. -L../.. -lcr
ASFLAGS=-Ofast

all: test

libcr.a: $(S_O) $(C_O)
	ar rcs $@ $^

test: libcr.a $(T)

$(T): CFLAGS += $(TEST_CFLAGS)
$(T):
	$(MAKE) -f ../../Makefile.tests -C test/$@ CFLAGS="$(CFLAGS)"

$(T_CLEAN):
	$(MAKE) -f ../../Makefile.tests -C test/$(subst clean_,,$@) clean

$(S_O): %.o : %.s
	as $(ASFLAGS) -o $@ $<

$(C_O): %.o : %.c
	gcc $(CFLAGS) -o $@ -c $<

clean: $(T_CLEAN)
	rm -f $(S_O) $(C_O) libcr.a

rlogin_build: clean
	ssh nilesr@rlogin.cs.vt.edu rm -rf cr-auto
	ssh nilesr@rlogin.cs.vt.edu mkdir cr-auto
	tar c . | ssh nilesr@rlogin.cs.vt.edu tar xv -C cr-auto
	ssh nilesr@rlogin.cs.vt.edu make -C cr-auto

gdb: rlogin_build
	ssh nilesr@rlogin.cs.vt.edu -t gdb cr-auto/test

.PHONY: clean rlogin_build rlogin gdb all $(T) $(T_CLEAN)
