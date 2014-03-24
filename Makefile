CC = gcc
CFLAGS = -Wall -pedantic
DEPS = main.h
OBJ = main agent bus customer

all: $(OBJ)

%: %.c $(DEPS)
	$(CC) $(CFLAGS) -o $@ $< 

clean:
	@rm -f $(OBJ)