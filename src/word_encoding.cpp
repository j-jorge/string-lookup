#include "word_encoding.hpp"

#include <cassert>

static constexpr std::uint64_t g_letter_size_in_bits( 5 );
static constexpr std::uint64_t g_letter_mask
( ( 1 << g_letter_size_in_bits ) - 1 );
        
static std::uint64_t encode_word( const char* word )
{
    assert( word != nullptr );
    
    std::uint64_t result( 0 );
    
    for ( const char* c( word ); *c != 0; ++c )
    {
        const std::uint64_t index( *c - 'A' + 1 );
        assert( index < ( 1 << g_letter_size_in_bits ) );
        
        result = ( result << g_letter_size_in_bits ) | index;
    }

    return result;
}

std::uint64_t encode_word( const std::string& word )
{
    return encode_word( word.c_str() );
}
