.PHONY: all clean

all: rapport

rapport: rapport.pdf

rapport.pdf: rapport.tex
	latexmk -pdf rapport.tex

clean:
	rm -f rapport.pdf
	latexmk -c rapport.tex
