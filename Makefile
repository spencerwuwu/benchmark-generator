GENERATOR=generator
GENERATOR-DEBUG=generator-debug
BGENERATOR=bmc-generator
BGENERATOR-DEBUG=bmc-generator-debug
SMT=smt-bmc-generator
SMT-DEBUG=smt-bmc-generator-debug

CC = gcc
WARNINGS = -Wall -Wextra -Werror -Wno-error=unused-parameter -Wmissing-declarations
CFLAGS   = -D_GNU_SOURCE -std=gnu11 
DEBUG	 = -g

all: $(GENERATOR) $(BGENERATOR) $(SMT)
debug: $(SMT-DEBUG) $(BGENERATOR-DEBUG)
bmc: $(BGENERATOR) $(SMT)

$(GENERATOR): generator.c
	$(CC) $^ $(CFLAGS) -o $(GENERATOR)

$(GENERATOR-DEBUG): generator.c
	$(CC) $^ $(CFLAGS) $(DEBUG) -o $(GENERATOR-DEBUG)

$(BGENERATOR): bmc-generator.c
	$(CC) $^ $(CFLAGS) -o $(BGENERATOR)

$(BGENERATOR-DEBUG): bmc-generator.c
	$(CC) $^ $(CFLAGS) $(DEBUG) -o $(BGENERATOR-DEBUG)

$(SMT): smt-bmc-generator.c
	$(CC) $^ $(CFLAGS) $(WARNINGS) -o $(SMT)

$(SMT-DEBUG): smt-bmc-generator.c
	$(CC) $^ $(CFLAGS) $(DEBUG) $(WARNINGS) -o $(SMT-DEBUG)

clean:
	$(RM) $(GENERATOR) $(GENERATOR-DEBUG) $(BGENERATOR) $(BGENERATOR-DEBUG) $(SMT) $(SMT-DEBUG)
