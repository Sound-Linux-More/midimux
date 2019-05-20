midimux: midimux.c
	gcc -o midimux -lasound midimux.c

clean:
	rm midimux
