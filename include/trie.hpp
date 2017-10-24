#pragma once

#include <string>
#include <vector>

struct trie 
{
  trie() = default;
  trie( const trie& ) = delete;
  ~trie();

  trie& operator=( const trie& ) = delete;
  
  bool terminal = false;
  std::vector< char > keys;
  std::vector< trie* > children;
};

void insert( trie& t, const std::string& word );
bool find( const trie& t, const std::string& word );

void test_trie();
