######################################################################
# Automatically generated by qmake (2.00a) Fri Sep 15 19:35:58 2006
######################################################################

TEMPLATE 		= 	app
TARGET 			=	BASIC256
DEPENDPATH 		+= 	.
INCLUDEPATH 	+= 	.
OBJECTS_DIR 	= 	tmp/obj
MOC_DIR 		= 	tmp/moc
TRANSLATIONS 	= 	Translations/basic256_en_US.ts \
					Translations/basic256_de.ts

# Input
HEADERS 		+= 	LEX/basicParse.tab.h 
HEADERS 		+= 	BasicEdit.h 
HEADERS 		+= 	Interpreter.h 
HEADERS 		+= 	RunController.h 
HEADERS 		+= 	BasicOutput.h 
HEADERS 		+= 	BasicGraph.h 
HEADERS 		+= 	GhostButton.h 
HEADERS 		+= 	PauseButton.h 
HEADERS 		+= 	DockWidget.h
HEADERS			+=	BasicWidget.h
HEADERS			+=	ToolBar.h
HEADERS			+=	ViewWidgetIFace.h
HEADERS			+=	MainWindow.h

SOURCES 		+= 	LEX/lex.yy.c 
SOURCES 		+= 	LEX/basicParse.tab.c 
SOURCES 		+= 	BasicEdit.cpp 
SOURCES 		+= 	Interpreter.cpp 
SOURCES 		+= 	RunController.cpp 
SOURCES 		+= 	Main.cpp 
SOURCES 		+= 	BasicOutput.cpp 
SOURCES 		+= 	BasicGraph.cpp 
SOURCES 		+= 	GhostButton.cpp 
SOURCES 		+= 	PauseButton.cpp 
SOURCES 		+= 	DockWidget.cpp
SOURCES			+=	BasicWidget.cpp
SOURCES			+=	ToolBar.cpp
SOURCES			+=	ViewWidgetIFace.cpp
SOURCES			+=	MainWindow.cpp
