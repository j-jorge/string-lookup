all:
	g++ -DNDEBUG -O3 -I./include `find src -name "*.cpp"` -o bench
