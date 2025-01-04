CC 		= gcc
CFLAGS  = -std=c90
SRC 	= src/main.c src/includes/* src/modules/*

EXEC_MONO_PROCESS  = pagy-mono
EXEC_MULTI_PROCESS = pagy-multi

all: $(EXEC_MULTI_PROCESS) $(EXEC_MONO_PROCESS)

$(EXEC_MULTI_PROCESS): CFLAGS += -DMULTI_PROCESS

$(EXEC_MULTI_PROCESS): $(SRC)
	mkdir -p build
	$(CC) $(CFLAGS) $(SRC) -o build/$@

$(EXEC_MONO_PROCESS): $(SRC)
	mkdir -p build
	$(CC) $(CFLAGS) $(SRC) -o build/$@

clean:
	rm -rf build
