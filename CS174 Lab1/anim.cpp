////////////////////////////////////////////////////
// anim.cpp version 4.1
// Template code for drawing an articulated figure.
// CS 174A 
////////////////////////////////////////////////////

#ifdef _WIN32
#include <windows.h>
#include "GL/glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#ifdef _WIN32
#include "GL/freeglut.h"
#else
#include <GLUT/glut.h>
#endif

#include "Ball.h"
#include "FrameSaver.h"
#include "Timer.h"
#include "Shapes.h"
#include "tga.h"

#include "Angel/Angel.h"

#ifdef __APPLE__
#define glutInitContextVersion(a,b)
#define glutInitContextProfile(a)
#define glewExperimental int glewExperimentalAPPLE
#define glewInit()
#endif

/////////////////////////////////////////////////////
// These are global variables
//
//
// Add additional ones if you need,
// especially for animation
//////////////////////////////////////////////////////
void drawRing (mat4 model_trans, mat4 view_trans, float segmentWidth, float height, float radius, float thickness);
void renderWing(float by, float bz, mat4 model_trans, mat4 view_trans, int side, float osc);
void renderLeg(float by, float bz, mat4 model_trans, mat4 view_trans, int leg, int side, float osc);

FrameSaver FrSaver ;
Timer TM ;

BallData *Arcball = NULL ;
int Width = 800;
int Height = 800 ;
int Button = -1 ;
float Zoom = 1 ;
int PrevY = 0 ;

int Animate = 0 ;
int Recording = 0 ;

float spiderLocX = 0.0f,
spiderLocY = 0.0f;

void resetArcball() ;
void save_image();
void instructions();
void set_colour(float r, float g, float b) ;

const int STRLEN = 100;
typedef char STR[STRLEN];

#define PI 3.1415926535897
#define X 0
#define Y 1
#define Z 2

//texture
GLuint texture_cube;
GLuint texture_earth;

// Structs that hold the Vertex Array Object index and number of vertices of each shape.
ShapeData cubeData;
ShapeData sphereData;
ShapeData coneData;
ShapeData cylData;

// Matrix stack that can be used to push and pop the modelview matrix.
class MatrixStack {
    int    _index;
    int    _size;
    mat4*  _matrices;

   public:
    MatrixStack( int numMatrices = 32 ):_index(0), _size(numMatrices)
        { _matrices = new mat4[numMatrices]; }

    ~MatrixStack()
	{ delete[]_matrices; }

    void push( const mat4& m ) {
        assert( _index + 1 < _size );
        _matrices[_index++] = m;
    }

    mat4& pop( void ) {
        assert( _index - 1 >= 0 );
        _index--;
        return _matrices[_index];
    }
};

MatrixStack  mvstack;
mat4         model_view;
GLint        uModelView, uProjection, uView;
GLint        uAmbient, uDiffuse, uSpecular, uLightPos, uShininess;
GLint        uTex, uEnableTex;

// The eye point and look-at point.
// Currently unused. Use to control a camera with LookAt().
Angel::vec4 eye(0, 0.0, 50.0,1.0);
Angel::vec4 ref(0.0, 0.0, 0.0,1.0);
Angel::vec4 up(0.0,1.0,0.0,0.0);

double TIME = 0.0 ;


/////////////////////////////////////////////////////
//    PROC: drawCylinder()
//    DOES: this function 
//          render a solid cylinder  oriented along the Z axis. Both bases are of radius 1. 
//          The bases of the cylinder are placed at Z = 0, and at Z = 1.
//
//          
// Don't change.
//////////////////////////////////////////////////////
void drawCylinder(void)
{
    glUniformMatrix4fv( uModelView, 1, GL_TRUE, model_view );
    glBindVertexArray( cylData.vao );
    glDrawArrays( GL_TRIANGLES, 0, cylData.numVertices );
}


//////////////////////////////////////////////////////
//    PROC: drawCone()
//    DOES: this function 
//          render a solid cone oriented along the Z axis with base radius 1. 
//          The base of the cone is placed at Z = 0, and the top at Z = 1. 
//         
// Don't change.
//////////////////////////////////////////////////////
void drawCone(void)
{
    glUniformMatrix4fv( uModelView, 1, GL_TRUE, model_view );
    glBindVertexArray( coneData.vao );
    glDrawArrays( GL_TRIANGLES, 0, coneData.numVertices );
}


