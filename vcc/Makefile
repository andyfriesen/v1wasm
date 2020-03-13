CC=clang
# LDFLAGS= -s
# CFLAGS= -O -g
CFLAGS=-g
# CFLAGS= -o3 -m486     #GCC
#CFLAGS= -o6 -mpentium #PGCC

OBJ = vcc.o compile.o funclib.o preproc.o

vcc: $(OBJ)

clean:
	rm *.o vcc || true

all: clean vcc