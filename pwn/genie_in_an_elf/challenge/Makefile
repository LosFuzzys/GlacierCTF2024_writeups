.PHONY: challenge dist clean

CFLAGS=-O0 -g1 -DNUM_WISHES=2 -DUNROLL_WISHES=1

challenge: main.c
	${CC} ${CFLAGS} -o $@ $^

lsp:
	make -C . clean
	bear -- make -C . challenge

clean:
	${RM} challenge compile_commands.json
