CC 		= gcc
CFLAGS  = -I.
DEPS 	= src/includes/*
OBJ		= src/main.c src/modules/*

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

build/pagy: $(OBJ)
	mkdir -p build
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -rf build