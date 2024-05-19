CC := gcc
CFLAGS := -Wall -Wextra -O3 -lm
PROGS := src/main.c src/include/fileutil.c src/include/utils.c src/include/textgen.c src/include/map.c src/include/argtable3.c
OBJ = obj
SRC = src
OUT = bin/csvcraft

all : clean $(PROGS)
	$(CC) $(CFLAGS) $(PROGS) -o $(OUT)

debug : clean $(PROGS)
	$(CC) -Wall -Wextra -lm -O0 -ggdb3 $(PROGS) -o $(OUT)

.PHONY: clean
	rm -f $(ODIR)/*.o $(OUT)