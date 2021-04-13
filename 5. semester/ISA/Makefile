CC=g++
FLAGS=-Wall -pedantic -std=c++17
FILE=dns
COMPILE_FILES=dns.cpp

all:
	$(CC) $(FLAGS) $(COMPILE_FILES) -o $(FILE)

test:
	@make > /dev/null
	@python3 ./tests/test.py

pack:
	tar -cvf xfeket00.tar dns.cpp dns.hpp Makefile README manual.pdf tests

clean:
	rm $(FILE)