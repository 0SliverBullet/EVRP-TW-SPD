SRC_DIR = src
BUILD_DIR = build
TARGET = $(BUILD_DIR)/evrp-tw-spd

SRC = $(wildcard $(SRC_DIR)/*.cpp)

OBJ = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRC))
DEPS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.d,$(SRC))

CXX = g++
CXXFLAGS = -std=c++11 \
		   -O3 -fdiagnostics-color=always \
           -I/home/wangdx_lab/cse12112328/opt/ibm/ILOG/CPLEX_Studio201/cplex/include \
           -I/home/wangdx_lab/cse12112328/opt/ibm/ILOG/CPLEX_Studio201/concert/include
LDFLAGS = -L/home/wangdx_lab/cse12112328/opt/ibm/ILOG/CPLEX_Studio201/cplex/lib/x86-64_linux/static_pic \
          -L/home/wangdx_lab/cse12112328/opt/ibm/ILOG/CPLEX_Studio201/concert/lib/x86-64_linux/static_pic
LDLIBS = -lconcert -lilocplex -lcplex -lm -lpthread -ldl

all: $(TARGET)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)         
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(TARGET): $(OBJ)
	@mkdir -p $(dir $@)         
	$(CXX) $(CXXFLAGS) $(OBJ) $(LDFLAGS) $(LDLIBS) -o $@

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)
