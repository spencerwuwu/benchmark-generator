GENERATOR=generator
GENERATOR-DEBUG=generator-debug
DGENERATOR=dual-generator
DGENERATOR-DEBUG=dual-generator-debug
BGENERATOR=bmc-generator
BGENERATOR-DEBUG=bmc-generator-debug
Z3=z3-bmc-generator
Z3-DEBUG=z3-bmc-generator-debug

CC = gcc
WARNINGS = -Wall -Wextra -Werror -Wno-error=unused-parameter -Wmissing-declarations
CFLAGS   = -D_GNU_SOURCE -std=gnu11 
DEBUG	 = -g

all: $(GENERATOR) $(DGENERATOR) $(BGENERATOR) $(Z3)
debug: $(Z3-DEBUG) $(BGENERATOR-DEBUG)
bmc: $(BGENERATOR)
z3: $(Z3)

$(GENERATOR): generator.c
	$(CC) $^ $(CFLAGS) -o $(GENERATOR)

$(GENERATOR-DEBUG): generator.c
	$(CC) $^ $(CFLAGS) $(DEBUG) -o $(GENERATOR-DEBUG)

$(DGENERATOR): dual-generator.c
	$(CC) $^ $(CFLAGS) -o $(DGENERATOR)

$(DGENERATOR-DEBUG): dual-generator.c
	$(CC) $^ $(CFLAGS) $(DEBUG) -o $(DGENERATOR-DEBUG)

$(BGENERATOR): bmc-generator.c
	$(CC) $^ $(CFLAGS) -o $(BGENERATOR)

$(BGENERATOR-DEBUG): bmc-generator.c
	$(CC) $^ $(CFLAGS) $(DEBUG) -o $(BGENERATOR-DEBUG)

$(Z3): z3-bmc-generator.c
	$(CC) $^ $(CFLAGS) $(WARNINGS) -o $(Z3)

$(Z3-DEBUG): z3-bmc-generator.c
	$(CC) $^ $(CFLAGS) $(DEBUG) $(WARNINGS) -o $(Z3-DEBUG)

clean:
	$(RM) $(GENERATOR) $(GENERATOR-DEBUG) $(DGENERATOR) $(DGENERATOR-DEBUG) $(BGENERATOR) $(BGENERATOR-DEBUG) $(Z3) $(Z3-DEBUG)
