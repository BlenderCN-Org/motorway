#include <Shared.h>

#include <Io/DirectDrawSurface.h>

#include <FileSystem/VirtualFileSystem.h>
#include <FileSystem/FileSystemNative.h>

using namespace flan::core;

int DDSLoading()
{
    std::unique_ptr<FileSystemNative> nativeFS( new FileSystemNative( FLAN_STRING( "./data/" ) ) );

    VirtualFileSystem vfs;
    vfs.mount( nativeFS.get(), FLAN_STRING( "GameData" ), 0 );

    DirectDrawSurface ddsTest;

    auto ddsFile = vfs.openFile( FLAN_STRING( "GameData/Textures/default.dds" ), eFileOpenMode::FILE_OPEN_MODE_BINARY | eFileOpenMode::FILE_OPEN_MODE_READ );
    if ( ddsFile == nullptr ) {
        return 1;
    }

    flan::core::LoadDirectDrawSurface( ddsFile, ddsTest );

    FLAN_CLOG << "DDS  " << ddsTest.textureDescription.width << "x" << ddsTest.textureDescription.height << "x"
              << ddsTest.textureDescription.depth << " : " << ddsTest.textureDescription.arraySize << std::endl;
    FLAN_CLOG << "MipCount " << ddsTest.textureDescription.mipCount << " dimension " << ddsTest.textureDescription.dimension << std::endl;
    FLAN_CLOG << "Format " << ddsTest.textureDescription.format << std::endl;
    FLAN_CLOG << "isCubeMap: " << ddsTest.textureDescription.flags.isCubeMap << std::endl;

    return 0;
}
