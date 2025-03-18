FLAGS = -Wall -O3 -fsanitize=address -Iinclude
LIBS = -lgsl -lgslcblas -lm
SRC = src/propheticBandits.c src/util.c $(wildcard src/banditAlgs/*.c)
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))

PROPHET = bin/propheticBandits
PRICE = bin/priceGenerator

all: $(PROPHET) $(PRICE)

$(PROPHET): $(OBJ)
	@mkdir -p bin
	@gcc $^ $(FLAGS) $(LIBS) -o $@

$(PRICE): obj/priceGenerator.o
	@mkdir -p bin
	@mkdir -p prophetData
	@gcc $^ $(FLAGS) $(LIBS) -o $@

obj/%.o: src/%.c
	@mkdir -p obj obj/banditAlgs
	@gcc $(FLAGS) -c $< -o $@

clean:
	@rm -r bin
	@rm -r obj
