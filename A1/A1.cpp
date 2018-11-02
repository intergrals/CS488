// Fall 2018

#define STB_IMAGE_IMPLEMENTATION

#include "A1.hpp"
#include "maze.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>

#include <sys/types.h>
#include <unistd.h>
#include "stb_image.h"

#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;
using namespace std;

static const size_t DIM = 16;

//----------------------------------------------------------------------------------------
// Constructor
A1::A1()
	: current_col( 0 ), m( Maze(DIM) )
{

	reset();
}

//----------------------------------------------------------------------------------------
// Destructor
A1::~A1()
{}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A1::init()
{
	// Initialize random number generator
	int rseed=getpid();
	srandom(rseed);
	// Print random number seed in case we want to rerun with
	// same random numbers
	cout << "Random number seed = " << rseed << endl;
	

	// DELETE FROM HERE...
	//Maze m(DIM);
	//m.digMaze();
	//m.printMaze();
	// ...TO HERE
    //m.digMaze();
    //m.printMaze();

	// Set the background colour.
	glClearColor( 0.3, 0.5, 0.7, 1.0 );

	// Build the shader
	m_shader.generateProgramObject();
	m_shader.attachVertexShader(
		getAssetFilePath( "VertexShader.vs" ).c_str() );
	m_shader.attachFragmentShader(
		getAssetFilePath( "FragmentShader.fs" ).c_str() );
	m_shader.link();

	// Set up the uniforms
	P_uni = m_shader.getUniformLocation( "P" );
	V_uni = m_shader.getUniformLocation( "V" );
	M_uni = m_shader.getUniformLocation( "M" );
	col_uni = m_shader.getUniformLocation( "colour" );

	initGrid();
	initFloor();

	cube_h = 1.0f;
	initCube( 0, 0, cube_h );

	// Set up initial view and projection matrices (need to do this here,
	// since it depends on the GLFW window being set up correctly).
	view = glm::lookAt( 
		glm::vec3( 0.0f, 2.*float(DIM)*2.0*M_SQRT1_2, float(DIM)*2.0*M_SQRT1_2 ),
		glm::vec3( 0.0f, 0.0f, 0.0f ),
		glm::vec3( 0.0f, 1.0f, 0.0f ) );

	proj = glm::perspective( 
		glm::radians( 30.0f ),
		float( m_framebufferWidth ) / float( m_framebufferHeight ),
		1.0f, 1000.0f );
}

/*
 * Reset values to default.
 */
void A1::reset() {

    // Reset colour

    // floor colour
    colour[0][0] = 0.5f;
    colour[0][1] = 0.5f;
    colour[0][2] = 0.5f;

    // wall colour
    colour[1][0] = 0;
    colour[1][1] = 1;
    colour[1][2] = 1;

    // player colour
    colour[2][0] = 1;
    colour[2][1] = 0;
    colour[2][2] = 0;

    // Reset player position
    p_loc[0] = -1;
    p_loc[1] = -1;
    chomp = false;
    mouse_held = false;

    // reset maze
    m.reset();

    // Reset view
    cube_h = 1;
    cur_rotation = 0.0f;
    scale_factor = 1.0f;
    c_loc[0] = 0.0f;
    c_loc[1] = 0.0f;
    c_follow[0] = 0.0f;
    c_follow[1] = 0.0f;
    follow = false;

    buggy = false;
}

/*
 * Creates maze and puts player at beginning (bottom) of it.
 */
void A1::dig() {
    m.digMaze();
    m.printMaze();

    // find starting position
    for( int i = 0; i < 16; i++ ) {
        if( m.getValue(15, i) == 0 ) {
            p_loc[0] = i;
            p_loc[1] = 15;
            if ( follow ) {
            	c_follow[0] = -i + 7;
            	c_follow[1] = -15 + 8;


                float angle = (float) glm::radians( 360 * cur_rotation );

                double temp[2];
                temp[0] = c_follow[0];
                temp[1] = c_follow[1];

                c_follow[0] = cos(angle) * temp[0] + sin(angle) * temp[1];
                c_follow[1] = -sin(angle) * temp[0] + cos(angle) * temp[1];

            }
            break;
        }
    }
}


