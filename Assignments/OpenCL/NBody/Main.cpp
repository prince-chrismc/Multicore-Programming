/*
MIT License

Copyright (c) 2018 Chris McArthur, prince.chrismc(at)gmail(dot)com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#include "NBody.hpp"
#include <GL/glut.h>

NBody* nb;

void GLInit();
void idle();
void reShape( int w, int h );
void displayfunc();
void keyboardFunc(unsigned char key, int, int );

int main( int argc, char** argv )
{
   NBody clNBody;
   nb = &clNBody;

   int status = clNBody.parseCommandLine( argc, argv );
   CHECK_ERROR( status, SDK_SUCCESS, "Failed to parse CLI agrs" );

   status = clNBody.setup();
   CHECK_ERROR( status, SDK_SUCCESS, "Failed to setup NBody" );

   status = clNBody.run();
   CHECK_ERROR( status, SDK_SUCCESS, "Sample Run Program Failed" );

   clNBody.printStats();

   if( clNBody.display )
   {
       // Run in  graphical window if requested
      glutInit( &argc, argv );
      glutInitWindowPosition( 100, 10 );
      glutInitWindowSize( 1000, 800 );
      glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE );
      glutCreateWindow( "N-body simulation" );
      GLInit();
      glutDisplayFunc( displayfunc );
      glutReshapeFunc( reShape );
      glutIdleFunc( idle );
      glutKeyboardFunc( keyboardFunc );
      glutMainLoop();
   }

   status = clNBody.cleanup();
   CHECK_ERROR( status, SDK_SUCCESS, "Sample CleanUP Failed" );

   return SDK_SUCCESS;
}


/**
* @brief Initialize GL
*/
void GLInit()
{
   glClearColor( 0.0, 0.0, 0.0, 0.0 );
   glClear( GL_COLOR_BUFFER_BIT );
   glClear( GL_DEPTH_BUFFER_BIT );
   glMatrixMode( GL_PROJECTION );
   glLoadIdentity();
}

/**
* @brief Glut Idle function
*/
void idle()
{
   glutPostRedisplay();
}

/**
* @brief Glut reshape func
*
* @param w width of OpenGL window
* @param h height of OpenGL window
*/
void reShape( int w, int h )
{
   glViewport( 0, 0, w, h );

   glViewport( 0, 0, w, h );
   glMatrixMode( GL_MODELVIEW );
   glLoadIdentity();
   gluPerspective( 45.0f, w / h, 1.0f, 1000.0f );
   gluLookAt( 0.0, 0.0, -2.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0 );
}

/**
* @brief OpenGL display function
*/
void displayfunc()
{
   static int numFrames = 0;

   glClearColor( 0.0, 0.0, 0.0, 0.0 );
   glClear( GL_COLOR_BUFFER_BIT );
   glClear( GL_DEPTH_BUFFER_BIT );

   glPointSize( 1.0 );
   glBlendFunc( GL_SRC_ALPHA, GL_ONE );
   glEnable( GL_BLEND );
   glDepthMask( GL_FALSE );

   glColor3f( 1.0f, 0.5f, 0.5f );

   if( nb->isFirstLuanch )
   {
       //Calling kernel for calculatig subsequent positions
      nb->runCLKernels();
      nb->isFirstLuanch = false;
      return;
   }


   float* pos = nb->getMappedParticlePositions();
   nb->runCLKernels();
   glBegin( GL_POINTS );
   for( cl_uint i = 0; i < nb->numParticles; ++i, pos += 4 )
   {
      //divided by 300 just for scaling
      glVertex4f( *pos, *( pos + 1 ), *( pos + 2 ), 300.0f );
   }
   glEnd();
   nb->releaseMappedParticlePositions();

   //Calling kernel for calculating subsequent positions
   glFlush();
   glutSwapBuffers();

   numFrames++;
   // update window title with FPS
   if( numFrames >= 100 )
   {
      char buf[ 256 ];
      sprintf( buf, "N-body simulation - %d Particles, %.02f FPS", nb->numParticles, nb->getFPS() );
      glutSetWindowTitle( buf );
      numFrames = 0;
   }
}

/**
* @brief keyboard function
*/
void keyboardFunc( unsigned char key, int, int )
{
   switch( key )
   {
       // If the user hits escape or Q, then exit

       // ESCAPE_KEY = 27
   case 27:
   case 'q':
   case 'Q':
   {
      if( nb->cleanup() != SDK_SUCCESS )
      {
         exit( 1 );
      }
      else
      {
         exit( 0 );
      }
   }
   default:
      break;
   }
}
