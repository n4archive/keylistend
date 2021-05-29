OUT ?= ./keylistend
CC ?= gcc
SRCS = main.c

build: $(OUT)

$(OUT): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(OUT)

clean:
	rm -rf $(OUT)

run: $(OUT)
	$(SUDO) $(OUT) $(ARGS)
