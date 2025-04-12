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
GTPTH = googletest/googletest/
GMPTH = googletest/googlemock/
JPATH = cJSON/

BUILD_PATHS = $(BPATH) $(DPATH) $(OPATH) $(RPATH)

##### Executables
EXEC = microDNS

##### Options
CPPFLAGS =
CFLAGS = -pipe -Wall -Wextra -Werror -std=iso9899:2018 -pedantic -I$(HPATH) -I$(JPATH)
CXXFLAGS = -pipe -Wall -Wextra -Werror -std=c++20 -pedantic -I$(HPATH) -I$(JPATH) -I$(GTPTH)include/ -I$(GMPTH)include/ -I$(GTPTH) -I$(GMPTH)
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
SRCT = $(wildcard $(TPATH)*.cpp)
DEP = $(addprefix $(DPATH), $(patsubst %.cpp,%.d, $(notdir $(SRC)))) $(addprefix $(DPATH), $(patsubst %.cpp,%.d, $(notdir $(SRCT))))

##### Tests
RESULTS = $(patsubst $(TPATH)Test%.cpp,$(RPATH)Test%.txt,$(SRCT) )
EXIT_STAT_TMP_FILE = $(RPATH)test_exit_stat.tmp

##### Documentation generation
DOCGEN = doxygen
DOXYFILE = doc/Doxyfile
DOCPATH = doc/public/

##### Various
### Protection against interruptions during the compilation
POSTCOMPILE = mv -f $(DPATH)$*.Td $(DPATH)$*.d && touch $@

##### Build rules
.PHONY: help all test doc clean cleandoc cleanall

help:
	@echo ""
	@echo "make [DEBUG=<1|{0}>] target"
	@echo ""
	@echo "      target :	help		this message"
	@echo " 		$(EXEC)	build project executable"
	@echo " 		all		build '$(EXEC)' executable and run tests"
	@echo " 		test		run the unit tests"
	@echo " 		doc		generate code documentation"
	@echo " 		clean		remove binaries and test results"
	@echo " 		cleandoc	remove generated documentation artifacts"
	@echo " 		cleanall	remove all generated files, start anew"
	@echo ""

$(EXEC): $(OBJ) $(OPATH)cJSON.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

all: $(EXEC) test

$(OPATH)%.o:: $(SPATH)%.cpp $(DPATH)%.d | $(OPATH) $(DPATH)
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) $(DEPFLAGS) -o $@ $<
	@$(POSTCOMPILE)

$(OPATH)%.o:: $(TPATH)%.cpp $(DPATH)%.d | $(OPATH) $(DPATH)
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) $(DEPFLAGS) -o $@ $<
	@$(POSTCOMPILE)

$(OPATH)%.o:: $(GTPTH)src/%.cc | $(OPATH)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$(OPATH)%.o:: $(GMPTH)src/%.cc | $(OPATH)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$(OPATH)cJSON.o: $(JPATH)cJSON.c $(JPATH)cJSON.h | $(OPATH)
	$(CC) -c $(CFLAGS) -o $@ $<

test: $(BUILD_PATHS) $(RESULTS)
	@[ -s $(EXIT_STAT_TMP_FILE) ] && VAL=$$(cat $(EXIT_STAT_TMP_FILE)) || VAL=0; rm -f $(EXIT_STAT_TMP_FILE); exit $$VAL

$(RPATH)%.txt: $(BPATH)%
	@echo "-----------------------\nRUNNING [$<]:\n"
	@./$< || echo $$? > $(EXIT_STAT_TMP_FILE)
	@echo "-----------------------"

$(BPATH)Test%: $(OPATH)Test%.o $(OPATH)%.o $(OPATH)gtest-all.o $(OPATH)gtest_main.o $(OPATH)cJSON.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(OPATH):
	mkdir -p $@

$(DPATH):
	mkdir -p $@

$(RPATH):
	mkdir -p $@

$(BPATH):
	mkdir -p $@

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
