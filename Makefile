OBJS = AC.o
LIBS = -lpatset -ladt -lcommon -lm -lmem

#OBJS := $(patsubst ./src/%.c, %.o, $(wildcard ./src/*.c))
CC = gcc

INCLUDE_1 = ../libs/include/
INCLUDE_2 = ./include/

vpath %.h $(INCLUDE_1) $(INCLUDE_2)
vpath %.c ./src

CFLAGS = -Wall -c -O3 -std=c99 -I$(INCLUDE_1) -I$(INCLUDE_2)

ac: main.o $(OBJS) $(LIBS)
	$(CC) $^ -o $@
	rm *.o

$(OBJS): AC.h common.h mem.h patset.h adt.h

$(OBJS): %.o: %.c %.h
	$(CC) $(CFLAGS) $< -o $@

# main.o: main.c AC.h
# 	$(CC) $(CFLAGS) $< -o $@

.PHONY: ra
ra : ro
	-rm a.test.out
ro:
	-rm $(OBJS)
