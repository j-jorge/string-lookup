#include "trie.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <limits>
#include <unordered_map>

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

typedef std::uint32_t offset_type;
  
void flatify( std::vector< std::uint8_t >& nodes, const trie& t )
{
  const trie* current( &t );
  std::unordered_map< const trie*, std::size_t > child_index;

  std::vector< const trie* > pending( { &t } );
  
  for( std::size_t i( 0 ); i != pending.size(); ++i )
  {
    const trie* current( pending[ i ] );

    child_index[ current ] = nodes.size();

    const std::size_t child_count( current->keys.size() );
    
    nodes.push_back( child_count );

    nodes.insert( nodes.end(), current->keys.begin(), current->keys.end() );
    nodes.insert( nodes.end(), child_count * sizeof( offset_type ), 0 );

    nodes.push_back( current->terminal );

    pending.insert
      ( pending.end(), current->children.begin(), current->children.end() );
  }

  std::size_t node( 0 );
  for ( const trie* t : pending )
    {
      assert( nodes[ node ] == t->keys.size() );
      
      node += 1 + t->keys.size();

      for ( const trie* c : t->children )
        {
          const std::size_t offset( child_index[ c ] - node );
          
          if ( offset > std::numeric_limits< offset_type >::max() )
            std::cerr << "child is too far: " << offset << '\n';

          *reinterpret_cast< offset_type* >( &nodes[ node ] ) = offset;
          node += sizeof( offset_type );
        }

      assert( nodes[ node ] == t->terminal );
      
      ++node;
    }
}

bool find( const std::vector< std::uint8_t >& nodes, const std::string& word )
{
  auto node( nodes.begin() );

  for ( char c : word )
    {
      const auto begin( node + 1 );
      const auto end( begin + *node );
      const auto it( std::lower_bound( begin, end, c ) );

      if ( ( it == end ) || ( *it != c ) )
        return false;

      node = end + ( it - begin ) * sizeof( offset_type );

      node += *reinterpret_cast< const offset_type* >( &*node );
    }

  node += 1 + *node * ( 1 + sizeof( offset_type ) );
  
  return *node;
}

#define test( e ) \
  if ( !(e) )                                                           \
    std::cerr << "Test failed: " << __FILE__ << ":" << __LINE__ << "\n\t" # e \
              << "\n";

void test_simple()
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

void test_static()
{
  trie t;

  insert( t, "abc" );
  insert( t, "ab" );
  insert( t, "acd" );
  insert( t, "bad" );

  std::vector< std::uint8_t > static_trie;
  flatify( static_trie, t );
  
  test( find( static_trie, "abc" ) );
  test( find( static_trie, "ab" ) );
  test( find( static_trie, "acd" ) );
  test( find( static_trie, "bad" ) );
  test( !find( static_trie, "a" ) );
  test( !find( static_trie, "" ) );
  test( !find( static_trie, "ac" ) );
  test( !find( static_trie, "bc" ) );
  test( !find( static_trie, "ba" ) );
  test( !find( static_trie, "b" ) );
}

void test_trie()
{
  test_simple();
  test_static();
}

#undef test
