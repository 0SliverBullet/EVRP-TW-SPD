# define variables
CXX = g++
CXXFLAGS = -fdiagnostics-color=always \
           -I/home/wangdx_lab/cse12112328/opt/ibm/ILOG/CPLEX_Studio201/cplex/include \
           -I/home/wangdx_lab/cse12112328/opt/ibm/ILOG/CPLEX_Studio201/concert/include
LDFLAGS = -L/home/wangdx_lab/cse12112328/opt/ibm/ILOG/CPLEX_Studio201/cplex/lib/x86-64_linux/static_pic \
          -L/home/wangdx_lab/cse12112328/opt/ibm/ILOG/CPLEX_Studio201/concert/lib/x86-64_linux/static_pic
LDLIBS = -lconcert -lilocplex -lcplex -lm -lpthread -ldl

# target and source files
TARGET = blend                     # executable
SRC = blend.cpp                    # if there are other source files (other.cpp), you can add them
OBJ = $(SRC:.cpp=.o)               #  convert all .cpp to .o

# default target
all: $(TARGET)

# compile rule
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) $(LDFLAGS) $(LDLIBS) -o $(TARGET)

# how .cpp to .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# clean
clean:
	rm -f $(OBJ) $(TARGET)
