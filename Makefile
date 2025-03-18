FLAGS = -Wall -O3 -fsanitize=address -Iinclude
LIBS = -lgsl -lgslcblas -lm
SRC = src/propheticBandits.c src/util.c src/banditAlgs/optimal.c src/banditAlgs/greedy.c src/banditAlgs/epsilonGreedy.c src/banditAlgs/succElim.c src/banditAlgs/ucb1.c src/banditAlgs/ucb2.c src/banditAlgs/exp3.c

all: propheticBandits priceGenerator

propheticBandits: src/propheticBandits.c
	@gcc $(SRC) $(FLAGS) $(LIBS) -o bin/propheticBandits

priceGenerator: src/priceGenerator.c
	@gcc src/priceGenerator.c $(FLAGS) $(LIBS) -o bin/priceGenerator
	@mkdir -p prophetData

clean:
	@rm bin/propheticBandits
	@rm bin/priceGenerator
