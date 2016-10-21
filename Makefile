all:
	@gcc -lpthread dirtyc0w.c -o dirtyc0w
clean:
	rm -rf dirtyc0w

