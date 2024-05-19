all:
	gcc -c toml.c -o toml.o
	gcc -c parseToml.c -o parseToml.o
	gcc toml.o parseToml.o sdl.c -o sdl