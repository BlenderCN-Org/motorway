#include <Shared.h>

#include <Io/Wave.h>

#include <FileSystem/VirtualFileSystem.h>
#include <FileSystem/FileSystemNative.h>

#include <Audio/AudioDevice.h>
#include <Audio/AudioSource.h>
#include <Audio/AudioBuffer.h>

int AudioTest()
{
    std::unique_ptr<FileSystemNative> nativeFS( new FileSystemNative( FLAN_STRING( "./data/" ) ) );

    VirtualFileSystem vfs;
    vfs.mount( nativeFS.get(), FLAN_STRING( "GameData" ), 0 );

    auto waveFile = vfs.openFile( FLAN_STRING( "GameData/Sounds/test_stereo.wav" ), eFileOpenMode::FILE_OPEN_MODE_BINARY | eFileOpenMode::FILE_OPEN_MODE_READ );
    if ( waveFile == nullptr ) {
        return 1;
    }

    Wave waveContent;
    flan::core::LoadWaveFile( waveFile, waveContent );

    std::unique_ptr<AudioDevice> audioDevice( new AudioDevice() );
    audioDevice->create();
    audioDevice->setListenerPosition( glm::vec3( 0, 0, 0 ) );

    std::unique_ptr<AudioBuffer> audioBuffer( new AudioBuffer() );
    audioBuffer->create( audioDevice.get() );
    audioBuffer->update( audioDevice.get(), waveContent.data.data(), waveContent.data.size(), waveContent.audioFormat, waveContent.sampleRate );

    std::unique_ptr<AudioSource> audioSource( new AudioSource() );
    audioSource->create( audioDevice.get() );
    audioSource->setPosition( audioDevice.get(), glm::vec3( 0, 0, 0 ) );
    audioSource->setGain( audioDevice.get(), 1.0f );
    audioSource->setPitch( audioDevice.get(), 1.0f );
    audioSource->setLooping( audioDevice.get(), false );
    audioSource->bindBuffer( audioDevice.get(), audioBuffer.get() );
    audioSource->play( audioDevice.get() );

    audioSource->destroy( audioDevice.get() );
    audioBuffer->destroy( audioDevice.get() );

    return 0;
}
