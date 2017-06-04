CC=gcc
CXX=g++
LD=g++

Cflags=-c -MD
CXXflags=-c -MD
LDflags=

Input=main.cpp expression.cpp equiv.cpp
ObjDir=obj
BinDir=bin
Output=createupn

Objects=$(addprefix $(ObjDir)/,$(addsuffix .o, $(Input)))

all: $(BinDir)/$(Output)
	cd $(BinDir); ./$(Output)
    
$(BinDir)/$(Output): $(Objects)
	$(LD) -o $(BinDir)/$(Output) $(Objects) $(LDflags)

obj/%.c.o: %.c
	$(CC) $(Cflags) -o $@ $<

obj/%.cpp.o: %.cpp
	$(CXX) $(CXXflags) -o $@ $<

-include $(ObjDir)/*.d

.IGNORE: clean
clean:
	rm $(ObjDir)/*
	rm $(BinDir)/$(Output)
