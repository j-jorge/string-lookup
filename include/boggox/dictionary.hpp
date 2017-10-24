#pragma once

#include <array>
#include <vector>

namespace boggox
{
  class dictionary
  {
  public:
    dictionary();
    dictionary( const dictionary& ) = delete;
    dictionary& operator=( const dictionary& ) = delete;
    ~dictionary();
      
    void clear();
    
    template<typename Iterator>
    void insert( Iterator first, Iterator last );
    
    bool terminal() const;

    const dictionary* suffixes( char key ) const;

  private:
    std::array<dictionary, 'Z' - 'A' + 1>* m_next;
    bool m_terminal;
  };
  
  bool load_dictionary( dictionary& d, const char* filename );
  void populate_dictionary( dictionary& d, const std::vector<std::string>& w );
}

#include "boggox/detail/dictionary.tpp"
