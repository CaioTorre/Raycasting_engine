IDIR=include
CC=gcc
CP=g++
CFLAGS=-I$(IDIR) -pthread

ODIR=obj
LDIR=lib
SDIR=src

LIBS=-lglfw3 -lm -lglut -lGL -lGLU -lGLEW -lXrandr -lXi -lX11 -lXxf86vm -lpthread 

RESULT=aesthetic.exe

_DEPS = bazinho.h raycast.h vec3_lib.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = bazinho.o raycast.o vec3_lib.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SDIR)/%.cpp $(DEPS)
	$(CP) -c -o $@ $< $(CFLAGS)

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CP) -c -o $@ $< $(CFLAGS)

main: $(OBJ)
	$(CP) -o $(RESULT) $(SDIR)/main.cpp $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
	rm -f $(RESULT)
