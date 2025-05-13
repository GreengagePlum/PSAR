OUT= rapport.pdf slides.pdf
SRC= $(patsubst %.pdf,%.tex,$(OUT))

.PHONY: all clean

all: $(OUT)

%.pdf: %.tex
	latexmk -pdfxe $<

clean:
	rm -f $(OUT)
	# latexmk -c $(SRC)
	latexmk -c
