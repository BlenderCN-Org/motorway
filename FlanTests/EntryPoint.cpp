#include <Shared.h>

#include "CompileTimeHashing.h"
#include "Environment.h"
#include "DirectDrawSurfaceLoading.h"
#include "AudioSource.h"

#define FLAN_TEST( test )\
auto test##Result = test();\
if ( test##Result != 0 ) {\
    FLAN_CLOG << #test << " FAILED" << std::endl;\
    testFailedCount++;\
} else {\
    FLAN_CLOG <<& #test << " OK" << std::endl;\
    testPassedCount++;\
}

int main( int argc, char** argv )
{
    int testFailedCount = 0, testPassedCount = 0;

    FLAN_TEST( CompileTimeHashing );
    FLAN_TEST( Environment );
    FLAN_TEST( DDSLoading );
    FLAN_TEST( AudioTest );

    FLAN_COUT << "Tests Failed: " << testFailedCount << " Passed: " << testPassedCount << std::endl;

    return 0;
}
