#include <Shared.h>
#include "HosekSkyRenderModule.h"

#include <Graphics/RenderPipeline.h>
#include <Graphics/ShaderCache.h>

#include <Rendering/ImageFormat.h>
#include <Rendering/RenderDevice.h>
#include <Rendering/CommandList.h>

#include <Framework/Cameras/Camera.h>

#include <glm/glm/gtc/constants.hpp>
#include <glm/glm/glm.hpp>

#include "ArHosekSkyModelData_RGB.h"

double EvaluateSpline( const double* spline, size_t stride, double value )
{
    return
        1 * std::pow( 1 - value, 5 ) *                      spline[0 * stride] +
        5 * std::pow( 1 - value, 4 ) * std::pow( value, 1 ) * spline[1 * stride] +
        10 * std::pow( 1 - value, 3 ) * std::pow( value, 2 ) * spline[2 * stride] +
        10 * std::pow( 1 - value, 2 ) * std::pow( value, 3 ) * spline[3 * stride] +
        5 * std::pow( 1 - value, 1 ) * std::pow( value, 4 ) * spline[4 * stride] +
        1 * std::pow( value, 5 ) * spline[5 * stride];
}

float Evaluate( const double* dataset, size_t stride, float turbidity, float albedo, float sunTheta )
{
    // Splines are functions of elevation^1/3
    const double elevationK = std::pow( glm::max<float>( 0.0f, 1.0f - sunTheta / ( glm::pi<float>() / 2.0f ) ), 1.0f / 3.0f );

    // Table has values for turbidity 1..10
    const int turbidity0 = glm::clamp<int>( static_cast<int>( turbidity ), 1, 10 );
    const int turbidity1 = glm::min( turbidity0 + 1, 10 );
    const float turbidityK = glm::clamp( turbidity - turbidity0, 0.0f, 1.0f );

    const double * datasetA0 = dataset;
    const double * datasetA1 = dataset + stride * 6 * 10;

    const double a0t0 = EvaluateSpline( datasetA0 + stride * 6 * ( turbidity0 - 1 ), stride, elevationK );
    const double a1t0 = EvaluateSpline( datasetA1 + stride * 6 * ( turbidity0 - 1 ), stride, elevationK );
    const double a0t1 = EvaluateSpline( datasetA0 + stride * 6 * ( turbidity1 - 1 ), stride, elevationK );
    const double a1t1 = EvaluateSpline( datasetA1 + stride * 6 * ( turbidity1 - 1 ), stride, elevationK );

    return static_cast<float>( a0t0 * ( 1 - albedo ) * ( 1 - turbidityK ) + a1t0 * albedo * ( 1 - turbidityK ) + a0t1 * ( 1 - albedo ) * turbidityK + a1t1 * albedo * turbidityK );
}

glm::vec3 HosekWilkie( float cos_theta, float gamma, float cos_gamma, const glm::vec3& A, const glm::vec3& B, const glm::vec3& C, const glm::vec3& D, const glm::vec3& E, const glm::vec3& F, const glm::vec3& G, const glm::vec3& H, const glm::vec3& I )
{
    const glm::vec3 chi = ( 1.0f + cos_gamma * cos_gamma ) / glm::pow( 1.0f + H * H - 2.0f * cos_gamma * H, glm::vec3( 1.5f ) );
    return ( 1.0f + A * glm::exp( B / ( cos_theta + 0.01f ) ) ) * ( C + D * glm::exp( E * gamma ) + F * ( cos_gamma * cos_gamma ) + G * chi + I * static_cast<float>( std::sqrt( glm::max( 0.0f, cos_theta ) ) ) );
}

glm::vec2 SquareToConcentricDiskMapping( float x, float y )
{
    float phi = 0.0f;
    float r = 0.0f;

    // -- (a,b) is now on [-1,1]ˆ2
    const float a = 2.0f * x - 1.0f;
    const float b = 2.0f * y - 1.0f;
    if ( a > -b )		// Region 1 or 2
    {
        if ( a > b )	// Region 1, also |a| > |b|
        {
            r = a;
            phi = ( glm::pi<float>() * 0.25f ) * ( b / a );
        } else		// Region 2, also |b| > |a|
        {
            r = b;
            phi = ( glm::pi<float>() * 0.25f ) * ( 2.0f - ( a / b ) );
        }
    } else			// region 3 or 4
    {
        if ( a < b )	// Region 3, also |a| >= |b|, a != 0
        {
            r = -a;
            phi = ( glm::pi<float>() * 0.25f ) * ( 4.0f + ( b / a ) );
        } else		// Region 4, |b| >= |a|, but a==0 and b==0 could occur.
        {
            r = -b;
            if ( b != 0.0f ) {
                phi = ( glm::pi<float>() * 0.25f ) * ( 6.0f - ( a / b ) );
            } else {
                phi = 0.0f;
            }
        }
    }

    // Done
    return glm::vec2( r * std::cos( phi ), r * std::sin( phi ) );
}