void A1::initGrid()
{
	size_t sz = 3 * 2 * 2 * (DIM+3);

	float *verts = new float[ sz ];
	size_t ct = 0;
	for( int idx = 0; idx < DIM+3; ++idx ) {
		verts[ ct ] = -1;
		verts[ ct+1 ] = 0;
		verts[ ct+2 ] = idx-1;
		verts[ ct+3 ] = DIM+1;
		verts[ ct+4 ] = 0;
		verts[ ct+5 ] = idx-1;
		ct += 6;

		verts[ ct ] = idx-1;
		verts[ ct+1 ] = 0;
		verts[ ct+2 ] = -1;
		verts[ ct+3 ] = idx-1;
		verts[ ct+4 ] = 0;
		verts[ ct+5 ] = DIM+1;
		ct += 6;
	}

	// Create the vertex array to record buffer assignments.
	glGenVertexArrays( 1, &m_grid_vao );
	glBindVertexArray( m_grid_vao );

	// Create the cube vertex buffer
	glGenBuffers( 1, &m_grid_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_grid_vbo );
	glBufferData( GL_ARRAY_BUFFER, sz*sizeof(float),
		verts, GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	// OpenGL has the buffer now, there's no need for us to keep a copy.
	delete [] verts;

	CHECK_GL_ERRORS;
}


//----------------------------------------------------------------------------------------
/*
 * Draw the floor
 */
void A1::initFloor() {
	float vertices[] = {
			-1.0f, -0.01f, -1.0f, // 0: back  left
			17.0f, -0.01f, -1.0f, // 1: back  right
			-1.0f, -0.01f, 17.0f, // 2: front  left
			17.0f, -0.01f, 17.0f, // 3: front  right

	};

	GLuint indices[] = {
			1, 0, 2, // first triangle
			2, 3, 1 // second triangle
	};

	glGenVertexArrays(1, &floor_vao);
	glBindVertexArray(floor_vao);

	glGenBuffers(1, &floor_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, floor_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &floor_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floor_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), nullptr );
}




//----------------------------------------------------------------------------------------
/*
 * Initialize a single cube at a specified location.
 */
