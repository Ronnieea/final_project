all:
	gcc -c src/toml.c -o src/toml.o
	gcc -c src/parseToml.c -o src/parseToml.o
	gcc -c src/save_game.c -o src/save_game.o
	gcc -c src/audio.c -o src/audio.o
	gcc src/audio.o src/toml.o src/parseToml.o src/save_game.o src/main.c -lcjson `sdl2-config --libs` -lSDL2_ttf -lSDL2_image -lopenal -lsndfile -g -o main