HosekSkyRenderModule::HosekSkyRenderModule()
    : coefficients{}
    , sunColor( 0, 0, 0 )
    , skyRenderPso( nullptr )
{

}

HosekSkyRenderModule::~HosekSkyRenderModule()
{

}

MutableResHandle_t HosekSkyRenderModule::renderSky( RenderPipeline* renderPipeline, bool renderSunDisk )
{
    struct PassData
    {
        MutableResHandle_t  output;

        ResHandle_t         coefficientsBuffer;
        ResHandle_t         cameraBuffer;
    };

    PassData& passData = renderPipeline->addRenderPass<PassData>(
        "Sky Render Pass",
        [&]( RenderPipelineBuilder& renderPipelineBuilder, PassData& passData ) {
            BufferDesc skyBufferDesc;
            skyBufferDesc.type = BufferDesc::CONSTANT_BUFFER;
            skyBufferDesc.size = sizeof( coefficients );

            passData.coefficientsBuffer = renderPipelineBuilder.allocateBuffer( skyBufferDesc, SHADER_STAGE_PIXEL );
          
            BufferDesc cameraBufferDesc;
            cameraBufferDesc.type = BufferDesc::CONSTANT_BUFFER;
            cameraBufferDesc.size = sizeof( CameraData );

            passData.cameraBuffer = renderPipelineBuilder.allocateBuffer( cameraBufferDesc, SHADER_STAGE_VERTEX );

            TextureDescription rtDesc = {};
            rtDesc.dimension = TextureDescription::DIMENSION_TEXTURE_2D;
            rtDesc.format = eImageFormat::IMAGE_FORMAT_R16G16B16A16_FLOAT;

            passData.output = renderPipelineBuilder.allocateRenderTarget( rtDesc, RenderPipelineBuilder::USE_PIPELINE_DIMENSIONS );
        },
        [=]( const PassData& passData, const RenderPipelineResources& renderPipelineResources, RenderDevice* renderDevice, CommandList* cmdList ) {
            // Resource List
            Buffer* skyBuffer = renderPipelineResources.getBuffer( passData.coefficientsBuffer );
            cmdList->updateBuffer( skyBuffer, &coefficients, sizeof( coefficients ) );
           
            const CameraData* camera = renderPipelineResources.getMainCamera();

            Buffer* cameraBuffer = renderPipelineResources.getBuffer( passData.cameraBuffer );
            cmdList->updateBuffer( cameraBuffer, camera, sizeof( CameraData ) );

            ResourceListDesc resListDesc = {};
            resListDesc.constantBuffers[0] = { 1, SHADER_STAGE_PIXEL, skyBuffer };
            resListDesc.constantBuffers[1] = { 0, SHADER_STAGE_VERTEX, cameraBuffer };

            ResourceList& resourceList = renderDevice->allocateResourceList( resListDesc );
            cmdList->bindResourceList( &resourceList );

            // Render Pass
            RenderTarget* outputTarget = renderPipelineResources.getRenderTarget( passData.output );

            RenderPassDesc passDesc = {}; 
            passDesc.attachements[0] = { outputTarget, RenderPassDesc::WRITE, RenderPassDesc::CLEAR_COLOR, { 0 } };

            RenderPass* renderPass = renderDevice->createRenderPass( passDesc );
            cmdList->useRenderPass( renderPass );

            // Pipeline State
            cmdList->bindPipelineState( skyRenderPso );

            cmdList->draw( 3 );

            renderDevice->destroyRenderPass( renderPass );
        }
    );
    
    return passData.output;
}

void HosekSkyRenderModule::loadCachedResources( RenderDevice* renderDevice, ShaderCache* shaderCache )
{
    PipelineStateDesc psoDesc = {};
    psoDesc.vertexShader = shaderCache->getOrUploadStage( "Atmosphere/HosekSky", SHADER_STAGE_VERTEX );
    psoDesc.pixelShader = shaderCache->getOrUploadStage( "Atmosphere/HosekSky", SHADER_STAGE_PIXEL );
    psoDesc.primitiveTopology = nya::rendering::ePrimitiveTopology::PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
    psoDesc.rasterizerState.cullMode = nya::rendering::eCullMode::CULL_MODE_NONE;
    psoDesc.depthStencilState.enableDepthTest = false;
    psoDesc.depthStencilState.enableDepthWrite = false;
    psoDesc.blendState.enableBlend = false;
    psoDesc.rasterizerState.useTriangleCCW = false;

    skyRenderPso = renderDevice->createPipelineState( psoDesc );
}

