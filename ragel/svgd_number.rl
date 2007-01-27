#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <cmath>
#include <vector>

%%{
	machine svgd_number;
	write data noerror;
}%%

void print_vec(std::vector<double> p) {
	for(int i=0;i < p.size(); i++)
	{
		std::cout<<p.at(i)<< " ";;
	}
}

double svgd_number( char *str )
{
	char *p = str;
	int cs = 0;
	int neg = 1;
	int neg_exp = 1;
	double whole = 0;
	double fract = 0;
	double exp = 0;
	int place = 0;
	double val = 0;
	bool absolute = false;
	std::vector<double> params;

	%%{
		action see_neg {
			neg = -1;
		}

		action see_neg_exp {
			neg_exp = -1;
		}

		action add_digit { 
			whole = whole * 10 + (fc - '0');
		}
		
		action add_fract_digit {
			fract = fract * 10 + (fc - '0');
			place--;
		}
		
		action add_exp_digit {
			exp = exp * 10 + (fc - '0');
		}

		action push_number {
			params.push_back(neg * (whole + (fract * pow(10, place * 1.0))) * pow(10, neg_exp * exp));
			neg = neg_exp = 1;
			place = 0;
			whole = fract = exp = 0;
		}

		action push_flag {
			if (fc == '0') {
				params.push_back(0.0);
			} else {
				params.push_back(1.0);
			}
		}

		action mode_abs {
			absolute = true;
		}
	
		action mode_rel {
			absolute = false;
		}
	
		action emit_moveto {
			std::cout << "m ";
			print_vec(params);
			std::cout << std::endl;
			params.clear();
		}	
		action emit_closepath {
			std::cout << "z" << std::endl;
			params.clear();
		}
		action emit_lineto {
			std::cout << "l ";
			print_vec(params);
			std::cout << std::endl;
			params.clear();
		}
		action emit_horizontal_lineto {
			std::cout << "h" << std::endl;
			params.clear();
		}
		action emit_vertical_lineto {
			std::cout << "v" << std::endl;
			params.clear();
		}
		action emit_curveto {
			std::cout << "c" << std::endl;
			params.clear();
		}
		action emit_smooth_curveto {
			std::cout << "s" << std::endl;
			params.clear();
		}
		action emit_quadratic_bezier_curveto {
			std::cout << "q" << std::endl;
			params.clear();
		}
		action emit_smooth_quadratic_bezier_curveto {
			std::cout << "t" << std::endl;
			params.clear();
		}
		action emit_elliptical_arc {
			std::cout << "a" << std::endl;
			params.clear();
		}
		
		
		#wsp = ('\x20' | '\x9' | '\xD' | '\xA');
		wsp = ' ';
		sign = ('+' | '-' @see_neg);
		exp_sign = ('+' | '-' @see_neg_exp);
		digits = (digit @add_digit)+;
		fract_digits = (digit @add_fract_digit)+;
		exp_digits = (digit @add_exp_digit)+;
		exponent = ('e' | 'E') exp_sign? exp_digits;
		fractional_constant = (digits? '.' fract_digits)
			| (digits '.');
		floating_point_constant = fractional_constant exponent?
			| digits exponent;
		number = ((sign? floating_point_constant) | (sign? digits)) @push_number;
		nonnegative_number = (floating_point_constant | digits) @push_number;
		comma = ',';
		comma_wsp = (wsp+ comma? wsp*) | (comma wsp*);
		flag = ('0' | '1') @push_flag;
		coordinate = number;
		coordinate_pair = number comma_wsp? number;
		
		elliptical_arc_argument = (nonnegative_number comma_wsp? nonnegative_number comma_wsp?
			number comma_wsp flag comma_wsp flag comma_wsp coordinate_pair) @emit_elliptical_arc;
		elliptical_arc_argument_sequence = elliptical_arc_argument (comma_wsp? elliptical_arc_argument)*;
		elliptical_arc = ('A' @mode_abs| 'a' @mode_rel) wsp* elliptical_arc_argument_sequence;
		
		smooth_quadratic_bezier_curveto_argument = coordinate_pair @emit_smooth_quadratic_bezier_curveto;
		smooth_quadratic_bezier_curveto_argument_sequence = smooth_quadratic_bezier_curveto_argument 
			(comma_wsp? smooth_quadratic_bezier_curveto_argument)*;
		smooth_quadratic_bezier_curveto = ('T' @mode_abs| 't' @mode_rel) wsp*
			 smooth_quadratic_bezier_curveto_argument_sequence;

		quadratic_bezier_curveto_argument = (coordinate_pair comma_wsp? coordinate_pair) 
			@emit_quadratic_bezier_curveto;
		quadratic_bezier_curveto_argument_sequence = quadratic_bezier_curveto_argument 
			(comma_wsp? quadratic_bezier_curveto_argument)*;		
		quadratic_bezier_curveto = ('Q' @mode_abs| 'q' @mode_rel) wsp* 
			quadratic_bezier_curveto_argument_sequence;

		smooth_curveto_argument = (coordinate_pair comma_wsp? coordinate_pair) @emit_smooth_curveto;
		smooth_curveto_argument_sequence = smooth_curveto_argument (comma_wsp? smooth_curveto_argument)*;
		smooth_curveto = ('S' @mode_abs| 's' @mode_rel) wsp* smooth_curveto_argument_sequence;

		curveto_argument = (coordinate_pair comma_wsp? coordinate_pair comma_wsp? coordinate_pair) 
			@emit_curveto;
		curveto_argument_sequence = curveto_argument (comma_wsp? curveto_argument)*;
		curveto = ('C' @mode_abs| 'c' @mode_rel) wsp* curveto_argument_sequence;

		vertical_lineto_argument = coordinate @emit_vertical_lineto;
		vertical_lineto_argument_sequence = vertical_lineto_argument 
			(comma_wsp? vertical_lineto_argument)*;
		vertical_lineto = ('V' @mode_abs| 'v' @mode_rel) wsp* vertical_lineto_argument_sequence;

		horizontal_lineto_argument = coordinate @emit_horizontal_lineto;
		horizontal_lineto_argument_sequence = horizontal_lineto_argument 
			(comma_wsp? horizontal_lineto_argument)*;
		horizontal_lineto = ('H' @mode_abs| 'h' @mode_rel) wsp* horizontal_lineto_argument_sequence;

		lineto_argument = coordinate_pair @emit_lineto;
		lineto_argument_sequence = lineto_argument (comma_wsp? lineto_argument)*;
		lineto = ('L' @mode_abs| 'l' @mode_rel) wsp* lineto_argument_sequence;

		closepath = ('Z' | 'z') @emit_closepath;

		moveto_argument = coordinate_pair @emit_moveto;
		moveto_argument_sequence = moveto_argument (comma_wsp? lineto_argument)*;
		moveto = ('M' @mode_abs| 'm' @mode_rel) wsp* moveto_argument_sequence;

		drawto_command = closepath ;#| lineto ; #| horizontal_lineto | vertical_lineto
#			| curveto | smooth_curveto | quadratic_bezier_curveto
#			| smooth_quadratic_bezier_curveto | elliptical_arc;

		drawto_commands = drawto_command (wsp* drawto_command)*;
		moveto_drawto_command_group = moveto wsp* drawto_commands?;
		moveto_drawto_command_groups = moveto_drawto_command_group (wsp* moveto_drawto_command_group)*;

		#main := wsp* moveto_drawto_command_groups wsp* '\n';
		main := moveto_drawto_command_groups '\n';

		# Inintialize and execute.
		write init;
		write exec noend;
	}%%

	if ( cs < svgd_number_first_final )
		std::cerr << "svgd_number: there was an error" << std::endl;
	
	return 1.0;
};


#define BUFSIZE 1024

int main()
{
	char buf[BUFSIZE];
	while ( fgets( buf, sizeof(buf), stdin ) != 0 ) {
		double value = svgd_number( buf );
		std::cout << value << std::endl;
	}
	return 0;
}
