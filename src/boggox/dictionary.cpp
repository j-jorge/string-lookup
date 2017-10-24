#include "boggox/dictionary.hpp"

#include <algorithm>
#include <fstream>
#include <cassert>

boggox::dictionary::dictionary()
  : m_next( nullptr ),
    m_terminal( false )
{

}

boggox::dictionary::~dictionary()
{
  delete m_next;
}
    
void boggox::dictionary::clear()
{
  delete m_next;
  m_terminal = false;
}
    
bool boggox::dictionary::terminal() const
{
  return m_terminal;
}

const boggox::dictionary* boggox::dictionary::suffixes( char key ) const
{
  if ( ( key < 'A' ) || ( 'Z' < key ) || ( m_next == nullptr ) )
    return nullptr;

  const boggox::dictionary* result( &(*m_next)[ key - 'A' ] );

  if ( !result->m_terminal && ( result->m_next == nullptr ) )
    return nullptr;
  
  return result;
}      

bool boggox::load_dictionary( dictionary& d, const char* filename )
{
  std::ifstream f( filename );
  
  if ( !f )
    return false;

  std::string line;

  while ( std::getline( f, line ) )
    d.insert( line.begin(), line.end() );

  return true;
}

void boggox::populate_dictionary
( dictionary& d, const std::vector<std::string>& w )
{
  for ( const std::string& s : w )
    d.insert( s.begin(), s.end() );
}

std::ostream& operator<<( std::ostream& os, const boggox::dictionary& d )
{
  //for ( const boggox::dictionary::value_type& v : d )
  //  os << v << '\n';

  return os;
}
#if 0
std::pair
<
  boggox::dictionary::const_iterator,
  boggox::dictionary::const_iterator
>
boggox::filter_words_out
( dictionary::const_iterator first, dictionary::const_iterator last,
  std::size_t p, char c )
{
  assert( std::is_sorted( first, last ) );
  
  const dictionary::const_iterator begin
    ( std::find_if
      ( first, last,
        [ p, c ]( const std::string& v ) -> bool
        {
          return ( p < v.size() ) && ( v[ p ] == c );
        } ) );
                 
  const dictionary::const_iterator end
    ( std::find_if
      ( begin, last,
        [ p, c ]( const std::string& v ) -> bool
        {
          return ( p >= v.size() ) || ( v[ p ] != c );
        } ) );

  return std::make_pair( begin, end );
}
#endif
