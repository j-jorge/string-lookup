#include "boggox/dictionary.hpp"
#include "marisa/trie.h"
#include "trie.hpp"

#include <chrono>
#include <fstream>
#include <iostream>

static constexpr std::size_t g_runs( 1000 );

std::chrono::nanoseconds now()
{
  return
    std::chrono::duration_cast< std::chrono::nanoseconds >
    ( std::chrono::steady_clock::now().time_since_epoch() );
}

bool contains
( const boggox::dictionary& dictionary, const std::string& word )
{
  const boggox::dictionary* d( &dictionary );
  const auto end( word.end() );
  
  for ( auto it( word.begin() ); it != end; ++it )
    if ( d == nullptr )
      return false;
    else
      d = d->suffixes( *it );

  return d->terminal();
}

void bench
( const boggox::dictionary& dictionary,
  const std::vector< std::string >& words )
{
  std::chrono::nanoseconds duration( 0 );

  for ( std::size_t i( 0 ); i != g_runs; ++i )
    for ( const std::string& w : words )
      {
        const std::chrono::nanoseconds start( now() );
        contains( dictionary, w );
        duration += now() - start;
      }

  std::cout << "Boggox:\t" << duration.count() << '\n';
}

void bench_boggox( const std::vector< std::string >& words )
{
  boggox::dictionary dictionary;
  boggox::populate_dictionary( dictionary, words );

  bench( dictionary, words );
}

void bench
( const marisa::Trie& dictionary,
  const std::vector< std::string >& words )
{
  std::chrono::nanoseconds duration( 0 );

  marisa::Agent agent;
  
  for ( std::size_t i( 0 ); i != g_runs; ++i )
    for ( const std::string& w : words )
      {
        const std::chrono::nanoseconds start( now() );

        agent.set_query( w.c_str() );
        dictionary.lookup( agent );
        
        duration += now() - start;
      }

  std::cout << "Marisa:\t" << duration.count() << '\n';
}

void bench_marisa( const std::vector< std::string >& words )
{
  marisa::Keyset keys;

  for ( const std::string& w : words )
    keys.push_back( w.c_str() );
  
  marisa::Trie trie;
  trie.build( keys );

  bench( trie, words );
}

void bench
( const trie& dictionary,
  const std::vector< std::string >& words )
{
  std::chrono::nanoseconds duration( 0 );

  for ( std::size_t i( 0 ); i != g_runs; ++i )
    for ( const std::string& w : words )
      {
        const std::chrono::nanoseconds start( now() );

        find( dictionary, w );
        
        duration += now() - start;
      }

  std::cout << "Trie:\t" << duration.count() << '\n';
}

void bench_trie( const std::vector< std::string >& words )
{
  trie t;

  for ( const std::string& w : words )
    insert( t, w );
  
  bench( t, words );
}

int main( int argc, char* argv[])
{
  test_trie();
  
  if ( argc != 2 )
    {
      std::cerr << "Usage: " << argv[ 0 ] << " word_list_file\n";
      return 1;
    }
  
  std::ifstream f( argv[ 1 ] );
  std::vector< std::string > words;
  std::string s;

  while ( f >> s )
    words.push_back( s );

  bench_boggox( words );
  bench_marisa( words );
  bench_trie( words );
  
  return 0;
}
