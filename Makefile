PROJECT = midimux
CC = gcc
CFLAGS = -Wall
LDFLAGS = -lasound -s
RM = rm -f

all: $(PROJECT)

$(PROJECT): $(PROJECT).c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	$(RM) $(PROJECT)