void A1::initCube( float x, float y, float h ) {

	float vertices[] = {
			// back vertices
			  x, 0.0f,   y, 0, 0, // 0: back bottom left
			x+1, 0.0f,   y, 1, 0, // 1: back bottom right
			  x,    h,   y, 0, 1, // 2: back top left
			x+1,    h,   y, 1, 1, // 3: back top right

			// front vertices
			  x, 0.0f, y+1, 0, 0, // 4: front bottom left
			x+1, 0.0f, y+1, 1, 0, // 5: front bottom right
			  x,    h, y+1, 0, 1, // 6: front top left
			x+1,    h, y+1, 1, 1 // 7: front top right

	};

    GLuint indices[] = {
            // back square
            1, 0, 2, // first triangle
            1, 2, 3, // second triangle

            // bottom square
            0, 1, 4,
            1, 5, 4,

            // front square
            4, 5, 6,
            5, 7, 6,

            // left square
			0, 4, 6,
			6, 2, 0,

            // right square
			1, 7, 5,
			7, 1, 3,

            // top square
            2, 7, 3,
            7, 2, 6
    };

    glGenVertexArrays(1, &cube_vao);
    glBindVertexArray(cube_vao);

    glGenBuffers(1, &cube_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &cube_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Specify the means of extracting the position values properly.
    GLint posAttrib = m_shader.getAttribLocation( "position" );
    glEnableVertexAttribArray( posAttrib );
    glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), nullptr );
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A1::appLogic()
{
	// Place per frame, application logic here ...

	/*
	 * do gravitational rotation:
	 *  set rotation factor to 0 if the mouse is held down. If mouse moves, the 0 gets overwritten and the maze
	 *  rotates. Otherwise if the mouse is not being held down, change the net rotation by the amount specified.
	 */
	if ( !mouse_held && change_rotation != 0 )
		cur_rotation += change_rotation;
	else
		change_rotation = 0;

	if ( buggy && follow && change_rotation != 0 ) {

		float angle = (float) glm::radians( 360 * change_rotation );

		double temp[2];
		temp[0] = c_follow[0];
		temp[1] = c_follow[1];

		c_follow[0] = cos(angle) * temp[0] + sin(angle) * temp[1];
		c_follow[1] = -sin(angle) * temp[0] + cos(angle) * temp[1];
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A1::guiLogic()
{
	// We already know there's only going to be one window, so for 
	// simplicity we'll store button states in static local variables.
	// If there was ever a possibility of having multiple instances of
	// A1 running simultaneously, this would break; you'd want to make
	// this into instance fields of A1.
	static bool showTestWindow(false);
	static bool showDebugWindow(true);

	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Debug Window", &showDebugWindow, ImVec2(100,100), opacity, windowFlags);
		if( ImGui::Button( "Quit Application [Q]" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}
		ImGui::SameLine();
        if( ImGui::Button( "Reset [R]" ) ) {
            reset();
        }

        if( ImGui::Button( "Dig Maze [ENTER]" ) ) {
            dig();
        }
        ImGui::SameLine();
        if( ImGui::Button( (follow)? "Follow [F] (ON)" : "Follow [F] (OFF)" ) ) {
            follow = !follow;

			if ( follow ) {
				c_follow[0] = -p_loc[0] + 7;
				c_follow[1] = -p_loc[1] + 8;


                float angle = (float) glm::radians( 360 * cur_rotation );

                double temp[2];
                temp[0] = c_follow[0];
                temp[1] = c_follow[1];

                c_follow[0] = cos(angle) * temp[0] + sin(angle) * temp[1];
                c_follow[1] = -sin(angle) * temp[0] + cos(angle) * temp[1];
			}
        }

		// Eventually you'll create multiple colour widgets with
		// radio buttons.  If you use PushID/PopID to give them all
		// unique IDs, then ImGui will be able to keep them separate.
		// This is unnecessary with a single colour selector and
		// radio button, but I'm leaving it in as an example.

		// Prefixing a widget name with "##" keeps it from being
		// displayed.

		ImGui::PushID( 0 );
		ImGui::ColorEdit3( "##Colour", colour[current_col] );
		//ImGui::SameLine();
		if( ImGui::RadioButton( "##fCol", &current_col, 0 ) ) {
			// Select this colour.
		}
		ImGui::SameLine();
        ImGui::Text( "Floor" );
		if( ImGui::RadioButton( "##wCol", &current_col, 1 ) ) {
			// Select this colour.
		}
		ImGui::SameLine();
        ImGui::Text( "Wall" );
		if( ImGui::RadioButton( "##pCol", &current_col, 2 ) ) {
			// Select this colour.
		}
        ImGui::SameLine();
        ImGui::Text( "Player" );
		ImGui::PopID();

/*
		// For convenience, you can uncomment this to show ImGui's massive
		// demonstration window right in your application.  Very handy for
		// browsing around to get the widget you want.  Then look in 
		// shared/imgui/imgui_demo.cpp to see how it's done.
		if( ImGui::Button( "Test Window" ) ) {
			showTestWindow = !showTestWindow;
		}
*/

		ImGui::Text( "Wall Height: %.lf", cube_h );
		ImGui::SameLine();
		if( ImGui::Button( "+ [Space]" ) ) {
			cube_h++;
			//initCube( 15, 15, cube_h );
		}
		ImGui::SameLine();
		if( ImGui::Button( "- [Backspace]" ) ) {
			if(cube_h > 0) cube_h--;
			//initCube( 15, 15, cube_h );
		}


		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();

	if( showTestWindow ) {
		ImGui::ShowTestWindow( &showTestWindow );
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame

            // front square, after guiLogic().
 */
void A1::draw()
{
    float angle = (float) glm::radians( 360 * cur_rotation );

	// Create a global transformation for the model (centre it).
	mat4 W;
	W = glm::scale( W, vec3( scale_factor, scale_factor, scale_factor ) );
    W = glm::translate( W, vec3( c_loc[0] + c_follow[0], 0, c_loc[1] + c_follow[1] ) );
    W = glm::rotate( W, angle, glm::vec3( 0.0f, 1.0f, 0.0f ) );
    W = glm::translate( W, vec3( -float(DIM)/2.0f, 0, -float(DIM)/2.0f) );

	m_shader.enable();
		glEnable( GL_DEPTH_TEST );

		glUniformMatrix4fv( P_uni, 1, GL_FALSE, value_ptr( proj ) );
		glUniformMatrix4fv( V_uni, 1, GL_FALSE, value_ptr( view ) );
		glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );

		// Draw the floor
		glBindVertexArray( floor_vao );
		glUniform3f( col_uni, colour[0][0], colour[0][1], colour[0][2] );
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floor_ebo);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// Just draw the grid for now.
		glBindVertexArray( m_grid_vao );
		glUniform3f( col_uni, 1, 1, 1 );
		glDrawArrays( GL_LINES, 0, (3+DIM)*4 );

		// Setup to draw cubes
        glBindVertexArray( cube_vao );
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_ebo);


        // set colour, coordinates, and draw player
        glUniform3f( col_uni, colour[2][0], colour[2][1], colour[2][2] );
        W = glm::translate( W, vec3( p_loc[0], 0.0f, p_loc[1] ) );
        glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        W = glm::translate( W, vec3( -p_loc[0], 0.0f, -p_loc[1] ) );
        glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );

		// Draw the cubes
		// Setup colour, shader, and height
		glUniform3f( col_uni, colour[1][0], colour[1][1], colour[1][2] );
        W = glm::scale( W, vec3( 1.0f, cube_h, 1.0f ) );
        glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );

		// loop through row
		for(int i = 0; i < 16; i++) {
		    // loop through column
            for(int j = 0; j < 16; j++) {
                if( m.getValue( i, j ) ) glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
                W = glm::translate( W, vec3( 1.0f, 0.0f, 0.0f ) );
                glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );
            }
            W = glm::translate( W, vec3( -16.0f, 0.0f, 1.0f ) );
            glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );
		}

		// Highlight the active square.
	m_shader.disable();

	// Restore defaults
	glBindVertexArray( 0 );

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A1::cleanup()
{}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A1::cursorEnterWindowEvent (
		int entered
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse cursor movement events.
 */
bool A1::mouseMoveEvent(double xPos, double yPos) 
{
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		// Put some code here to handle rotations.  Probably need to
		// check whether we're *dragging*, not just moving the mouse.
		// Probably need some instance variables to track the current
		// rotation amount, and maybe the previous X position (so 
		// that you can rotate relative to the *change* in X.

		if( mouse_held ) {
			change_rotation = ( xPos - prevX ) / 1024;
			cur_rotation += change_rotation;

			if( buggy && follow ) {
				float angle = (float) glm::radians( 360 * change_rotation );

				double temp[2];
				temp[0] = c_follow[0];
				temp[1] = c_follow[1];

				c_follow[0] = cos(angle) * temp[0] + sin(angle) * temp[1];
				c_follow[1] = -sin(angle) * temp[0] + cos(angle) * temp[1];
			}

		}

		prevX = xPos;
		(void) yPos;
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A1::mouseButtonInputEvent(int button, int actions, int mods) {
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		// The user clicked in the window.  If it's the left
		// mouse button, initiate a rotation.
		if( button == GLFW_MOUSE_BUTTON_LEFT) {
			if ( actions == GLFW_PRESS ) {
				change_rotation = 0;
				mouse_held = true;

				eventHandled = true;
			} else if ( actions == GLFW_RELEASE ) {
				mouse_held = false;

				eventHandled = true;
			}

		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A1::mouseScrollEvent(double xOffSet, double yOffSet) {
	bool eventHandled(false);

	// Zoom in or out.
	if( yOffSet < 0 && scale_factor < 0.2 ) return true;
	if( yOffSet > 0 && scale_factor > 3 ) return true;

	scale_factor += 0.1 * yOffSet;

	//cout << scale_factor << endl;

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A1::windowResizeEvent(int width, int height) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------

/*
 * Returns whether or not the given coordinates represent a valid space.
 */
bool A1::canMoveTo( int x, int y ) {
    if ( outsideGrid( x, y ) ) return false;
    if ( atBorder( x, y ) ) return true;
    return m.getValue( y, x ) == 0;
}

// Returns whether or not specified location is outside of the grid
bool A1::outsideGrid( int x, int y ) {
    return (x < -1 || y < -1 || x > 16 || y > 16);
}

// Returns whether or not specified location is on the grid border
bool A1::atBorder( int x, int y ) {
    return ( x == -1 || y == -1 || x == 16 || y == 16 );
}

void A1::move( int dx, int dy ) {
    if( canMoveTo( p_loc[0] + dx, p_loc[1] + dy ) || ( chomp && !outsideGrid( p_loc[0] + dx, p_loc[1] + dy ) ) ) {
        p_loc[0] += dx;
        p_loc[1] += dy;
        if( follow ) {
            float angle = (float) glm::radians( 360 * cur_rotation );

            c_follow[0] -= cos(angle) * dx + sin(angle) * dy;
            c_follow[1] -= -sin(angle) * dx + cos(angle) * dy;
        }
        if( chomp && !atBorder( p_loc[0], p_loc[1] ) ) {
            m.setValue( p_loc[1], p_loc[0], 0 );
            cout << "Chomp" << endl;
        }
        //cout << "p = " << p_loc[0] << ", " << p_loc[1] << endl;
    }
}

/*
 * Event handler.  Handles key input events.
 */
bool A1::keyInputEvent(int key, int action, int mods) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if( action == GLFW_PRESS ) {
		// Respond to some key events.
		if( key == GLFW_KEY_UP ) {
		    move( 0, -1 );

			eventHandled = true;
		} else if( key == GLFW_KEY_DOWN ) {
			move( 0, 1 );

			eventHandled = true;
		} else if( key == GLFW_KEY_LEFT ) {
			move( -1, 0 );

			eventHandled = true;
		} else if( key == GLFW_KEY_RIGHT ) {
			move( 1, 0 );

			eventHandled = true;
		} else if( key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT ) {
			chomp = true;
		} else if( key == GLFW_KEY_W ) {
		    if( c_loc[1] < 30 ) c_loc[1] += 0.3;

		    eventHandled = true;
        } else if( key == GLFW_KEY_A ) {
            if( c_loc[0] < 30 ) c_loc[0] += 0.3;

            eventHandled = true;
        } else if( key == GLFW_KEY_S ) {
            if( c_loc[1] > -30 ) c_loc[1] -= 0.3;

            eventHandled = true;
        } else if( key == GLFW_KEY_D ) {
            if( c_loc[0] > -30 ) c_loc[0] -= 0.3;

            eventHandled = true;
        } else if( key == GLFW_KEY_SPACE ) {
		        cube_h++;

		        //cout << "h = " << cube_h << endl;

		        eventHandled = true;
		} else if( key == GLFW_KEY_BACKSPACE ) {
			if( cube_h > 0 ) cube_h--;

			//cout << "h = " << cube_h << endl;
			eventHandled = true;
		} else if( key == GLFW_KEY_R ) {
		    reset();

		    //cout << "RESET" << endl;
		    eventHandled = true;
		} else if( key == GLFW_KEY_ENTER ) {
		    dig();

		    eventHandled = true;
		} else if( key == GLFW_KEY_Q ) {
			glfwSetWindowShouldClose( m_window, GL_TRUE );

			eventHandled = true;
		} else if( key == GLFW_KEY_F ) {
		    follow = !follow;

			if ( follow ) {
				c_follow[0] = -p_loc[0] + 7;
				c_follow[1] = -p_loc[1] + 8;


                float angle = (float) glm::radians( 360 * cur_rotation );

                double temp[2];
                temp[0] = c_follow[0];
                temp[1] = c_follow[1];

                c_follow[0] = cos(angle) * temp[0] + sin(angle) * temp[1];
                c_follow[1] = -sin(angle) * temp[0] + cos(angle) * temp[1];
			}

			eventHandled = true;
		} else if( key == GLFW_KEY_B ) {
			buggy = !buggy;
		}
	} else if ( action == GLFW_RELEASE ) {
		if ( key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT ) {
			chomp = false;

			eventHandled = true;
		}
	} else if ( action == GLFW_REPEAT ) {
        if( key == GLFW_KEY_W ) {
            if( c_loc[1] < 30 ) c_loc[1] += 0.3;

            eventHandled = true;
        } else if( key == GLFW_KEY_A ) {
            if( c_loc[0] < 30 ) c_loc[0] += 0.3;

            eventHandled = true;
        } else if( key == GLFW_KEY_S ) {
            if( c_loc[1] > -30 ) c_loc[1] -= 0.3;

            eventHandled = true;
        } else if( key == GLFW_KEY_D ) {
            if( c_loc[0] > -30 ) c_loc[0] -= 0.3;

            eventHandled = true;
        } else if( key == GLFW_KEY_UP ) {
            move( 0, -1 );

            eventHandled = true;
        } else if( key == GLFW_KEY_DOWN ) {
            move( 0, 1 );

            eventHandled = true;
        } else if( key == GLFW_KEY_LEFT ) {
            move( -1, 0 );

            eventHandled = true;
        } else if( key == GLFW_KEY_RIGHT ) {
            move( 1, 0 );

            eventHandled = true;
        } else if( key == GLFW_KEY_SPACE ) {
            cube_h++;

            //cout << "h = " << cube_h << endl;

            eventHandled = true;
        } else if( key == GLFW_KEY_BACKSPACE ) {
            if( cube_h > 0 ) cube_h--;

            //cout << "h = " << cube_h << endl;
            eventHandled = true;
        }
	}

	if ( eventHandled ) change_rotation = 0;

	return eventHandled;
}
