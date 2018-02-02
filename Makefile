CC = g++
CFLAGS = -std=c++11 -Wall -Wextra -pedantic
LOGIN = xkhait00
NAME = isamon
FILES = Makefile src/*.cpp src/*.h
ALLFILES = $(FILES) README.md manual.pdf

all: src/utils.o src/ping.o src/main.o 
	$(CC) $(CFLAGS) -o $(NAME) src/utils.o src/ping.o src/main.o

clean:
	find . -type f -name '*.o' -delete
	rm -f $(NAME) *.tar *~ 

tar: clean
	tar -cvf $(NAME).tar $(ALLFILES)
