#include "benchmark.hpp"
#include "trie.hpp"

#include <fstream>
#include <iostream>

int main( int argc, char* argv[])
{
  test_trie();
  
  if ( argc != 2 )
    {
      std::cerr << "Usage: " << argv[ 0 ] << " word_list_file\n";
      return 1;
    }
  
  std::ifstream f( argv[ 1 ] );
  bench_all( std::cout, f );
  
  return 0;
}