//////////////////////////////////////////////////////
//    PROC: drawCube()
//    DOES: this function draws a cube with dimensions 1,1,1
//          centered around the origin.
// 
// Don't change.
//////////////////////////////////////////////////////
void drawCube(void)
{
    glBindTexture( GL_TEXTURE_2D, texture_cube );
    glUniformMatrix4fv( uModelView, 1, GL_TRUE, model_view );
    glBindVertexArray( cubeData.vao );
    glDrawArrays( GL_TRIANGLES, 0, cubeData.numVertices );
}


//////////////////////////////////////////////////////
//    PROC: drawSphere()
//    DOES: this function draws a sphere with radius 1
//          centered around the origin.
// 
// Don't change.
//////////////////////////////////////////////////////
void drawSphere(void)
{
    glBindTexture( GL_TEXTURE_2D, texture_earth);
    glUniformMatrix4fv( uModelView, 1, GL_TRUE, model_view );
    glBindVertexArray( sphereData.vao );
    glDrawArrays( GL_TRIANGLES, 0, sphereData.numVertices );
}


void resetArcball()
{
    Ball_Init(Arcball);
    Ball_Place(Arcball,qOne,0.75);
}


/*********************************************************
 PROC: set_colour();
 DOES: sets all material properties to the given colour
 -- don't change
 **********************************************************/

void set_colour(float r, float g, float b)
{
    float ambient  = 0.2f;
    float diffuse  = 0.6f;
    float specular = 0.2f;
    glUniform4f(uAmbient,  ambient*r,  ambient*g,  ambient*b,  1.0f);
    glUniform4f(uDiffuse,  diffuse*r,  diffuse*g,  diffuse*b,  1.0f);
    glUniform4f(uSpecular, specular*r, specular*g, specular*b, 1.0f);
}


/*********************************************************
 PROC: instructions()
 DOES: display instruction in the console window.
 -- No need to change

 **********************************************************/
void instructions()
{
    printf("Press:\n");
    printf("  s to save the image\n");
    printf("  r to restore the original view.\n") ;
    printf("  0 to set it to the zero state.\n") ;
    printf("  a to toggle the animation.\n") ;
    printf("  m to toggle frame dumping.\n") ;
    printf("  q to quit.\n");
}


/*********************************************************
 PROC: myinit()
 DOES: performs most of the OpenGL intialization
 -- change these with care, if you must.
 
 **********************************************************/
void myinit(void)
{
    // Load shaders and use the resulting shader program
    GLuint program = InitShader( "vshader.glsl", "fshader.glsl" );
    glUseProgram(program);
    
    // Generate vertex arrays for geometric shapes
    generateCube(program, &cubeData);
    generateSphere(program, &sphereData);
    generateCone(program, &coneData);
    generateCylinder(program, &cylData);
    
    uModelView  = glGetUniformLocation( program, "ModelView"  );
    uProjection = glGetUniformLocation( program, "Projection" );
    uView       = glGetUniformLocation( program, "View"       );
    
    glClearColor( 0.3, 0.3, 0.8, 1.0 ); // dark blue background
    
    uAmbient   = glGetUniformLocation( program, "AmbientProduct"  );
    uDiffuse   = glGetUniformLocation( program, "DiffuseProduct"  );
    uSpecular  = glGetUniformLocation( program, "SpecularProduct" );
    uLightPos  = glGetUniformLocation( program, "LightPosition"   );
    uShininess = glGetUniformLocation( program, "Shininess"       );
    uTex       = glGetUniformLocation( program, "Tex"             );
    uEnableTex = glGetUniformLocation( program, "EnableTex"       );
    
    glUniform4f(uAmbient,    0.2f,  0.2f,  0.2f, 1.0f);
    glUniform4f(uDiffuse,    0.6f,  0.6f,  0.6f, 1.0f);
    glUniform4f(uSpecular,   0.2f,  0.2f,  0.2f, 1.0f);
    glUniform4f(uLightPos,  15.0f, 15.0f, 30.0f, 0.0f);
    glUniform1f(uShininess, 100.0f);
    
    glEnable(GL_DEPTH_TEST);
    
    Arcball = new BallData;
    Ball_Init(Arcball);
    Ball_Place(Arcball,qOne,0.75);
}


