# Makefile for keyValueStore
#  At the command line in this directory
#  'make' is short for...
#  'make all' which updates the executables
#  'make clean' cleans up
#  'make kvstore.tgz', 'make project.tgz', etc will
#    create a compressed archive file out of the important bits

also_archive = # extra files to put in archive, e.g. README notes.txt

# standard Make variables, used in implicit rules
CC     = gcc -std=c99 # gnu c compiler, 1999 version of the C language
CFLAGS = -Wall -lm # all warnings, link math

src  = $(wildcard *.c) # source files
head = $(wildcard *.h) # header files
obj  = $(src:.c=.o) # object files corresponding to source files

# source files containing "int main(...)"
# these will become standalone executables
exe_src = $(shell grep -El 'int\s*main\s*\(.*\)' $(src))
exe     = $(basename $(exe_src))

# source files without a main
# these are shared objects to be linked into executables
shared_src = $(filter-out $(exe_src), $(src))
shared     = $(shared_src:.c=.o)

# .PHONY indicates these are not target files to be built, they are commands
.PHONY: all clean

# the first target is the default, builds the executables
all: $(exe)

# targets ending in '.o' (object files) are handled by implicit rules,
# but we also want them to recompile if the headers change,
# or the makefile itself changes (edited CFLAGS perhaps)
*.o: $(head) $(lastword $(MAKEFILE_LIST))

# Each executable depends on the object file with that same name
# and any shared object files
# $^ is everything after the last ':', $@ is the executable name
$(exe): %: %.o $(shared)
	$(CC) $(CFLAGS) $^ -o $@

# delete the executables, any '.o' files, and any archives
clean:
	rm --force $(exe) $(wildcard *.o) $(wildcard *.t?z) $(wildcard *.tar.*)

# make a compressed tarball of the sources, headers, and this makefile
%.tgz %.tar.gz %.tbz %.tar.bz2: $(src) $(head) $(lastword $(MAKEFILE_LIST)) $(also_archive)
	tar --create --auto-compress --file $@ $^

SHELL := $(shell which bash) # tell Make to use the bash shell

# ensure the .gitignore file contains a rule for object files, and each executable.
# creates a list of the files to ignore, separates them into lines, then uses
# comm to find any lines not already in the .gitignore file, and appends them to it
.gitignore: $(src)
	@comm -23 <(echo '*.o' $(exe) | sed -e 's/\s\+/\n/g' | sort) <(sort $@) \
		| tee --append $@ | cat <(echo Adding lines to $@:) -
	@touch $@