void HosekSkyRenderModule::destroy( RenderDevice* renderDevice )
{
    renderDevice->destroyPipelineState( skyRenderPso );
}

void HosekSkyRenderModule::recompute( const glm::vec3& worldSpaceSunDirection, float turbidity, float albedo, float normalizedSunY ) noexcept
{
    coefficients.SunDirection = worldSpaceSunDirection;

    const float sunTheta = std::acos( glm::clamp( worldSpaceSunDirection.y, 0.0f, 1.0f ) );
    glm::vec3& A = coefficients.A;
    glm::vec3& B = coefficients.B;
    glm::vec3& C = coefficients.C;
    glm::vec3& D = coefficients.D;
    glm::vec3& E = coefficients.E;
    glm::vec3& F = coefficients.F;
    glm::vec3& G = coefficients.G;
    glm::vec3& H = coefficients.H;
    glm::vec3& I = coefficients.I;
    glm::vec3& Z = coefficients.Z;

    for ( int i = 0; i < 3; ++i ) {
        A[i] = Evaluate( datasetsRGB[i] + 0, 9, turbidity, albedo, sunTheta );
        B[i] = Evaluate( datasetsRGB[i] + 1, 9, turbidity, albedo, sunTheta );
        C[i] = Evaluate( datasetsRGB[i] + 2, 9, turbidity, albedo, sunTheta );
        D[i] = Evaluate( datasetsRGB[i] + 3, 9, turbidity, albedo, sunTheta );
        E[i] = Evaluate( datasetsRGB[i] + 4, 9, turbidity, albedo, sunTheta );
        F[i] = Evaluate( datasetsRGB[i] + 5, 9, turbidity, albedo, sunTheta );
        G[i] = Evaluate( datasetsRGB[i] + 6, 9, turbidity, albedo, sunTheta );

        // Swapped in the dataset
        H[i] = Evaluate( datasetsRGB[i] + 8, 9, turbidity, albedo, sunTheta );
        I[i] = Evaluate( datasetsRGB[i] + 7, 9, turbidity, albedo, sunTheta );

        Z[i] = Evaluate( datasetsRGBRad[i], 1, turbidity, albedo, sunTheta );
    }

    if ( normalizedSunY ) {
        const glm::vec3 S = HosekWilkie( std::cos( sunTheta ), 0, 1.0f, A, B, C, D, E, F, G, H, I ) * Z;
        Z /= glm::dot( S, glm::vec3( 0.2126f, 0.7152f, 0.0722f ) );
        Z *= normalizedSunY;
    }

    sunColor = glm::vec3( 0, 0, 0 );
    if ( worldSpaceSunDirection.y > 0.0f ) {
        // https://www.gamedev.net/topic/671214-simple-solar-radiance-calculation/
        const float thetaS = std::acos( 1.0f - worldSpaceSunDirection.y );
        const float elevation = ( glm::pi<float>() * 0.5f ) - thetaS;
        const float sunSize = glm::radians( 0.27f );	// Angular radius of the sun from Earth
        static constexpr int NUMBER_OF_DISC_SAMPLES = 8;
        for ( int x = 0; x < NUMBER_OF_DISC_SAMPLES; ++x ) {
            for ( int y = 0; y < NUMBER_OF_DISC_SAMPLES; ++y ) {
                const float u = ( x + 0.5f ) / NUMBER_OF_DISC_SAMPLES;
                const float v = ( y + 0.5f ) / NUMBER_OF_DISC_SAMPLES;
                const glm::vec2 discSamplePos = SquareToConcentricDiskMapping( u, v );
                const float cos_theta = elevation + discSamplePos.y * sunSize;
                const float cos_gamma = discSamplePos.x * sunSize;
                const float gamma = acos( cos_gamma );
                sunColor +=  HosekWilkie( cos_theta, gamma, cos_gamma, coefficients.A, coefficients.B, coefficients.C, coefficients.D, coefficients.E, coefficients.F, coefficients.G, coefficients.H, coefficients.I );
            }
        }
        sunColor /= NUMBER_OF_DISC_SAMPLES * NUMBER_OF_DISC_SAMPLES;
        sunColor = glm::max( sunColor, glm::vec3( 0, 0, 0 ) );

        sunColor = glm::vec3( sunColor.z, sunColor.y, sunColor.x ) * 0.75f;
    }
}