//////////////////////////////////////////////////////
//    PROC: myKey()
//    DOES: this function gets caled for any keypresses
// 
//////////////////////////////////////////////////////
void myKey(unsigned char key, int x, int y)
{
    float time ;
    switch (key) {
        case 'q':
        case 27:
            exit(0); 
        case 's':
            FrSaver.DumpPPM(Width,Height) ;
            break;
        case 'r':
            resetArcball() ;
            glutPostRedisplay() ;
            break ;
        case 'a': // togle animation
            Animate = 1 - Animate ;
            // reset the timer to point to the current time		
            time = TM.GetElapsedTime() ;
            TM.Reset() ;
            // printf("Elapsed time %f\n", time) ;
            break ;
        case '0':
            //reset your object
            break ;
        case 'm':
            if( Recording == 1 )
            {
                printf("Frame recording disabled.\n") ;
                Recording = 0 ;
            }
            else
            {
                printf("Frame recording enabled.\n") ;
                Recording = 1  ;
            }
            FrSaver.Toggle(Width);
            break ;
        case 'h':
        case '?':
            instructions();
            break;
    }
    glutPostRedisplay() ;

}


/**********************************************
 PROC: myMouseCB()
 DOES: handles the mouse button interaction
 
 -- don't change
 **********************************************************/
void myMouseCB(int button, int state, int x, int y)
{
    Button = button ;
    if( Button == GLUT_LEFT_BUTTON && state == GLUT_DOWN )
    {
        HVect arcball_coords;
        arcball_coords.x = 2.0*(float)x/(float)Width-1.0;
        arcball_coords.y = -2.0*(float)y/(float)Height+1.0;
        Ball_Mouse(Arcball, arcball_coords) ;
        Ball_Update(Arcball);
        Ball_BeginDrag(Arcball);

    }
    if( Button == GLUT_LEFT_BUTTON && state == GLUT_UP )
    {
        Ball_EndDrag(Arcball);
        Button = -1 ;
    }
    if( Button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN )
    {
        PrevY = y ;
    }


    // Tell the system to redraw the window
    glutPostRedisplay() ;
}


/**********************************************
 PROC: myMotionCB()
 DOES: handles the mouse motion interaction
 
 -- don't change
 **********************************************************/
void myMotionCB(int x, int y)
{
    if( Button == GLUT_LEFT_BUTTON )
    {
        HVect arcball_coords;
        arcball_coords.x = 2.0*(float)x/(float)Width - 1.0 ;
        arcball_coords.y = -2.0*(float)y/(float)Height + 1.0 ;
        Ball_Mouse(Arcball,arcball_coords);
        Ball_Update(Arcball);
        glutPostRedisplay() ;
    }
    else if( Button == GLUT_RIGHT_BUTTON )
    {
        if( y - PrevY > 0 )
            Zoom  = Zoom * 1.03 ;
        else 
            Zoom  = Zoom * 0.97 ;
        PrevY = y ;
        glutPostRedisplay() ;
    }
}


/**********************************************
 PROC: myReshape()
 DOES: handles the window being resized
 
 -- don't change
 **********************************************************/
void myReshape(int w, int h)
{
    Width = w;
    Height = h;
    
    glViewport(0, 0, w, h);
    
    mat4 projection = Perspective(50.0f, (float)w/(float)h, 1.0f, 1000.0f);
    glUniformMatrix4fv( uProjection, 1, GL_TRUE, projection );
}


/*********************************************************
 **********************************************************
 **********************************************************
 
 PROC: display()
 DOES: this gets called by the event handler to draw the scene
       so this is where you need to build your BEE
 
 MAKE YOUR CHANGES AND ADDITIONS HERE
 
 ** Add other procedures, such as renderLegs
 *** Use a hierarchical approach
 
 **********************************************************
 **********************************************************
 **********************************************************/
