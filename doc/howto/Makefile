all: de

de: FidoNode.html FidoNode.dvi FidoNode.ps FidoNode.txt FidoNode.info 

html: FidoNode.html

dvi: FidoNode.dvi

ps: FidoNode.ps

txt: FidoNode.txt

info: FidoNode.info

FidoNode.html: FidoNode.sgml
	sgml2html --language=de FidoNode.sgml

FidoNode.tex: FidoNode.sgml
	sgml2latex -p a4 -o tex FidoNode.sgml

FidoNode.dvi: FidoNode.sgml
	sgml2latex -p a4 -o dvi FidoNode.sgml

FidoNode.ps: FidoNode.sgml
	sgml2latex -p a4 -o ps FidoNode.sgml

FidoNode.txt: FidoNode.sgml
	sgml2txt --language=de FidoNode.sgml

FidoNode.info: FidoNode.sgml
	sgml2info --language=de FidoNode.sgml

clean:
	rm -f FidoNode.sgml~ FidoNode*.html FidoNode.dep
	rm -f FidoNode.dvi FidoNode.ps FidoNode.aux FidoNode.tex
	rm -f FidoNode.aux FidoNode.log FidoNode.txt FidoNode.toc
	rm -f FidoNode.info* FidoNode.toc.bak

