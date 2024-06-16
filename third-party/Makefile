all:
	gcc -c llm.c -o llm.o
	gcc -c main.c -o main.o
	gcc llm.o main.o src/toml.o -lcurl -lcjson -lsndfile -o my_program