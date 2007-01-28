RAGEL = ragel
RLCODEGEN = rlcodegen

all: svg_path_parser

ps: svg_path_parser.ps

svg_path_parser: svg_path_parser.o
	g++ -g -o svg_path_parser svg_path_parser.o 

svg_path_parser.cpp: svg_path_parser.rl
	$(RAGEL) svg_path_parser.rl | $(RLCODEGEN) -T0 -o svg_path_parser.cpp

svg_path_parser.o: svg_path_parser.cpp
	g++ -Wall -g -c -O3 -o $@ $<

svg_path_parser.svg: svg_path_parser.rl
	$(RAGEL) svg_path_parser.rl | $(RLCODEGEN) -V | dot -Tsvg > svg_path_parser.svg

svg_path_parser.ps: svg_path_parser.rl
	$(RAGEL) svg_path_parser.rl | $(RLCODEGEN) -V | dot -Tps > svg_path_parser.ps

distclean clean:
<<<<<<< .mine
	rm -Rf *.o svg_path_parser.cpp svg_path_parser svg_path_parser.ps
=======
	rm -Rf *.o svg_path_parser.cpp svg_path_parser svg_path_parser.ps svg_path_parser.svg
>>>>>>> .r573
