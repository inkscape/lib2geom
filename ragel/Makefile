RAGEL = ragel
RLCODEGEN = rlcodegen

all: svgd_path_parser

ps: svgd_path_parser.ps

svgd_path_parser: svgd_path_parser.o
	g++ -g -o svgd_path_parser svgd_path_parser.o 

svgd_path_parser.cpp: svgd_path_parser.rl
	$(RAGEL) svgd_path_parser.rl | $(RLCODEGEN) -G2 -o svgd_path_parser.cpp

svgd_path_parser.o: svgd_path_parser.cpp
	g++ -Wall -g -c -O3 -o $@ $<

svgd_path_parser.svg: svgd_path_parser.rl
	$(RAGEL) svgd_path_parser.rl | $(RLCODEGEN) -V | dot -Tsvg > svgd_path_parser.svg

svgd_path_parser.ps: svgd_path_parser.rl
	$(RAGEL) svgd_path_parser.rl | $(RLCODEGEN) -V | dot -Tps > svgd_path_parser.ps

distclean clean:
	rm -Rf *.o svgd_path_parser.cpp svgd_path_parser svgd_path_parser.ps
