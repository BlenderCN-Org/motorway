#include <Shared.h>

#include <Core/Environment.h>

int Environment()
{
    fnString_t workingDirectory;

    flan::core::RetrieveWorkingDirectory( workingDirectory );

    if ( workingDirectory.empty() ) {
        return 1;
    }

    FLAN_CLOG << "WorkingDirectory = '" << workingDirectory << "'" << std::endl;

    //
    fnString_t homeDirectory;

    flan::core::RetrieveHomeDirectory( homeDirectory );

    if ( homeDirectory.empty() ) {
        return 1;
    }

    FLAN_CLOG << "HomeDirectory = '" << homeDirectory << "'" << std::endl;

    //
    fnString_t cpuName;
    flan::core::RetrieveCPUName( cpuName );
    int32_t cpuCoreCount = flan::core::GetCPUCoreCount();

    FLAN_CLOG << "CPU = '" << cpuName << "'" << " (" << cpuCoreCount << " cores)" << std::endl;

    //
    fnString_t osName;
    flan::core::RetrieveOSName( osName );
    FLAN_CLOG << "OS = '" << osName << "'" << std::endl;

    //
    FLAN_CLOG << "RAM = " << ( flan::core::GetTotalRAMSizeAsMB() ) << "MB" << std::endl;

    return 0;
}