void display(void)
{
    // Clear the screen with the background colour (set in myinit)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    mat4 model_trans(1.0f);
    mat4 view_trans(1.0f);
    
    view_trans *= Translate(0.0f, 0.0f, -15.0f); //the same effect as zoom out
    
    // below deals with zoom in/out by mouse
    HMatrix r;
    Ball_Value(Arcball,r);
    mat4 mat_arcball_rot(
                         r[0][0], r[0][1], r[0][2], r[0][3],
                         r[1][0], r[1][1], r[1][2], r[1][3],
                         r[2][0], r[2][1], r[2][2], r[2][3],
                         r[3][0], r[3][1], r[3][2], r[3][3]);
    view_trans *= mat_arcball_rot;
    view_trans *= Scale(Zoom);
    
    glUniformMatrix4fv( uView, 1, GL_TRUE, model_view );
    
    mvstack.push(model_trans);//push, now identity is on the stack
    
   
/**************************************************************
   Your drawing/modeling starts here
***************************************************************/
    
	float groundx = 50.0f,
    groundy = 0.1f,
    groundz = 50.0f;
    
    float housex = 7.5f,
    housey = 5.0f,
    housez = 5.0f;
    
    float roofHeight = 3.0f,
    roofDelta = 0.05f;
    
    float doorx = housex*0.25,
    doory = housey*0.75,
    doorz = 0.01f;
    
    float pipeRadius = 0.15f,
    pipeThickness = 0.005f,
    pipeSegmentWidth = 0.005f,
    pipeSegmentHeight = 0.05f;
    
	float legsOscillate = 30*cos(5*TIME)+50; // Interval of [0,1]
    
    view_trans *= LookAt(eye, ref, up);
    
	// Model the ground
	model_trans *= Translate(0.0f, -5.0f, 0.0f);
	mvstack.push(model_trans);
	model_trans *= Scale(groundx,groundy,groundz);
	model_view = view_trans * model_trans;
    set_colour(0.0f, 0.7f, 0.0f);
	drawCube();
	model_trans = mvstack.pop();
    
    // Model the house
    model_trans *= Translate(0.0f,2.5f+groundy,0.0f);
    mvstack.push(model_trans);
    model_trans *= Scale(housex,housey,housez);
    model_view = view_trans*model_trans;
    set_colour(2.0f,2.0f,1.0f);
    drawCube();
    model_trans=mvstack.pop();
    // Roof
    mvstack.push(model_trans);
    model_trans *= Translate(0.0f,housey/2,0.0f);
    for (float i=0; i<roofHeight; i+=roofDelta) {
        mvstack.push(model_trans);
        model_trans *= Translate(0.0f,i,0.0f);
        model_trans *= Scale((1.2*housex)-(i),roofDelta,(1.2*housez)-(i));
        model_view = view_trans*model_trans;
        set_colour(1.0f,0.3f,0.2f);
        drawCube();
        model_trans = mvstack.pop();
    }
    model_trans = mvstack.pop();
    // Door
    mvstack.push(model_trans);
    model_trans *= Translate(0.0f,0.0f,housez/2);
    mvstack.push(model_trans);
    model_trans *= Translate(0.0f,(doory-housey)/2,doorz);
    model_trans *= Scale(doorx,doory,doorz);
    model_view = view_trans * model_trans;
    set_colour(2.0f,1.0f,1.0f);
    drawCube();
    model_trans = mvstack.pop();
    // Windows
    float loc = (doorx/2)+(housex-doorx)/4;
    for (int i=0;i<2;i++) {
        mvstack.push(model_trans);
        if (i%2==0) {
            // left
            model_trans *= Translate(-loc,0.0f,0.0f);
        } else {
            // right
            model_trans *= Translate(loc,0.0f,0.0f);
        }
        mvstack.push(model_trans);
        model_trans *= Scale(0.1f,1.5f,0.1f);
        set_colour(1.5f,1.5f,1.5f);
        
        mvstack.push(model_trans);
        model_trans *= Translate(-7.5f,0.0f,0.0f);
        model_view = view_trans * model_trans;
        drawCube();
        model_trans = mvstack.pop();
        model_view = view_trans * model_trans;
        drawCube();
        mvstack.push(model_trans);
        model_trans *= Translate(7.5f,0.0f,0.0f);
        model_view = view_trans * model_trans;
        drawCube();
        model_trans = mvstack.pop();
        
        model_trans = mvstack.pop();
        mvstack.push(model_trans);
        
        model_trans *= Scale(1.6f,0.1f,0.1f);

        mvstack.push(model_trans);
        model_trans *= Translate(0.0f,-7.5f,0.0f);
        model_view = view_trans * model_trans;
        drawCube();
        model_trans = mvstack.pop();
        model_view = view_trans * model_trans;
        drawCube();
        mvstack.push(model_trans);
        model_trans *= Translate(0.0f,7.5f,0.0f);
        model_view = view_trans * model_trans;
        drawCube();
        model_trans = mvstack.pop();
        
        model_trans = mvstack.pop();
        
        model_trans = mvstack.pop();
    }
    model_trans = mvstack.pop();
    // Water SPOUT
    mvstack.push(model_trans);
    model_trans *= Translate(housex/2+2*pipeRadius,-housey/2+pipeRadius,housez/4);
    model_trans *= RotateZ(90);
    mvstack.push(model_trans);
    set_colour(1.0f, 1.0f, 1.0f);
    model_trans *= Translate(-pipeRadius,-0.1f,0.0f);
    drawRing(model_trans, view_trans, pipeSegmentWidth, 0.2, pipeRadius, pipeThickness);
    model_trans = mvstack.pop();
    for (float i=0; i<90; i+=6) {
        mvstack.push(model_trans);
        model_trans *= RotateZ(-i);
        model_trans *= Translate(-pipeRadius,0.0f,0.0f);
        drawRing(model_trans, view_trans, pipeSegmentWidth, pipeSegmentHeight, pipeRadius, pipeThickness);
        model_trans = mvstack.pop();
    }
    mvstack.push(model_trans);
    model_trans *= RotateZ(-90);
    model_trans *= Translate(-pipeRadius,(housey+(0.4*roofHeight))/2,0.0f);
    drawRing(model_trans, view_trans, pipeSegmentWidth, housey+(0.4*roofHeight), pipeRadius, pipeThickness);
    model_trans = mvstack.pop();
    model_trans = mvstack.pop();
    
    // Model the spider
	float hx = 0.2f,
    hy = 0.2f,
    hz = 0.2f,
    tx = 0.8f,
    ty = 0.4f,
    tz = 0.6f,
    bx = 0.50f,
    by = 0.35f,
    bz = 0.35f;
	// Movement and starting position of spider
	model_trans = mvstack.pop();
    model_trans *= Translate(housex/2+5.0-spiderLocX,-housey+groundy+spiderLocY,housez/4);
    model_trans *= Scale(0.1f,0.1f,0.1f);
    // spider movement around tree
	//model_trans *= RotateY(-50*TIME);
    // spider movement up and down
	//model_trans *= Translate(0.0f, sway, 5.0f);
	
	// Head
	mvstack.push(model_trans);
	model_trans *= Translate(-(hx + bx/2), 0.0f, 0.0f);
	model_trans *= Scale(hx,hy,hz);
	model_view = view_trans * model_trans;
	set_colour(0.4f, 0.2f, 0.2f);
	drawSphere();
	model_trans = mvstack.pop();
    
	// Tail
	mvstack.push(model_trans);
	model_trans *= Translate(tx + bx/2, 0.0f, 0.0f);
	model_trans *= Scale(tx,ty,tz);
	model_view = view_trans * model_trans;
	drawSphere();
	model_trans = mvstack.pop();
	
	// Body
	mvstack.push(model_trans);
	model_trans *= Scale(bx,by,bz);
	model_view = view_trans * model_trans;
	drawSphere();
	model_trans = mvstack.pop();
    model_trans *= Translate(0.3f,0.0f,0.0f);
    set_colour(0.4f, 0.3f, 0.3f);
    // Legs
	for (int i = -1; i < 3; i++) {
        renderLeg(by, bz, model_trans, view_trans, i, 1, legsOscillate);  // Left
        renderLeg(by, bz, model_trans, view_trans, i, -1, legsOscillate); // Right
    }
    
/**************************************************************
     Your drawing/modeling ends here
 ***************************************************************/
    
    glutSwapBuffers();
    if(Recording == 1)
        FrSaver.DumpPPM(Width, Height);
}

