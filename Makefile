
example:
	cc example.c falloc.c -o example

.PHONY: test

test:
	cc test.c && ./a.out && rm a.out

.PHONY: clean

clean:
	rm -rf a.out example
