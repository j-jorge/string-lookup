FLAGS=-DNDEBUG -O3
#FLAGS=-D_DEBUG -g

INCLUDES=-I./include \
	-I./marisa-trie/include \
	-I./marisa-trie/lib/

SOURCES=`find src -name "*.cpp"` \
	`find marisa-trie/lib/ -name "*.cc"`

all:
	g++ $(FLAGS) $(INCLUDES) $(SOURCES) -o bench