void drawRing (mat4 model_trans, mat4 view_trans, float segmentWidth, float height, float radius, float thickness) {
    mvstack.push(model_trans);
    float circumference = 2*M_PI*(radius+thickness);
    float pieces = circumference/segmentWidth;
    float delta = segmentWidth*360/circumference;
    for (float i=0;i<pieces;i++) {
        model_trans *= RotateY(delta);
        mvstack.push(model_trans);
        model_trans *= Translate(radius, 0.0f, 0.0f);
        model_trans *= Scale(thickness,height,segmentWidth);
        model_view = view_trans * model_trans;
        drawCube();
        model_trans = mvstack.pop();
    }
    model_trans = mvstack.pop();
}

void renderWing(float by, float bz, mat4 model_trans, mat4 view_trans, int side, float osc) {
    // Left side = 1, Right side = -1
    float wingx = 0.5f,
    wingy = 0.1f,
    wingz = 1.0f;
    model_trans *= Translate(0.0f, by/2, side*bz/2);
    model_trans *= RotateX(side*40*osc);
    model_trans *= Translate(0.0f, wingy/2, side*wingz/2);
    model_trans *= Scale(wingx,wingy,wingz);
    model_view = view_trans * model_trans;
    drawCube();
    // No need to push and pop onto stack because model_trans passed by value
}

