all: manual.pdf

manual.pdf: manual.tex
	pdflatex manual.tex
	bibtex manual
	pdflatex manual.tex
