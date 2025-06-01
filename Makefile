all: clean ki

clean:
	rm -f ki ki-fast ki-mini

ki: src/*.c
	$(CC) src/*.c -o ki -Wall -Wpedantic -g3

ki-mini: src/*.c
	$(CC) src/*.c -o ki-mini -Wall -Wpedantic -Os
	strip ki-mini

ki-fast: src/*.c
	$(CC) src/*.c -o ki-fast -Wall -Wpedantic -O3
	strip ki-fast