OBJDIR=target
SRCDIR=src
TARGET=$(OBJDIR)/main

OPT=s

# Define all source files.
SRC=$(call rwildcard, $(SRCDIR), *.c)

# Define all object files.
OBJ=$(SRC:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# Library Options
MATH_LIB=-lm
SIMAVR_LIB=-lsimavr -lelf
OPENSSL_LIB=-L/usr/local/opt/openssl/lib
WEBSOCKETS_LIB=-L/opt/libwebsockets/lib -lwebsockets

# Define header directories.
VPATH=$(call uniq,$(dir $(SRC)))
EXTRAINCDIRS=$(call uniq,$(dir $(call rwildcard,$(SRCDIR),*.h)))
SIMAVR_HEADERS=/Users/voddenr/eclipse-workspace/simavr/simavr/sim
OPENSSL_HEADERS=/usr/local/opt/openssl/include
WEBSOCKETS_HEADERS=/opt/libwebsockets/include

CSTANDARD= -std=gnu99

# C Options
# CFLAGS = -g$(DEBUG)
CFLAGS = -O$(OPT)
CFLAGS += -pedantic
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wmissing-prototypes
CFLAGS += -Wunreachable-code
CFLAGS += -Wsign-compare
CFLAGS += -Wno-zero-length-array
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))
CFLAGS += -I$(SIMAVR_HEADERS)
CFLAGS += -I$(OPENSSL_HEADERS)
CFLAGS += $(CSTANDARD)

# Linker Options
LDFLAGS = -Wl 
LDFLAGS += $(patsubst %,-L%,$(EXTRALIBDIRS))
LDFLAGS += $(PRINTF_LIB) $(SCANF_LIB) $(MATH_LIB) $(SIMAVR_LIB) 
LDFLAGS += $(OPENSSL_LIB) $(WEBSOCKETS_LIB)

# Define programs and commands.
SHELL = bash
CC = gcc
OBJCOPY = objcopy
OBJDUMP = objdump
SIZE = size
AR = ar rcs
AVRDUDE = avrdude
REMOVE = rm -f
REMOVEDIR = rm -rf
NM = nm
COPY = cp
BEAR = bear
HEAD = head
MAKE = make

# Compiler flags to generate dependency files.
GENDEPFLAGS = -MMD -MP -MF .dep/$(@F).d

# Combine all necessary flags and optional flags.
# Add target processor to flags.
ALL_CFLAGS = -I. $(CFLAGS) $(GENDEPFLAGS)

# Default target.
all: gccversion build

debug: CFLAGS += -DDEBUG -g
debug: OPT=0
debug: build

# Change the build target to build a HEX file or a library.
build: $(TARGET)

# Display compiler version information.
gccversion :
	@echo -n "Using "
	@$(CC) --version | $(HEAD) -1

# Link: create ELF output file from object files.
.SECONDEXPANSION:
.SECONDARY : $(TARGET)
.PRECIOUS : $$(OBJ)
$(TARGET): $$(OBJ)
	@echo -n Creating $@ from object files...
	@$(CC) $(ALL_CFLAGS) $^ --output $@ $(LDFLAGS)
	@echo done.

# Compile: create object files from C source files.
$(OBJDIR)/%.o : %.c
	@echo -n Compiling $^ to object file...
	@mkdir -p $(dir $@)
	@$(CC) -c $(ALL_CFLAGS) $< -o $@
	@echo done.

# Target: clean project.
clean: clean_list

clean_list :
	@echo -n "Cleaning..."
	@$(REMOVE) $(OBJ)
	@$(REMOVE) $(SRC:.c=.d)
	@$(REMOVEDIR) .dep
	@$(REMOVEDIR) $(OBJDIR)
	@echo "...done."

# Include the dependency files.
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

compile_commands.json: Makefile
	$(MAKE) clean
	$(BEAR) -v --cdb $@ $(MAKE)

# Listing of phony targets.
.PHONY : all gccversion build clean clean_list debug

# Make does not offer a recursive wildcard function, so here's one:
define rwildcard=
	$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))
endef

define uniq=
	$(if $1,$(firstword $1) $(call uniq,$(filter-out $(firstword $1),$1)))
endef

