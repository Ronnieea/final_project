all:
	gcc -c src/toml.c -o src/toml.o
	gcc -c src/parseToml.c -o src/parseToml.o
	gcc -c src/save_game.c -o src/save_game.o
	gcc -c src/audio.c -o src/audio.o
	gcc -c third-party/llm.c -o third-party/llm.o
	gcc src/toml.o src/parseToml.o src/test.c -lcjson `sdl2-config --libs` -lSDL2_ttf -lSDL2_image -lopenal -lsndfile -g -o test
	gcc src/audio.o src/toml.o src/parseToml.o src/save_game.o src/main.c -lcjson `sdl2-config --libs` -lSDL2_ttf -lSDL2_image -lopenal -lsndfile -g -o main
	gcc third-party/llm.o third-party/main.c src/toml.o -lcurl -lcjson -lsndfile -o my_llm
