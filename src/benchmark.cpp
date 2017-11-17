#include "boggox/dictionary.hpp"
#include "marisa/trie.h"
#include "trie.hpp"
#include "word_encoding.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <limits>
#include <unistd.h>
#include <unordered_set>

std::size_t g_runs( 1000 );

typedef std::array< std::uint64_t, 11 > time_per_length;

struct dictionary
{
  std::vector< std::string > words;
  std::vector< std::size_t > lengths;
};

std::chrono::nanoseconds now()
{
  return
    std::chrono::duration_cast< std::chrono::nanoseconds >
    ( std::chrono::steady_clock::now().time_since_epoch() );
}

template <class T>
void do_not_optimize_away(T&& datum)
{
  if (getpid() == 1)
    {
      const void* p = &datum;
      putchar(*static_cast<const char*>(p));
    }
}

template< typename T, typename F >
time_per_length run_benchmark
( const std::vector< T >& words, const std::vector< std::size_t >& length,
  F&& f )
{
  const std::size_t count( words.size() );
  assert( count == length.size() );
  
  std::array
    <
      std::vector< time_per_length::value_type >,
      11
    > durations;
  
  for ( std::size_t i( 0 ); i != count; ++i )
    {
      const std::chrono::nanoseconds start( now() );

      for ( std::size_t i( 0 ); i != g_runs; ++i )
        do_not_optimize_away( f( words[ i ] ) );
      
      durations[ length[ i ] ].push_back( ( now() - start ).count() );
    }

  time_per_length result;

  for ( std::size_t i( 0 ); i != durations.size(); ++i )
    if ( durations[ i ].empty() )
      result[ i ] = 1;
    else
    {
      std::sort( durations[ i ].begin(), durations[ i ].end() );
      result[ i ] = durations[ i ][ durations[ i ].size() / 2 ];
    }
  
  return result;
}

struct bench_result
{
  time_per_length forward;
  time_per_length reverse;
};

template< typename F >
bench_result bench
( const std::vector< std::string >& words,
  const std::vector< std::string >& reversed_words,
  const std::vector< std::size_t >& needle_lengths,
  F&& f )
{
  return bench_result
    {
      f( words, words, needle_lengths ),
      f( words, reversed_words, needle_lengths )
    };
}

time_per_length bench_binary_search
( const std::vector< std::string >& words,
  const std::vector< std::string >& needles,
  const std::vector< std::size_t >& needle_lengths )
{
  const auto begin( words.begin() );
  const auto end( words.end() );

  return run_benchmark
    ( needles, needle_lengths,
      [ & ]( const std::string& w ) -> bool
      {
        return std::binary_search( begin, end, w );
      } );
}

std::vector< std::uint64_t > encode_words
( const std::vector< std::string >& words )
{
  const std::size_t count( words.size() );
  std::vector< std::uint64_t > result( count );

  for ( std::size_t i( 0 ); i != count; ++i )
    result[ i ] = encode_word( words[ i ] );

  return result;
}

time_per_length bench_binary_search_code
( const std::vector< std::string >& words,
  const std::vector< std::string >& needles,
  const std::vector< std::size_t >& needle_lengths )
{
  const std::vector< std::uint64_t > coded_needles( encode_words( needles ) );

  std::vector< std::uint64_t > sorted( encode_words( words ) );
  std::sort( sorted.begin(), sorted.end() );

  const auto begin( sorted.begin() );
  const auto end( sorted.end() );

  return run_benchmark
    ( coded_needles, needle_lengths,
      [ & ]( std::uint64_t w ) -> bool
      {
        return std::binary_search( begin, end, w );
      } );
}

time_per_length bench_hash_set_code
( const std::vector< std::string >& words,
  const std::vector< std::string >& needles,
  const std::vector< std::size_t >& needle_lengths )
{
  std::unordered_set< std::uint64_t > set;

  for ( const std::string& w : words )
    set.insert( encode_word( w ) );
  
  const std::vector< std::uint64_t > coded_needles( encode_words( needles ) );
  const auto end( set.end() );

  return run_benchmark
    ( coded_needles, needle_lengths,
      [ & ]( std::uint64_t w ) -> bool
      {
        return set.find( w ) != end;
      } );
}

time_per_length bench_hash_set_code_direct
( const std::vector< std::string >& words,
  const std::vector< std::string >& needles,
  const std::vector< std::size_t >& needle_lengths )
{
  struct hash
  {
    std::size_t operator()( std::uint64_t value ) const noexcept
    {
      return value;
    }
  };
    
  std::unordered_set< std::uint64_t, hash > set;

  for ( const std::string& w : words )
    set.insert( encode_word( w ) );
  
  const std::vector< std::uint64_t > coded_needles( encode_words( needles ) );
  const auto end( set.end() );

  return run_benchmark
    ( coded_needles, needle_lengths,
      [ & ]( std::uint64_t w ) -> bool
      {
        return set.find( w ) != end;
      } );
}

