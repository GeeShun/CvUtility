TARGET := harris.out
CXX := g++
CXXFLAGS := -std=c++11 -Wall -g -fsanitize=address
INCLUDES := -I/usr/local/include -I../../imageUtility -I../../featureDetect -I../../common
LIBS := -L/usr/local/lib -ljpeg -lm
SRCDIRS := ../../featureDetect ../../imageUtility .
SRCS := $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.cpp))
OBJS := $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(LIBS) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) *.jpg .gdb_history


