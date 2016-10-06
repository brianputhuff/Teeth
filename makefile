# teeth makefile

# files to compile
SOURCE = main.c

# c compiler
CC = cc

# compiler flags
# -ansi for ansi c standard
# -Wall for compiler warnings
CFLAGS = -Wall -ansi -pedantic -lSDL2

# output file (executable)
OUT = teeth

# compile
all :   $(SOURCE)
	$(CC) $(SOURCE) $(CFLAGS) -o $(OUT)

# clean
clean :
	-rm $(OUT)
