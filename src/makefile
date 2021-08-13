IDIR=../include
CC=g++-9.2
CPPFLAGS=-I$(IDIR)

ODIR=obj

_DEPS=deslayer.h event.h process.h scheduler.h
DEPS=$(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ=process.o event.o deslayer.o scheduler.o sched.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ)) 

$(ODIR)/%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CPPFLAGS)

sched: $(OBJ) 
	$(CC) -o $@ $^ $(CPPFLAGS)

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~