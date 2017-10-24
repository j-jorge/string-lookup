#include "boggox/dictionary.hpp"

#include <chrono>
#include <fstream>
#include <iostream>

static constexpr std::size_t g_runs( 10000 );

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

        duration += now() - start;
      }

  std::cout << "Boggox: " << duration.count() << '\n';
}

int main( int argc, char* argv[])
{
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

  boggox::dictionary dictionary;
  boggox::populate_dictionary( dictionary, words );

  bench( dictionary, words );
  
  return 0;
}
