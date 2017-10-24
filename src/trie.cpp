#include "trie.hpp"

#include <algorithm>
#include <iostream>

trie::~trie()
{
  for ( trie* c : children )
    delete c;
}

void insert( trie& t, const std::string& word )
{
  trie* current( &t );
  
  for ( char c : word )
    {
      const auto begin( current->keys.begin() );
      const auto end( current->keys.end() );
      const auto it( std::lower_bound( begin, end, c ) );

      if ( ( it != end ) && ( *it == c ) )
        current = *( current->children.begin() + ( it - begin ) );
      else
        {
          trie* const next( new trie() );
          const std::size_t offset( it - begin );
          
          current->keys.insert( begin + offset, c );
          current->children.insert
            ( current->children.begin() + offset, next );
          
          current = next;
        }
    }

  current->terminal = true;
}

bool find( const trie& t, const std::string& word )
{
  const trie* current( &t );
  
  for ( char c : word )
    {
      const auto begin( current->keys.begin() );
      const auto end( current->keys.end() );
      const auto it( std::find( begin, end, c ) );

      if ( it == end )
        return false;

      current = current->children[ it - begin ];
    }

  return current->terminal;
}

#define test( e ) \
  if ( !(e) )                                                           \
    std::cerr << "Test failed: " << __FILE__ << ":" << __LINE__ << "\n" # e

void test_trie()
{
  trie t;

  insert( t, "abc" );
  insert( t, "ab" );
  insert( t, "acd" );
  insert( t, "bad" );

  test( find( t, "abc" ) );
  test( find( t, "ab" ) );
  test( find( t, "acd" ) );
  test( find( t, "bad" ) );
  test( !find( t, "a" ) );
  test( !find( t, "" ) );
  test( !find( t, "ac" ) );
  test( !find( t, "bc" ) );
  test( !find( t, "ba" ) );
  test( !find( t, "b" ) );
}

#undef test
