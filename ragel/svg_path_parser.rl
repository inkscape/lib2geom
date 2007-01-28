#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <cmath>
#include <vector>

%%{
	machine svgd;
	write data noerror;
}%%

void print_vec(std::vector<double> p) {
	for(unsigned i=0;i < p.size(); i++)
	{
		std::cout<<p.at(i)<< " ";;
	}
}

void parse( char *str )
{
	char *p = str;
	char *pe = str + strlen(str);
        char *start = NULL;
	int cs;
	bool absolute = false;
	std::vector<double> params;

	%%{
		action start_number {
			start = p;
		}

		action push_number {
			char *end=p;
			std::string buf(start, end);
			params.push_back(strtod(start, &end));
			start = NULL;
		}

		action push_true {
			params.push_back(1.0);
		}

		action push_false {
			params.push_back(0.0);
		}

		action mode_abs {
			absolute = true;
		}
	
		action mode_rel {
			absolute = false;
		}
	
		action moveto {
			std::cout << ( absolute ? "M " : "m ");
			print_vec(params);
			std::cout << std::endl;
			params.clear();
		}	

		action closepath {
			std::cout << "z" << std::endl;
		}

		action lineto {
			std::cout << (absolute ? "L " : "l ");
			print_vec(params);
			std::cout << std::endl;
			params.clear();
		}

		action horizontal_lineto {
			std::cout << (absolute ? "H " : "h ");
			print_vec(params);
			std::cout << std::endl;
			params.clear();
		}

		action vertical_lineto {
			std::cout << (absolute ? "V " : "v ");
			print_vec(params);
			std::cout << std::endl;
			params.clear();
		}

		action curveto {
			std::cout << (absolute ? "C " : "c ");
			print_vec(params);
			std::cout << std::endl;
			params.clear();
		}

		action smooth_curveto {
			std::cout << (absolute ? "S " : "s ");
			print_vec(params);
			std::cout << std::endl;
			params.clear();
		}

		action quadratic_bezier_curveto {
			std::cout << (absolute ? "Q " : "q ");
			print_vec(params);
			std::cout << std::endl;
			params.clear();
		}

		action smooth_quadratic_bezier_curveto {
			std::cout << (absolute ? "T " : "t ");
			print_vec(params);
			std::cout << std::endl;
			params.clear();
		}

		action elliptical_arc {
			std::cout << (absolute ? "A " : "a ") << std::endl;
			print_vec(params);
			std::cout << std::endl;
			params.clear();
		}
		
		#wsp = ('\x20' | '\x9' | '\xD' | '\xA');
		wsp = ' ';
		sign = ('+' | '-');
		digit_sequence = digit+;
		exponent = ('e' | 'E') sign? digit_sequence;
		fractional_constant =
			digit_sequence? '.' digit_sequence
			| digit_sequence '.';
		floating_point_constant =
			fractional_constant exponent?
			| digit_sequence exponent;
		integer_constant = digit_sequence;
		comma = ',';
		comma_wsp = (wsp+ comma? wsp*) | (comma wsp*);

		flag = ('0' %push_false | '1' %push_true );
		
		number =
			( sign? integer_constant
			| sign? floating_point_constant )
			>start_number %push_number;

		nonnegative_number =
			( integer_constant
			| floating_point_constant)
			>start_number %push_number;

		coordinate = number;
		coordinate_pair = coordinate $1 %0 comma_wsp? coordinate;
		
		elliptical_arc_argument =
			(nonnegative_number $1 %0 comma_wsp?
			 nonnegative_number $1 %0 comma_wsp?
			 number comma_wsp
			 flag comma_wsp flag comma_wsp
			 coordinate_pair)
			%elliptical_arc;
		elliptical_arc_argument_sequence =
			elliptical_arc_argument $1 %0
			(comma_wsp? elliptical_arc_argument $1 %0)*;
		elliptical_arc =
			('A' %mode_abs| 'a' %mode_rel) wsp*
			elliptical_arc_argument_sequence;
		
		smooth_quadratic_bezier_curveto_argument =
			coordinate_pair %smooth_quadratic_bezier_curveto;
		smooth_quadratic_bezier_curveto_argument_sequence =
			smooth_quadratic_bezier_curveto_argument $1 %0
			(comma_wsp?
			 smooth_quadratic_bezier_curveto_argument $1 %0)*;
		smooth_quadratic_bezier_curveto =
			('T' %mode_abs| 't' %mode_rel) wsp*
			 smooth_quadratic_bezier_curveto_argument_sequence;

		quadratic_bezier_curveto_argument =
			(coordinate_pair $1 %0 comma_wsp? coordinate_pair)
			%quadratic_bezier_curveto;
		quadratic_bezier_curveto_argument_sequence =
			quadratic_bezier_curveto_argument $1 %0
			(comma_wsp? quadratic_bezier_curveto_argument $1 %0)*;
		quadratic_bezier_curveto =
			('Q' %mode_abs| 'q' %mode_rel) wsp* 
			quadratic_bezier_curveto_argument_sequence;

		smooth_curveto_argument =
			(coordinate_pair $1 %0 comma_wsp? coordinate_pair)
			%smooth_curveto;
		smooth_curveto_argument_sequence =
			smooth_curveto_argument $1 %0
			(comma_wsp? smooth_curveto_argument $1 %0)*;
		smooth_curveto =
			('S' %mode_abs| 's' %mode_rel)
			wsp* smooth_curveto_argument_sequence;

		curveto_argument =
			(coordinate_pair $1 %0 comma_wsp?
			 coordinate_pair $1 %0 comma_wsp?
			 coordinate_pair) 
			%curveto;
		curveto_argument_sequence =
			curveto_argument $1 %0
			(comma_wsp? curveto_argument $1 %0)*;
		curveto =
			('C' %mode_abs| 'c' %mode_rel)
			wsp* curveto_argument_sequence;

		vertical_lineto_argument = coordinate %vertical_lineto;
		vertical_lineto_argument_sequence =
			vertical_lineto_argument $1 %0
			(comma_wsp? vertical_lineto_argument $1 %0)*;
		vertical_lineto =
			('V' %mode_abs| 'v' %mode_rel)
			wsp* vertical_lineto_argument_sequence;

		horizontal_lineto_argument = coordinate %horizontal_lineto;
		horizontal_lineto_argument_sequence =
			horizontal_lineto_argument $1 %0
			(comma_wsp? horizontal_lineto_argument $1 %0)*;
		horizontal_lineto =
			('H' %mode_abs| 'h' %mode_rel)
			wsp* horizontal_lineto_argument_sequence;

		lineto_argument = coordinate_pair %lineto;
		lineto_argument_sequence =
			lineto_argument $1 %0
			(comma_wsp? lineto_argument $1 %0)*;
		lineto =
			('L' %mode_abs| 'l' %mode_rel) wsp*
			lineto_argument_sequence;

		closepath = ('Z' | 'z') %closepath;

		moveto_argument = coordinate_pair %moveto;
		moveto_argument_sequence =
			moveto_argument $1 %0
			(comma_wsp? lineto_argument $1 %0)*;
		moveto =
			('M' %mode_abs | 'm' %mode_rel)
			wsp* moveto_argument_sequence;

		drawto_command =
			closepath | lineto |
			horizontal_lineto | vertical_lineto |
			curveto | smooth_curveto |
			quadratic_bezier_curveto |
			smooth_quadratic_bezier_curveto |
			elliptical_arc;

		drawto_commands = drawto_command (wsp* drawto_command)*;
		moveto_drawto_command_group = moveto wsp* drawto_commands?;
		moveto_drawto_command_groups =
			moveto_drawto_command_group wsp*
			(wsp* moveto_drawto_command_group)*;

		svg_path = wsp* moveto_drawto_command_groups? wsp*;
                main := svg_path '\n';

		# Inintialize and execute.
		write init;
		write exec;
	}%%

	if ( cs != svgd_first_final )
		std::cerr << "state " << cs << ", but expected " << svgd_first_final << std::endl;
}


#define BUFSIZE 1024

int main()
{
	char buf[BUFSIZE];
	while ( fgets( buf, sizeof(buf), stdin ) != 0 ) {
		parse( buf );
	}
	return 0;
}
