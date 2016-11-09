#  See http://www.gnu.org/software/make/manual/html_node/Static-Usage.html
#  And http://stackoverflow.com/questions/13552575/gnu-make-pattern-to-build-output-in-different-directory-than-src
#  Nice intro: http://www.jfranken.de/homepages/johannes/vortraege/make_inhalt.en.html
#  Google for "make text replacement functions"
#  Refer to above links for more info
#  This is the 4th version of the Makefile.txt
#  This makefile is for c++ !!

#  "make" notes:
#   $@  current target
#   $<  current prerequisite
#   %   is placeholder to match word

# The PROGNAME should match the folder where the project is located
# Else the makebuild_and_run.sh script will not work properly
PROGNAME = myprog

# The OUTDIR is assumed to be "build-"$PROGNAME by makebuild_and_run.sh script
OUTDIR = build-$(PROGNAME)

# add more files here as the system grows
SRC_FILES = main.cpp


CXX=g++
FLAGS=-Wall -Wextra -std=c++11 -g
LIBS=-lm

# Take the source files basenames. (Without ".cxx" suffix)
# Add ".o" suffix and "output/" directory prefix.
OBJ_FILES = $(addprefix $(OUTDIR)/, $(addsuffix .o, $(basename $(SRC_FILES))))

# The program file is placed in the OUTDIR (because of -o $(OUTDIR)/$@)
$(PROGNAME): $(OBJ_FILES)
	$(CXX) $(FLAGS) -o $(OUTDIR)/$@ $(OBJ_FILES) $(LIBS)

# Be careful here, the obj file is recompiled ONLY when its .c file changes.
$(OBJ_FILES): $(OUTDIR)/%.o: %.cpp
	$(CXX) -c $(FLAGS) $< -o $@
