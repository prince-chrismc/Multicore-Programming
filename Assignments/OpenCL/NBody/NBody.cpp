/**********************************************************************
Copyright �2015 Advanced Micro Devices, Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

�   Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
�   Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************/


#include "NBody.hpp"
#include <cmath>
#include <malloc.h>
#include <random>

NBody::NBody() : isFirstLuanch( true ), glEvent( nullptr ), display( true ), sampleArgs( true ),
initPos( nullptr ), initVel( nullptr ), vel( nullptr ), devices( nullptr ), mappedPosBuffer( nullptr ),
groupSize( GROUP_SIZE )
{
   sampleArgs.sampleVerStr = SAMPLE_VERSION;
   //numParticles = 8192;
   numParticles = 4096;

   initialize();
}

float NBody::random( float randMax, float randMin )
{
   const auto result = rand() / static_cast<float>( RAND_MAX );
   return ( ( 1.0f - result ) * randMin + result * randMax );
}

int NBody::setupNBody()
{
   numParticles = max( numParticles, static_cast<cl_uint>( groupSize ) ); // can not have fewer particles then one work group compute elements
   numParticles = static_cast<cl_uint>( ( numParticles / groupSize ) * groupSize ); // make sure numParticles is multiple of group size

   initPos = reinterpret_cast<cl_float*>( new cl_float4[ numParticles ] );
   CHECK_ALLOCATION( initPos, "Failed to allocate host memory. (initPos)" );

   static constexpr const long double PI = 3.141592653589793238462643383279502884L;

   std::random_device rd;
   std::mt19937 gen( rd() );
   const std::lognormal_distribution<double> numGenPos( 0.0, 3.8645 );

    // initialization of inputs
   for( cl_uint i = 0; i < numParticles; ++i )
   {
      const int index = 4 * i;
      const auto a = static_cast<float>( numGenPos( gen ) * 2.0L * PI );
      const auto r = static_cast<float>( sqrt( numGenPos( gen ) * 35.0 ) );

      // First 3 values are position in x,y and z direction
      //for(int j = 0; j < 3; ++j)
      if( i < numParticles / 2.75 )
      {
         initPos[ index ] = 100 + r * cos( a );
         initPos[ index + 1 ] = 40 + r * sin( a );
         initPos[ index + 2 ] = random( 109.0f, 50.0f );
      }
      else
      {
         initPos[ index ] = -25 + r * cos( a );
         initPos[ index + 1 ] = -44 + r * sin( a );
         initPos[ index + 2 ] = random( 3.0f, 100.0f );
      }

      // Mass valuee
      initPos[ index + 3 ] = random( 1.0f, 1000.0f );
   }

   return SDK_SUCCESS;
}

