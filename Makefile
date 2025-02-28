all: propheticBandits priceGenerator

propheticBandits: propheticBandits.c
	@gcc propheticBandits.c -Wall -O3 -o propheticBandits -lgsl -lgslcblas -lm -fsanitize=address

priceGenerator: priceGenerator.c
	@gcc priceGenerator.c -Wall -O3 -o priceGenerator -lgsl -lgslcblas -lm -fsanitize=address
	@mkdir -p prophetData

clean:
	@rm propheticBandits
	@rm priceGenerator
