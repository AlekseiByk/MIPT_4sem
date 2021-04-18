NUMBERS = 1 2 3 4 5 6 7 8 9 10
LINE = ---------------------------------------

all:
	gcc -Wall main.c -o prog -pthread

test:
	for i in $(NUMBERS); do echo $$i:'\n'; time -p ./prog $$i; echo $(LINE); done;
