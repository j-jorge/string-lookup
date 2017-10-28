FLAGS=-DNDEBUG -O3
#FLAGS=-D_DEBUG -g
CXX=g++

INCLUDES=-I./include \
	-I./marisa-trie/include \
	-I./marisa-trie/lib/

SOURCES=`find src -name "*.cpp"` \
	`find marisa-trie/lib/ -name "*.cc"`

all:
	$(CXX) -std=c++11 $(FLAGS) $(INCLUDES) $(SOURCES) -o bench
