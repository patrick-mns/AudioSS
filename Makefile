CC = gcc
CFLAGS = -Wall -O2 -fsanitize=address 
LDFLAGS = -ljson-c -lm
SRC = main.c audioss.c external/wav16.c 
OUT = dist/audio§

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)

clean:
	rm -f $(OUT)