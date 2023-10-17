CC=cc
RPC_SYSTEM=rpc.o

.PHONY: all format

all: $(RPC_SYSTEM)

$(RPC_SYSTEM): rpc.c rpc.h
	$(CC) -Wall -c -o $@ $<

format:
	clang-format -style=file -i *.c *.h

clean:
	rm -f $(RPC_SYSTEM)