time_per_length bench_hash_set
( const std::vector< std::string >& words,
  const std::vector< std::string >& needles,
  const std::vector< std::size_t >& needle_lengths )
{
  const std::unordered_set< std::string > set( words.begin(), words.end() );

  const auto end( set.end() );
  
  return run_benchmark
    ( needles, needle_lengths,
      [ & ]( const std::string& w ) -> bool
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

time_per_length bench_boggox
( const std::vector< std::string >& words,
  const std::vector< std::string >& needles,
  const std::vector< std::size_t >& needle_lengths )
{
  boggox::dictionary dictionary;
  boggox::populate_dictionary( dictionary, words );

  return run_benchmark
    ( needles, needle_lengths,
      [ & ]( const std::string& w ) -> bool
      {
        return contains( dictionary, w );
      } );
}

time_per_length bench_marisa
( const std::vector< std::string >& words,
  const std::vector< std::string >& needles,
  const std::vector< std::size_t >& needle_lengths )
{
  marisa::Keyset keys;

  for ( const std::string& w : words )
    keys.push_back( w.c_str() );
  
  marisa::Trie trie;
  trie.build( keys );

  marisa::Agent agent;
  
  return run_benchmark
    ( needles, needle_lengths,
      [ & ]( const std::string& w ) -> bool
      {
        agent.set_query( w.c_str() );
        return trie.lookup( agent );
      } );
}

time_per_length bench_dynamic_trie
( const std::vector< std::string >& words,
  const std::vector< std::string >& needles,
  const std::vector< std::size_t >& needle_lengths )
{
  trie t;

  for ( const std::string& w : words )
    insert( t, w );
  
  return run_benchmark
    ( needles, needle_lengths,
      [ & ]( const std::string& w ) -> bool
      {
        return find( t, w );
      } );
}

time_per_length bench_static_trie
( const std::vector< std::string >& words,
  const std::vector< std::string >& needles,
  const std::vector< std::size_t >& needle_lengths )
{
  trie t;

  for ( const std::string& w : words )
    insert( t, w );

  std::vector< std::uint8_t > nodes;
  flatify( nodes, t );

  return run_benchmark
    ( needles, needle_lengths,
      [ & ]( const std::string& w ) -> bool
      {
        return find( nodes, w );
      } );
}

void output_result
( std::ostream& output, const std::string& tag, const bench_result& baseline,
  const bench_result& result )
{
  for ( std::size_t length( 3 ); length <= 10; ++length )
    output << length << '\t'
           << (float)baseline.forward[ length ] / result.forward[ length ]
           << '\t'
           << (float)baseline.reverse[ length ] / result.reverse[ length ]
           << '\t'
           << "# " << tag << '\n';
}

void bench_all
( std::ostream& output, const std::vector< std::string >& words,
  const std::vector< std::string >& reversed_words,
  const std::vector< std::size_t >& lengths )
{
  const bench_result baseline
    ( bench( words, reversed_words, lengths, &bench_binary_search ) );
  
  output_result
    ( output, "bsearch-string", baseline, baseline );
  output_result
    ( output, "bsearch-code", baseline,
      bench( words, reversed_words, lengths, &bench_binary_search_code ) );
  output_result
    ( output, "hashset(code)", baseline,
      bench( words, reversed_words, lengths, &bench_hash_set_code ) );
  output_result
    ( output, "hashset(code,hash)", baseline,
      bench( words, reversed_words, lengths, &bench_hash_set_code_direct ) );
  output_result
    ( output, "hashset(string)", baseline,
      bench( words, reversed_words, lengths, &bench_hash_set ) );
  output_result
    ( output, "array-trie", baseline,
      bench( words, reversed_words, lengths, &bench_boggox ) );
  output_result
    ( output, "marisa", baseline,
      bench( words, reversed_words, lengths, &bench_marisa ) );
  output_result
    ( output, "dynamic-trie", baseline,
      bench( words, reversed_words, lengths, &bench_dynamic_trie ) );
  output_result
    ( output, "static-trie", baseline,
      bench( words, reversed_words, lengths, &bench_static_trie ) );
}

void bench_all
( std::ostream& output, const std::vector< std::string >& words )
{
  const std::size_t count( words.size() );

  std::vector< std::string > reversed_words;
  reversed_words.reserve( count );
  
  std::vector< std::size_t > lengths;
  lengths.reserve( count );

  for ( std::size_t i( 0 ); i != count; ++i )
    {
      const std::string& w( words[ i ] );
      lengths.emplace_back( w.size() );
      reversed_words.emplace_back( w.rbegin(), w.rend() );
    }

  bench_all( output, words, reversed_words, lengths );
}

void bench_all( std::ostream& output, std::istream& input )
{
  std::vector< std::string > words;
  std::string s;

  while ( input >> s )
    words.push_back( s );

  assert( std::is_sorted( words.begin(), words.end() ) );
  bench_all( output, words );
}
