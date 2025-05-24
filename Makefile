all: clean ki

clean:
	rm -f ki

ki: src/*.c
	gcc src/*.c -o ki -Wall -Wpedantic -g3