SRCS := $(wildcard source/*.c)
OBJS := $(patsubst source/%.c, build/%.o, $(SRCS))
HDRS := include

serve: $(OBJS)
	$(CC) $^ -o $@

build/%.o: source/%.c | build
	$(CC) -c -g $< -o $@ -I$(HDRS)

build:
	mkdir build

clean:
	rm -rf build serve
