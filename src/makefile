CXX = g++
OBJECTS = execute.o Lexer_Aql.o Lexer_Input.o Parser.o
CXXFLAGS = -std=c++0x

run: build
	./execute ../dataset/PerLoc.aql ../dataset/PerLoc.input > ../dataset/PerLoc.output
	./execute ../dataset/Revenue.aql ../dataset/Revenue.input > ../dataset/Revenue.output
	./execute ../dataset/Revenue.aql ../dataset/Revenue.input > ../dataset/testCase.output
	./execute ../dataset/PerLoc.aql ../dataset/PerLoc.input
	./execute ../dataset/Revenue.aql ../dataset/Revenue.input
	./execute ../dataset/testCase.aql ../dataset/testCase.input

build: clean compile
	$(CXX) -o execute $(OBJECTS)

compile: $(OBJECTS)

clean:
	rm -f *.o execute
