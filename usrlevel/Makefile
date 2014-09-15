CC = gcc
CFLAGS = -W -o2 -g

SOURCES = $(shell find *.c)
OBJS = $(addsuffix .o, $(basename $(shell find *.c)))

EXEC = run

all: $(SOURCES) $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

.c.o:
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm -f $(EXEC) $(OBJS)

r:
	@./$(EXEC)
