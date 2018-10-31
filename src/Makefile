GENERATOR=generator
GENERATOR-DEBUG=generator-debug
BGENERATOR=bmc-generator
BGENERATOR-DEBUG=bmc-generator-debug
Z3=z3-bmc-generator
Z3-DEBUG=z3-bmc-generator-debug

CC = gcc
WARNINGS = -Wall -Wextra -Werror -Wno-error=unused-parameter -Wmissing-declarations
CFLAGS   = -D_GNU_SOURCE -std=gnu11 
DEBUG	 = -g

all: $(GENERATOR) $(BGENERATOR) $(Z3)
debug: $(Z3-DEBUG) $(BGENERATOR-DEBUG)
bmc: $(BGENERATOR) $(Z3)

$(GENERATOR): generator.c
	$(CC) $^ $(CFLAGS) -o $(GENERATOR)

$(GENERATOR-DEBUG): generator.c
	$(CC) $^ $(CFLAGS) $(DEBUG) -o $(GENERATOR-DEBUG)

$(BGENERATOR): bmc-generator.c
	$(CC) $^ $(CFLAGS) -o $(BGENERATOR)

$(BGENERATOR-DEBUG): bmc-generator.c
	$(CC) $^ $(CFLAGS) $(DEBUG) -o $(BGENERATOR-DEBUG)

$(Z3): z3-bmc-generator.c
	$(CC) $^ $(CFLAGS) $(WARNINGS) -o $(Z3)

$(Z3-DEBUG): z3-bmc-generator.c
	$(CC) $^ $(CFLAGS) $(DEBUG) $(WARNINGS) -o $(Z3-DEBUG)

clean:
	$(RM) $(GENERATOR) $(GENERATOR-DEBUG) $(BGENERATOR) $(BGENERATOR-DEBUG) $(Z3) $(Z3-DEBUG)
