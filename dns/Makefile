# Inspired partly by Unity unit test framework makefile on 29 May 2024:
# https://www.throwtheswitch.org/s/makefile
# https://www.throwtheswitch.org/build/make

##### Common
CC = gcc
CXX = g++

##### Paths
SPATH = src/
HPATH = include/
BPATH = build/
OPATH = $(BPATH)obj/
DPATH = $(BPATH)dep/
RPATH = $(BPATH)results/
TPATH = test/
UPATH = unity/src/
JPATH = cJSON/

BUILD_PATHS = $(BPATH) $(DPATH) $(OPATH) $(RPATH)

##### Executables
EXEC = microDNS

##### Options
CPPFLAGS =
CFLAGS = -Wall -Wextra -Werror -std=iso9899:2018 -pedantic -I$(HPATH) -I$(JPATH) -I$(UPATH)
CXXFLAGS = -Wall -Wextra -Werror -std=c++20 -pedantic -I$(HPATH) -I$(JPATH) -I$(UPATH)
LDFLAGS =
LDLIBS =
DEPFLAGS = -MT $@ -MMD -MP -MF $(DPATH)$*.Td

# Use `make DEBUG=0` (or nothing) and `make DEBUG=1` to switch
DEBUG ?= 0
ifeq ($(DEBUG), 1)
	CPPFLAGS += -DDEBUG
	CFLAGS += -g
	CXXFLAGS += -g
else
	CPPFLAGS += -DNDEBUG
	CFLAGS += -march=native -O3
	CXXFLAGS += -march=native -O3
endif

##### Files
SRC = $(wildcard $(SPATH)*.cpp)
HDR = $(wildcard $(HPATH)*.hpp)
OBJ = $(addprefix $(OPATH), $(patsubst %.cpp,%.o, $(notdir $(SRC))))
DEP = $(addprefix $(DPATH), $(patsubst %.cpp,%.d, $(notdir $(SRC)))) $(addprefix $(DPATH), $(patsubst %.c,%.d, $(notdir $(SRCT))))
SRCT = $(wildcard $(TPATH)*.c)

##### Tests
RESULTS = $(patsubst $(TPATH)Test%.c,$(RPATH)Test%.txt,$(SRCT) )

PASSED = `grep -s PASS $(RPATH)*.txt`
FAIL = `grep -s FAIL $(RPATH)*.txt`
IGNORE = `grep -s IGNORE $(RPATH)*.txt`

EXIT_STAT_TMP_FILE = $(RPATH)test_exit_stat.tmp

##### Documentation generation
DOCGEN = doxygen
DOXYFILE = doc/Doxyfile
DOCPATH = doc/public/

##### Various
### Protection against interruptions during the compilation
POSTCOMPILE = mv -f $(DPATH)$*.Td $(DPATH)$*.d && touch $@

##### Build rules
.PHONY: all test doc clean cleandoc cleanall FORCE

$(EXEC): $(OBJ) $(OPATH)cJSON.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

all: $(EXEC) test

$(OPATH)%.o:: $(SPATH)%.cpp $(DPATH)%.d | $(OPATH) $(DPATH)
	$(CXX) -c $(CFLAGS) $(CPPFLAGS) $(DEPFLAGS) -o $@ $<
	@$(POSTCOMPILE)

$(OPATH)%.o:: $(TPATH)%.c $(DPATH)%.d | $(OPATH) $(DPATH)
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(DEPFLAGS) -o $@ $<
	@$(POSTCOMPILE)

$(OPATH)%.o:: $(UPATH)%.c $(UPATH)%.h | $(OPATH)
	$(CC) -c $(CFLAGS) -o $@ $<

$(OPATH)cJSON.o:: $(JPATH)cJSON.c $(JPATH)cJSON.h | $(OPATH)
	$(CC) -c $(CFLAGS) -o $@ $<

test: $(BUILD_PATHS) $(RESULTS)
	@echo "-----------------------\nIGNORES:\n-----------------------"
	@echo "$(IGNORE)"
	@echo "-----------------------\nFAILURES:\n-----------------------"
	@echo "$(FAIL)"
	@echo "-----------------------\nPASSED:\n-----------------------"
	@echo "$(PASSED)"
	@echo "\nDONE"
	@[ -s $(EXIT_STAT_TMP_FILE) ] && VAL=$$(cat $(EXIT_STAT_TMP_FILE)) || VAL=0; rm -f $(EXIT_STAT_TMP_FILE); exit $$VAL

$(RPATH)%.txt: $(BPATH)% FORCE
	-./$< > $@ 2>&1 || echo $$? > $(EXIT_STAT_TMP_FILE)

$(BPATH)Test%: $(OPATH)Test%.o $(OPATH)%.o $(OPATH)unity.o $(OPATH)cJSON.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(OPATH):
	mkdir -p $@

$(DPATH):
	mkdir -p $@

$(RPATH):
	mkdir -p $@

$(BPATH):
	mkdir -p $@

FORCE:

$(DEP):

-include $(DEP)

clean:
	rm -f $(EXEC) $(OBJ) $(DEP) $(RESULTS) $(EXIT_STAT_TMP_FILE)

cleandoc:
	rm -rf $(DOCPATH)

cleanall: clean
cleanall: cleandoc
cleanall:
	rm -rf $(BUILD_PATHS)

doc: $(DOCPATH)

$(DOCPATH): $(SRC) $(HDR) README.md $(DOXYFILE)
	DOXY_OUT=$@ $(DOCGEN) $(DOXYFILE)

.PRECIOUS: $(BPATH)Test%
.PRECIOUS: $(DPATH)%.d
.PRECIOUS: $(OPATH)%.o
.PRECIOUS: $(RPATH)%.txt
