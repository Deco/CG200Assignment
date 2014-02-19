# Makefile
# Builds to `bin/turtle`.
# Copies `res/*` to `bin/`. Put test drawings in `res/`.
# Typical workflow:
#	cd Assignment
#	lua design/prototype.lua drawing=r gen >res/drawing.txt
#	make debug && (cd ./bin/; valgrind --leak-check=full ./turtle drawing.txt)
# If you want to build release version:
# make clean && make
# 

default: full
debug: full
quick: build
quickdebug: build
profile: build

TARGET=cactusgame

SRC=src
OBJ=obj
BIN=bin
RES=res
LIB=lib

CC=g++
CCFLAGS =
#CCFLAGS =-D_GNU_SOURCE
#CCFLAGS+=-Wextra
#CCFLAGS+=-pedantic
#CCFLAGS+=-Weffc++
#CCFLAGS+=-Werror
#CCFLAGS+=-std=c99
CCFLAGS+=-std=c++0x
CCFLAGS+=-Isrc -IAntTweakBar/include -ISOIL/src
DBG_CCFLAGS=-ggdb -g3 -O0 -DDEBUG
#DBG_CCFLAGS+=-D_GLIBCXX_DEBUG
PRF_CCFLAGS=-pg

LD=g++
LDFLAGS =-L$(LIB)
LDFLAGS+=-lm -lGL -lGLU -lglut -lX11
DBG_LDFLAGS=

LDOBJS =$(LIB)/libAntTweakBar.a $(LIB)/libSOIL.a

SRCFILES:=$(shell find $(SRC) -type f -name "*.cpp")
DEPFILES:=$(patsubst $(SRC)/%.cpp,$(OBJ)/%.d,$(SRCFILES))
OBJFILES:=$(patsubst $(SRC)/%.cpp,$(OBJ)/%.o,$(SRCFILES))

$(OBJ)/%.o: $(SRC)/%.cpp Makefile
	$(CC) $(CCFLAGS) -MMD -c $< -o $@
# Generates dependency information for each C file's object file as a new
# Makefile. This has many benefits, and no tangible disadvantages!

-include $(DEPFILES)
#`-` suppresses warnings about the files not existing first time round

$(BIN)/$(TARGET): $(OBJFILES)
	$(LD) $(LDFLAGS) -o $@ $^ $(LDOBJS)

anttweakbar: .FORCE
	@echo "# Compiling AntTweakBar."
	cd AntTweakBar/src; make $(MFLAGS)
	cp AntTweakBar/lib/libAntTweakBar.a lib
	@echo "# Compiled AntTweakBar to $(DEP)/$(LIB)/libAntTweakBar.a"

tinyobjloader: .FORCE
	cp tiny_obj_loader.* src

soil: .FORCE
	@echo "# Compiling SOIL."
	cd SOIL/projects/makefile; make
	cp SOIL/lib/libSOIL.a lib
	@echo "# Compiled SOIL to $(LIB)/libSOIL.a."

build: $(BIN)/$(TARGET)
	@echo "# Compiled to $(BIN)/$(TARGET)."

full: anttweakbar soil res build

debug: CCFLAGS+=$(DBG_CCFLAGS)
quickdebug: CCFLAGS+=$(DBG_CCFLAGS)

profile: CCFLAGS+=$(PRF_CCFLAGS)

# -DDEBUG_DRAW -DDEBUG_PARSE

res: .FORCE
	cp -u res/* bin/
# Have the contents of 'res' changed? No idea! Copy them every time
# by using .FORCE
run:
	cd bin; /cactusgame


clean:
	cd AntTweakBar/src; make clean
	cd SOIL/projects/makefile; make clean
	$(RM) $(wildcard $(OBJ)/* $(BIN)/*)

quickclean:
	$(RM) $(wildcard $(OBJ)/* $(BIN)/*)

.PHONY: default clean
.FORCE:
