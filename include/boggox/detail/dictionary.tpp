#include <cassert>

template<typename Iterator>
void boggox::dictionary::insert( Iterator first, Iterator last )
{
  if ( first == last )
    m_terminal = true;
  else
    {
      assert( 'A' <= *first );
      assert( *first <= 'Z' );
      
      if ( m_next == nullptr )
        m_next = new std::array<dictionary, 26>();

      const char key( *first - 'A' );
      ++first;
      (*m_next)[ key ].insert( first, last );
    }
}