int NBody::setupCL()
{
   cl_int status = CL_SUCCESS;

   cl_device_type dType;

   if( sampleArgs.deviceType == "cpu" )
   {
      dType = CL_DEVICE_TYPE_CPU;
   }
   else if( sampleArgs.deviceType == "gpu" )
   {
      dType = CL_DEVICE_TYPE_GPU;
      if( !sampleArgs.isThereGPU() )
      {
         std::cout << "GPU not found. Falling back to CPU device" << std::endl;
         dType = CL_DEVICE_TYPE_CPU;
      }
   }
   else
   {
      dType = CL_DEVICE_TYPE_ALL;
   }

   /*
    * Have a look at the available platforms and pick either
    * the AMD one if available or a reasonable default.
    */
   cl_platform_id platform = nullptr;
   int retValue = getPlatform( platform, sampleArgs.platformId, sampleArgs.isPlatformEnabled() );
   CHECK_ERROR( retValue, SDK_SUCCESS, "getPlatform() failed" );

   // Display available devices.
   retValue = displayDevices( platform, dType );
   CHECK_ERROR( retValue, SDK_SUCCESS, "displayDevices() failed" );

   /*
    * If we could find our platform, use it. Otherwise use just available platform.
    */
   cl_context_properties cps[ 3 ] = { CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>( platform ), 0 };
   context = clCreateContextFromType( cps, dType, nullptr, nullptr, &status );
   CHECK_OPENCL_ERROR( status, "clCreateContextFromType failed." );

   // getting device on which to run the sample
   status = getDevices( context, &devices, sampleArgs.deviceId, sampleArgs.isDeviceIdEnabled() );
   CHECK_ERROR( status, SDK_SUCCESS, "getDevices() failed" );

   {
      // The block is to move the declaration of prop closer to its use
      const cl_command_queue_properties prop = 0;
      commandQueue = clCreateCommandQueue( context, devices[ sampleArgs.deviceId ], prop, &status );
      CHECK_OPENCL_ERROR( status, "clCreateCommandQueue failed." );
   }

   //Set device info of given cl_device_id
   retValue = deviceInfo.setDeviceInfo( devices[ sampleArgs.deviceId ] );
   CHECK_ERROR( retValue, SDK_SUCCESS, "SDKDeviceInfo::setDeviceInfo() failed" );

   /*
   * Create and initialize memory objects
   */
   const size_t bufferSize = numParticles * sizeof( cl_float4 );
   for( int i = 0; i < 2; i++ )
   {
      particlePos[ i ] = clCreateBuffer( context, CL_MEM_READ_WRITE, bufferSize, nullptr, &status );
      CHECK_OPENCL_ERROR( status, "clCreateBuffer failed. (particlePos)" );
      particleVel[ i ] = clCreateBuffer( context, CL_MEM_READ_WRITE, bufferSize, nullptr, &status );
      CHECK_OPENCL_ERROR( status, "clCreateBuffer failed. (particleVel)" );
   }

   // Initialize position buffer
   status = clEnqueueWriteBuffer( commandQueue, particlePos[ 0 ], CL_TRUE, 0, bufferSize, initPos, 0, nullptr, nullptr );
   CHECK_OPENCL_ERROR( status, "clEnqueueWriteBuffer failed. " );

   // Initialize the velocity buffer to zero
   const auto p = static_cast<float*>( clEnqueueMapBuffer( commandQueue, particleVel[ 0 ], CL_TRUE, CL_MAP_WRITE, 0, bufferSize, 0,
                                       nullptr, nullptr, &status ) );
   CHECK_OPENCL_ERROR( status, "clEnqueueMapBuffer failed. " );
   memset( p, 0, bufferSize );
   status = clEnqueueUnmapMemObject( commandQueue, particleVel[ 0 ], p, 0, nullptr, nullptr );
   CHECK_OPENCL_ERROR( status, "clEnqueueUnmapMemObject failed. " );

   status = clFlush( commandQueue );
   CHECK_OPENCL_ERROR( status, "clFlush failed. " );

   // create a CL program using the kernel source
   buildProgramData buildData;
   buildData.kernelName = "NBody_Kernels.cl";
   buildData.devices = devices;
   buildData.deviceId = sampleArgs.deviceId;
   buildData.flagsStr.clear();
   if( sampleArgs.isLoadBinaryEnabled() )
   {
      buildData.binaryName = sampleArgs.loadBinary;
   }

   if( sampleArgs.isComplierFlagsSpecified() )
   {
      buildData.flagsFileName = sampleArgs.flags;
   }

   retValue = buildOpenCLProgram( program, context, buildData );
   CHECK_ERROR( retValue, SDK_SUCCESS, "buildOpenCLProgram() failed" );

   // get a kernel object handle for a kernel with the given name
   kernel = clCreateKernel( program, "nbody_sim", &status );
   CHECK_OPENCL_ERROR( status, "clCreateKernel failed." );

   return SDK_SUCCESS;
}


// Set appropriate arguments to the kernel
int NBody::setupCLKernels() const
{
   // numParticles
   cl_int status = clSetKernelArg( kernel, 2, sizeof( cl_uint ), &numParticles );
   CHECK_OPENCL_ERROR( status, "clSetKernelArg failed. (numParticles)" );

   // time step
   status = clSetKernelArg( kernel, 3, sizeof( cl_float ), &delT );
   CHECK_OPENCL_ERROR( status, "clSetKernelArg failed. (delT)" );

   // upward Pseudoprobability
   status = clSetKernelArg( kernel, 4, sizeof( cl_float ), &espSqr );
   CHECK_OPENCL_ERROR( status, "clSetKernelArg failed. (espSqr)" );

   return SDK_SUCCESS;
}


