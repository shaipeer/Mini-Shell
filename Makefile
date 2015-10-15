
# our compiler
CC=gcc

# compile arguments
CFLAGS+=-c -g -Wall

# linker flags
LDFLAGS+=


# our source files
SOURCES= main.c
# a macro to define the objects from sources
OBJECTS=$(SOURCES:.c=.o)

# executable name	
EXECUTABLE=ex1

$(EXECUTABLE): $(OBJECTS)
	@echo "      Building target '" $@ "'"
	@ $(CC) $(LDFLAGS) $(OBJECTS) -o $@

# a rule for generating object files given their c files
.c.o:
	@ $(CC) $(CFLAGS) $< -o $@

clean:
	@echo "Cleaning..."
	@ rm -rf *s *o $(EXECUTABLE)

.PHONY: all clean
run:
	@ ./test
