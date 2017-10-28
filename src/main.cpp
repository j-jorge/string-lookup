#include "boggox/dictionary.hpp"
#include "marisa/trie.h"
#include "trie.hpp"
#include "word_encoding.hpp"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <unordered_set>

static constexpr std::size_t g_runs( 1000 );

std::chrono::nanoseconds now()
{
  return
    std::chrono::duration_cast< std::chrono::nanoseconds >
    ( std::chrono::steady_clock::now().time_since_epoch() );
}

template< typename F >
void bench_forward
( const std::vector< std::string >& words, const std::string& tag,
  F&& f )
{
  std::chrono::nanoseconds duration( 0 );

  for ( std::size_t i( 0 ); i != g_runs; ++i )
    for ( const std::string& w : words )
      {
        const std::chrono::nanoseconds start( now() );
        f( w );
        duration += now() - start;
      }

  std::cout << tag << " (forward):\t" << duration.count() << std::endl;
}

template< typename F >
void bench_reverse
( const std::vector< std::string >& words, const std::string& tag,
  F&& f )
{
  std::chrono::nanoseconds duration( 0 );

  for ( std::size_t i( 0 ); i != g_runs; ++i )
    for ( const std::string& w : words )
      {
        const std::string needle( w.rbegin(), w.rend() );
        const std::chrono::nanoseconds start( now() );
        f( needle );
        duration += now() - start;
      }

  std::cout << tag << " (reverse):\t" << duration.count() << std::endl;
}

template< typename F >
void bench
( const std::vector< std::string >& words, const std::string& tag,
  F&& f )
{
  bench_forward( words, tag, f );
  bench_reverse( words, tag, f );
}

void bench_binary_search( const std::vector< std::string >& words )
{
  std::vector< std::string > sorted( words );
  std::sort( sorted.begin(), sorted.end() );

  const auto begin( sorted.begin() );
  const auto end( sorted.end() );

  bench
    ( words, "List", [ & ]( const std::string& w ) -> bool
      {
        return std::binary_search( begin, end, w );
      } );
}

void bench_encoded( const std::vector< std::string >& words )
{
  const std::size_t count( words.size() );
  std::vector< std::uint64_t > sorted( count );

  for ( std::size_t i( 0 ); i != count; ++i )
    sorted[ i ] = encode_word( words[ i ] );
  
  std::sort( sorted.begin(), sorted.end() );

  const auto begin( sorted.begin() );
  const auto end( sorted.end() );

  bench
    ( words, "Uint", [ & ]( const std::string& w ) -> bool
      {
        return std::binary_search( begin, end, encode_word( w ) );
      } );
}

void bench_hash_set_code( const std::vector< std::string >& words )
{
  std::unordered_set< std::uint64_t > set;

  for ( const std::string& w : words )
    set.insert( encode_word( w ) );
  
  const auto end( set.end() );

  bench
    ( words, "set<code>", [ & ]( const std::string& w ) -> bool
      {
        return set.find( encode_word( w ) ) != end;
      } );
}

void bench_hash_set( const std::vector< std::string >& words )
{
  std::unordered_set< std::string > set( words.begin(), words.end() );

  const auto end( set.end() );
  
  bench
    ( words, "set<string>", [ & ]( const std::string& w ) -> bool
      {
        return set.find( w ) != end;
      } );
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

  return ( d != nullptr ) && d->terminal();
}

void bench
( const boggox::dictionary& dictionary,
  const std::vector< std::string >& words )
{
  bench
    ( words, "Boggox", [ & ]( const std::string& w ) -> bool
      {
        return contains( dictionary, w );
      } );
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
  marisa::Agent agent;
  
  bench
    ( words, "Marisa", [ & ]( const std::string& w ) -> bool
      {
        agent.set_query( w.c_str() );
        return dictionary.lookup( agent );
      } );
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
  bench
    ( words, "Trie", [ & ]( const std::string& w ) -> bool
      {
        return find( dictionary, w );
      } );
}

void bench_trie( const std::vector< std::string >& words )
{
  trie t;

  for ( const std::string& w : words )
    insert( t, w );
  
  bench( t, words );
}

void bench
( const std::vector< std::uint8_t >& dictionary,
  const std::vector< std::string >& words )
{
  bench
    ( words, "Static", [ & ]( const std::string& w ) -> bool
      {
        return find( dictionary, w );
      } );
}

void bench_static_trie( const std::vector< std::string >& words )
{
  trie t;

  for ( const std::string& w : words )
    insert( t, w );

  std::vector< std::uint8_t > nodes;
  flatify( nodes, t );

  std::cout << "Flat trie's size: " << nodes.size() << '\n';
  
  bench( nodes, words );
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

  bench_hash_set( words );
  bench_hash_set_code( words );
  bench_encoded( words );
  bench_binary_search( words );
  bench_boggox( words );
  bench_marisa( words );
  bench_trie( words );
  bench_static_trie( words );
  
  return 0;
}