int NBody::runCLKernels()
{
   const int currentBuffer = currentPosBufferIndex;
   const int nextBuffer = ( currentPosBufferIndex + 1 ) % 2;

   /*
   * Enqueue a kernel run call.
   */
   size_t globalThreads[] = { numParticles };
   size_t localThreads[] = { groupSize };

   // Particle positions
   cl_int status = clSetKernelArg( kernel, 0, sizeof( cl_mem ), particlePos + currentBuffer );
   CHECK_OPENCL_ERROR( status, "clSetKernelArg failed. (updatedPos)" );

   // Particle velocity
   status = clSetKernelArg( kernel, 1, sizeof( cl_mem ), particleVel + currentBuffer );
   CHECK_OPENCL_ERROR( status, "clSetKernelArg failed. (updatedVel)" );

   // Particle positions
   status = clSetKernelArg( kernel, 5, sizeof( cl_mem ), particlePos + nextBuffer );
   CHECK_OPENCL_ERROR( status, "clSetKernelArg failed. (unewPos)" );

   // Particle velocity
   status = clSetKernelArg( kernel, 6, sizeof( cl_mem ), particleVel + nextBuffer );
   CHECK_OPENCL_ERROR( status, "clSetKernelArg failed. (newVel)" );

   status = clEnqueueNDRangeKernel( commandQueue, kernel, 1, nullptr, globalThreads, localThreads, 0, nullptr, nullptr );
   CHECK_OPENCL_ERROR( status, "clEnqueueNDRangeKernel failed." );

   status = clFlush( commandQueue );
   CHECK_OPENCL_ERROR( status, "clFlush failed." );

   currentPosBufferIndex = nextBuffer;
   timerNumFrames++;

   return SDK_SUCCESS;
}

float* NBody::getMappedParticlePositions()
{
   cl_int status;
   mappedPosBufferIndex = currentPosBufferIndex;
   mappedPosBuffer = static_cast<float*>( clEnqueueMapBuffer( commandQueue, particlePos[ mappedPosBufferIndex ], CL_TRUE, CL_MAP_READ,
                                          0, numParticles * 4 * sizeof( float ), 0, nullptr, nullptr, &status ) );
   return mappedPosBuffer;
}

void NBody::releaseMappedParticlePositions()
{
   if( mappedPosBuffer )
   {
      clEnqueueUnmapMemObject( commandQueue, particlePos[ mappedPosBufferIndex ], mappedPosBuffer, 0, nullptr, nullptr );
      mappedPosBuffer = nullptr;
      clFlush( commandQueue );
   }
}

int NBody::initialize()
{
    // Call base class Initialize to get default configuration
   if( sampleArgs.initialize() != SDK_SUCCESS )
   {
      return SDK_FAILURE;
   }

   auto num_particles = Option{ "x","particles","Number of particles", "" , CA_ARG_INT , &numParticles };
   sampleArgs.AddOption( &num_particles );

   return SDK_SUCCESS;
}

int NBody::setup()
{
   fpsTimer = sampleTimer.createTimer();
   sampleTimer.resetTimer( fpsTimer );
   sampleTimer.startTimer( fpsTimer );

   CHECK_ERROR( setupNBody(), SDK_SUCCESS, "Failed to setup NBody" );
   CHECK_ERROR( setupCL(), SDK_SUCCESS, "Failed to setup NBody OpenCL" );
   CHECK_ERROR( setupCLKernels(), SDK_SUCCESS, "Failed to setup NBody OpenCl kernels" );
   return SDK_SUCCESS;
}

double NBody::getFPS()
{
   sampleTimer.stopTimer( fpsTimer );
   const double fps = timerNumFrames / sampleTimer.readTimer( fpsTimer );
   timerNumFrames = 0;
   sampleTimer.resetTimer( fpsTimer );
   sampleTimer.startTimer( fpsTimer );
   return fps;
}

int NBody::cleanup()
{
   cl_int status = clReleaseKernel( kernel );
   CHECK_OPENCL_ERROR( status, "clReleaseKernel failed.(kernel)" );

   status = clReleaseProgram( program );
   CHECK_OPENCL_ERROR( status, "clReleaseProgram failed.(program)" );

   for( int i = 0; i < 2; i++ )
   {
      status = clReleaseMemObject( particlePos[ i ] );
      CHECK_OPENCL_ERROR( status, "clReleaseMemObject failed.(particlePos)" );
      status = clReleaseMemObject( particleVel[ i ] );
      CHECK_OPENCL_ERROR( status, "clReleaseMemObject failed.(particleVel)" );
   }

   status = clReleaseCommandQueue( commandQueue );
   CHECK_OPENCL_ERROR( status, "clReleaseCommandQueue failed.(commandQueue)" );

   status = clReleaseContext( context );
   CHECK_OPENCL_ERROR( status, "clReleaseContext failed.(context)" );

   return SDK_SUCCESS;
}

NBody::~NBody()
{
   if( this->glEvent )
   {
      clReleaseEvent( this->glEvent );
   }
   // release program resources
   FREE( initPos );

   FREE( initVel );

#if defined (_WIN32)
   ALIGNED_FREE( vel );
#else
   FREE( vel );
#endif

   FREE( devices );
}

int NBody::parseCommandLine( int argc, char** argv )
{
   return sampleArgs.parseCommandLine( argc, argv );
}