void renderLeg(float by, float bz, mat4 model_trans, mat4 view_trans, int leg, int side, float osc) {
    // Left side = 1, Right side = -1
    float lx = 0.1f,
    ly = 0.7f,
    lz = 0.1f;
    // Top leg segment
    model_trans *= Translate(leg*0.4f, -by/2, side*bz/2);
    model_trans *= RotateX(-(side)*0.5*osc-(side*120));
    // Push so that bottom segment will also have the additional frame of rotation
    mvstack.push(model_trans);
    model_trans *= Translate(0.0f, -ly/2, side*lz/2);
    model_trans *= Scale(lx,ly,lz);
    model_view = view_trans * model_trans;
    drawCube();
    model_trans = mvstack.pop();
    // Bottom leg segment
    model_trans *= Translate(0.0f, -ly, 0.0f);
    model_trans *= RotateX(side/3*osc+(side*120));
    model_trans *= Translate(0.0f, -ly/2, side*lz/2);
    model_trans *= Scale(lx,ly,lz);
    model_view = view_trans * model_trans;
    drawCube();
}

/*********************************************************
 **********************************************************
 **********************************************************
 
 PROC: idle()
 DOES: this gets called when nothing happens. 
       That's not true. 
       A lot of things happen can here.
       This is where you do your animation.
 
 MAKE YOUR CHANGES AND ADDITIONS HERE
 
 **********************************************************
 **********************************************************
 **********************************************************/
void idle(void)
{
    if( Animate == 1 )
    {
        // TM.Reset() ; // commenting out this will make the time run from 0
        // leaving 'Time' counts the time interval between successive calls to idleCB
        if( Recording == 0 )
            TIME = TM.GetElapsedTime() ;
        else
            TIME += 0.033 ; // save at 30 frames per second.
        
        //Your code starts here
        if (TIME<3) {
            eye = vec4(5*cos((2*PI)/3*TIME), 0.0, 5*sin((2*PI)/3*TIME),1.0);
        }
        if (TIME>=3 && TIME<6) {
            Zoom = 1.1 * 1+(TIME-3);
            eye = vec4(7.5/2+4.0-spiderLocX, -4.0, 5.0/4,1.0);
            ref = vec4(7.5/2,-5.0,5.0/4,1.0);
        }
        if (TIME > 6) {
            Zoom=1.5;
            ref = vec4(7.5/2,-1.0,5.0/4,1.0);
            eye = vec4(7.5/2+0.3,-1.0,5.0/4+0.1,1.0);
        }
        if (TIME>=3 && TIME<8) {
            spiderLocX=TIME-3;
        }
        
        if (TIME>8 && TIME<10) {
            glClearColor( 0.45, 0.45, 0.5, 1.0 );
        }
        
        
        //Your code ends here
        
        printf("TIME %f\n", TIME) ;
        glutPostRedisplay() ;
    }
}

/*********************************************************
     PROC: main()
     DOES: calls initialization, then hands over control
           to the event handler, which calls 
           display() whenever the screen needs to be redrawn
**********************************************************/

int main(int argc, char** argv) 
{
    glutInit(&argc, argv);
    // If your code fails to run, uncommenting these lines may help.
    //glutInitContextVersion(3, 2);
    //glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowPosition (0, 0);
    glutInitWindowSize(Width,Height);
    glutCreateWindow(argv[0]);
    printf("GL version %s\n", glGetString(GL_VERSION));
    glewExperimental = GL_TRUE;
    glewInit();
    
    instructions();
    myinit(); //performs most of the OpenGL intialization
    
    
    glutKeyboardFunc( myKey );   //keyboard event handler
    glutMouseFunc(myMouseCB) ;   //mouse button event handler
    glutMotionFunc(myMotionCB) ; //mouse motion event handler
    
    glutReshapeFunc (myReshape); //reshape event handler
    glutDisplayFunc(display);    //draw a new scene
    glutIdleFunc(idle) ;         //when nothing happens, do animaiton

    
    glutMainLoop();

    TM.Reset() ;
    return 0;         // never reached
}




