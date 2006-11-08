
unix:
	bison -d basicParse.y
	flex basicParse.l
	cp basicParse.tab.* ../
	cp lex.yy.c ../

win: basicParse.l basicParse.y
	bison -d basicParse.y
	flex basicParse.l 
	moveStuff.bat

