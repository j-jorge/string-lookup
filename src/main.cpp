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
std::size_t bench_forward
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

  return duration.count();
}

template< typename F >
std::size_t bench_reverse
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

  return duration.count();
}

struct bench_result
{
  std::string name;
  std::size_t forward;
  std::size_t reverse;
};

template< typename F >
bench_result bench
( const std::vector< std::string >& words, const std::string& tag,
  F&& f )
{
  return bench_result
    {
      tag,
      bench_forward( words, tag, f ),
      bench_reverse( words, tag, f ),
    };
}

bench_result bench_binary_search( const std::vector< std::string >& words )
{
  std::vector< std::string > sorted( words );
  std::sort( sorted.begin(), sorted.end() );

  const auto begin( sorted.begin() );
  const auto end( sorted.end() );

  return bench
    ( words, "bsearch-vector", [ & ]( const std::string& w ) -> bool
      {
        return std::binary_search( begin, end, w );
      } );
}

bench_result bench_encoded( const std::vector< std::string >& words )
{
  const std::size_t count( words.size() );
  std::vector< std::uint64_t > sorted( count );

  for ( std::size_t i( 0 ); i != count; ++i )
    sorted[ i ] = encode_word( words[ i ] );
  
  std::sort( sorted.begin(), sorted.end() );

  const auto begin( sorted.begin() );
  const auto end( sorted.end() );

  return bench
    ( words, "uint-code", [ & ]( const std::string& w ) -> bool
      {
        return std::binary_search( begin, end, encode_word( w ) );
      } );
}

bench_result bench_hash_set_code( const std::vector< std::string >& words )
{
  std::unordered_set< std::uint64_t > set;

  for ( const std::string& w : words )
    set.insert( encode_word( w ) );
  
  const auto end( set.end() );

  return bench
    ( words, "unordered_set<code>", [ & ]( const std::string& w ) -> bool
      {
        return set.find( encode_word( w ) ) != end;
      } );
}

bench_result
bench_hash_set_code_direct( const std::vector< std::string >& words )
{
  struct hash
  {
    std::size_t operator()( std::uint64_t value ) const
    {
      return value;
    }
  };
    
  std::unordered_set< std::uint64_t, hash > set;

  for ( const std::string& w : words )
    set.insert( encode_word( w ) );
  
  const auto end( set.end() );

  return bench
    ( words, "unordered_set<code, hash>", [ & ]( const std::string& w ) -> bool
      {
        return set.find( encode_word( w ) ) != end;
      } );
}

bench_result bench_hash_set( const std::vector< std::string >& words )
{
  std::unordered_set< std::string > set( words.begin(), words.end() );

  const auto end( set.end() );
  
  return bench
    ( words, "unordered_set<string>", [ & ]( const std::string& w ) -> bool
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

bench_result bench
( const boggox::dictionary& dictionary,
  const std::vector< std::string >& words )
{
  return bench
    ( words, "array-trie", [ & ]( const std::string& w ) -> bool
      {
        return contains( dictionary, w );
      } );
}

bench_result bench_boggox( const std::vector< std::string >& words )
{
  boggox::dictionary dictionary;
  boggox::populate_dictionary( dictionary, words );

  return bench( dictionary, words );
}

bench_result bench
( const marisa::Trie& dictionary,
  const std::vector< std::string >& words )
{
  marisa::Agent agent;
  
  return bench
    ( words, "marisa", [ & ]( const std::string& w ) -> bool
      {
        agent.set_query( w.c_str() );
        return dictionary.lookup( agent );
      } );
}

bench_result bench_marisa( const std::vector< std::string >& words )
{
  marisa::Keyset keys;

  for ( const std::string& w : words )
    keys.push_back( w.c_str() );
  
  marisa::Trie trie;
  trie.build( keys );

  return bench( trie, words );
}

bench_result bench
( const trie& dictionary,
  const std::vector< std::string >& words )
{
  return bench
    ( words, "dynamic-trie", [ & ]( const std::string& w ) -> bool
      {
        return find( dictionary, w );
      } );
}

bench_result bench_trie( const std::vector< std::string >& words )
{
  trie t;

  for ( const std::string& w : words )
    insert( t, w );
  
  return bench( t, words );
}

bench_result bench
( const std::vector< std::uint8_t >& dictionary,
  const std::vector< std::string >& words )
{
  return bench
    ( words, "static-trie", [ & ]( const std::string& w ) -> bool
      {
        return find( dictionary, w );
      } );
}

bench_result bench_static_trie( const std::vector< std::string >& words )
{
  trie t;

  for ( const std::string& w : words )
    insert( t, w );

  std::vector< std::uint8_t > nodes;
  flatify( nodes, t );

  return bench( nodes, words );
}

void output_result
( std::size_t size, const bench_result& base_line, const bench_result& result )
{
  std::cout << size << '\t' << (float)base_line.forward / result.forward
            <<  '\t' << (float)base_line.reverse / result.reverse
            << "\t# " << result.name << std::endl;
}

void bench_for_size
( const std::vector< std::string >& all_words, std::size_t size )
{
  std::vector< std::string > words;
  words.reserve( all_words.size() );
  
  for ( const std::string& w : all_words )
    if ( w.size() <= size )
      words.push_back( w );

  const bench_result base_line( bench_hash_set( words ) );

  output_result( size, base_line, base_line );
  output_result( size, base_line, bench_hash_set_code( words ) );
  output_result( size, base_line, bench_hash_set_code_direct( words ) );
  output_result( size, base_line, bench_encoded( words ) );
  output_result( size, base_line, bench_binary_search( words ) );
  output_result( size, base_line, bench_boggox( words ) );
  output_result( size, base_line, bench_marisa( words ) );
  output_result( size, base_line, bench_trie( words ) );
  output_result( size, base_line, bench_static_trie( words ) );
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

  for ( std::size_t i( 3 ); i <= 10; ++i )
    bench_for_size( words, i );
  
  return 0;
}
