all: compile
	@echo
	@echo "run './shell' for testing"
	@echo

run: compile
	./shell

compile:
	gcc shell.c readline.c replace.c lib_string.c error.c arguments_t.c -o shell

debug:
	gcc shell.c readline.c replace.c lib_string.c error.c arguments_t.c -o shell -ggdb && gdb ./shell

clean:
	rm -f shell
