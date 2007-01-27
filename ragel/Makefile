RAGEL = ragel
RLCODEGEN = rlcodegen

all: svgd_number

ps: svgd_number.ps

svgd_number: svgd_number.o
	g++ -g -o svgd_number svgd_number.o 

svgd_number.cpp: svgd_number.rl
	$(RAGEL) svgd_number.rl | $(RLCODEGEN) -T0 -o svgd_number.cpp

svgd_number.o: svgd_number.cpp
	g++ -Wall -g -c -O3 -o $@ $<

svgd_number.svg: svgd_number.rl
	$(RAGEL) svgd_number.rl | $(RLCODEGEN) -V | dot -Tsvg > svgd_number.svg

svgd_number.ps: svgd_number.rl
	$(RAGEL) svgd_number.rl | $(RLCODEGEN) -V | dot -Tps > svgd_number.ps

distclean clean:
	rm -Rf *.o svgd_number.cpp svgd_number svgd_number.ps
