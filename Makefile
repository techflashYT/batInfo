CC=gcc
CFLAGS=-Wall -Wextra -Wno-format-truncation -Wno-sign-compare -g

all: batInfo findbat

.PHONY: all batInfo findbat

batInfo: bin/batInfo
bin/batInfo: batInfo.c
	@mkdir -p bin
	$(CC) $(CFLAGS) -o bin/batInfo batInfo.c


findbat: bin/findbat
bin/findbat: findbat.c
	@mkdir -p bin
	$(CC) $(CFLAGS) -o bin/findbat findbat.c

clean:
	rm -rf bin
