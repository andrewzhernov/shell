all: compile
	@echo
	@echo "run './shell' for testing"
	@echo

run: compile
	./shell

compile:
	gcc shell.c readline.c -o shell

debug:
	gcc shell.c readline.c -o shell -ggdb && gdb ./shell

clean:
	rm -f shell
