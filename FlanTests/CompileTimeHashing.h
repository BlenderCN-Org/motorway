#include <Shared.h>

int CompileTimeHashing()
{
    constexpr fnChar_t* STRING_1 = (fnChar_t* const)FLAN_STRING( "The quick brown fox jumps over the lazy dog" );
    constexpr fnStringHash_t STRING_1_HASH = FLAN_STRING_HASH( STRING_1 );

    fnString_t string2 = FLAN_STRING( "The quick brown fox jumps over the lazy dog" );
    fnStringHash_t string2Hashcode = flan::core::CRC32( string2 );

    if ( string2Hashcode != STRING_1_HASH ) {
        FLAN_CERR << "Test1 failed! (hashcode mismatch)\n" << std::flush;
        FLAN_CERR << "Expected: " << STRING_1_HASH << " Got: " << string2Hashcode << std::endl;

        return 1;
    }

    constexpr fnChar_t* STRING_3 = (fnChar_t* const)FLAN_STRING( "FoOFoOFoObARbARbAR" );
    constexpr fnStringHash_t STRING_3_HASH = FLAN_STRING_HASH( STRING_3 );

    fnString_t string4 = FLAN_STRING( "FoOFoOFoObARbARbAR" );
    fnStringHash_t string4Hashcode = flan::core::CRC32( string4 );

    if ( string4Hashcode != STRING_3_HASH ) {
        FLAN_CERR << "Test2 failed! (hashcode mismatch)\n" << std::flush;
        FLAN_CERR << "Expected: " << STRING_3_HASH << " Got: " << string4Hashcode << std::endl;

        return 1;
    }

    return 0;
}
