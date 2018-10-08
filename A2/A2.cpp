// Fall 2018

#include "A2.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>
using namespace std;

#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
using namespace glm;

//----------------------------------------------------------------------------------------
// Constructor
VertexData::VertexData()
	: numVertices(0),
	  index(0)
{
	positions.resize(kMaxVertices);
	colours.resize(kMaxVertices);
}


//----------------------------------------------------------------------------------------
// Constructor
A2::A2()
	: m_currentLineColour(vec3(0.0f))
{

}

//----------------------------------------------------------------------------------------
// Destructor
A2::~A2()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A2::init()
{
	// Set the background colour.
	glClearColor(0.3, 0.5, 0.7, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao);

	enableVertexAttribIndices();

	generateVertexBuffers();

	mapVboDataToVertexAttributeLocation();


	// Initialize frames and transformation matrix

    F_w = glm::mat4(1.0f);
    F_v = glm::mat3(1.0f);
    MT = glm::mat4(1.0f);

    scale_factors[0] = 0.8f;
    scale_factors[1] = 0.8f;
    scale_factors[2] = 0.8f;
    

    glm::mat4 rM(1.0f);

	rM[1][1] = cos( glm::radians(10.0f) );
	rM[2][1] = sin( glm::radians(10.0f) );
	rM[1][2] = -sin( glm::radians(10.0f) );
	rM[2][2] = cos( glm::radians(10.0f) );

    MT = rM * MT;
    wMT = glm::mat4(1.0f);

    mode = R;
    mL = false;
    mM = false;
    mR = false;
}

//----------------------------------------------------------------------------------------
void A2::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();
}

//---------------------------------------------------------------------------------------- Fall 2018
void A2::enableVertexAttribIndices()
{
	glBindVertexArray(m_vao);

	// Enable the attribute index location for "position" when rendering.
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray(positionAttribLocation);

	// Enable the attribute index location for "colour" when rendering.
	GLint colourAttribLocation = m_shader.getAttribLocation( "colour" );
	glEnableVertexAttribArray(colourAttribLocation);

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A2::generateVertexBuffers()
{
	// Generate a vertex buffer to store line vertex positions
	{
		glGenBuffers(1, &m_vbo_positions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	// Generate a vertex buffer to store line colors
	{
		glGenBuffers(1, &m_vbo_colours);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A2::mapVboDataToVertexAttributeLocation()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao);

	// Tell GL how to map data from the vertex buffer "m_vbo_positions" into the
	// "position" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glVertexAttribPointer(positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_colours" into the
	// "colour" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
	GLint colorAttribLocation = m_shader.getAttribLocation( "colour" );
	glVertexAttribPointer(colorAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void A2::initLineData()
{
	m_vertexData.numVertices = 0;
	m_vertexData.index = 0;
}

//---------------------------------------------------------------------------------------
void A2::setLineColour (
		const glm::vec3 & colour
) {
	m_currentLineColour = colour;
}

//---------------------------------------------------------------------------------------
void A2::drawLine(
		const glm::vec2 & V0,   // Line Start (NDC coordinate)
		const glm::vec2 & V1    // Line End (NDC coordinate)
) {

	m_vertexData.positions[m_vertexData.index] = V0;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;
	m_vertexData.positions[m_vertexData.index] = V1;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;

	m_vertexData.numVertices += 2;
}

//---------------------------------------------------------------------------------------
void A2::drawOctahedron() {
/*
	// do some rotation testing
	glm::mat4 rM(1.0f);
	rM[0][0] = cos( glm::radians(1.0f) );
	rM[2][0] = sin( glm::radians(1.0f) );
    rM[0][2] = -sin( glm::radians(1.0f) );
    rM[2][2] = cos( glm::radians(1.0f) );

    MT = rM * MT;
*/
	// vertices
	glm::vec4 v[6];
	v[0] = glm::vec4( glm::vec3(  0.0f,  0.0f,  1.0f ), 1 );
	v[1] = glm::vec4( glm::vec3(  0.0f,  0.0f, -1.0f ), 1 );
	v[2] = glm::vec4( glm::vec3(  0.0f,  1.0f,  0.0f ), 1 );
	v[3] = glm::vec4( glm::vec3(  0.0f, -1.0f,  0.0f ), 1 );
	v[4] = glm::vec4( glm::vec3(  1.0f,  0.0f,  0.0f ), 1 );
	v[5] = glm::vec4( glm::vec3( -1.0f,  0.0f,  0.0f ), 1 );

	// local coordinate axis
	glm::vec4 a[3];
	a[0] = glm::vec4( glm::vec3( 0.1f, 0.0f, 0.0f ), 1 );
	a[1] = glm::vec4( glm::vec3( 0.0f, 0.1f, 0.0f ), 1 );
	a[2] = glm::vec4( glm::vec3( 0.0f, 0.0f, 0.1f ), 1 );

	// origin
	glm::vec4 o =  glm::vec4( glm::vec3( 0.0f, 0.0f, 0.0f ), 1 );
	
	// make scaling matrix
	glm::mat4 MS(1.0f);
	MS[0][0] = scale_factors[0];
    MS[1][1] = scale_factors[1];
    MS[2][2] = scale_factors[2];

	// apply transformations
	for( int i = 0; i < 6; i++ ) {
		v[i] = wMT * MT * MS * v[i];
		if ( i < 3 ) a[i] = wMT * MT * a[i];
	}
	o = wMT * MT * o;

	// vertices in 2d
	glm::vec2 v2[6];
	v2[0] = vec2( v[0].x, v[0].y );
	v2[1] = vec2( v[1].x, v[1].y );
	v2[2] = vec2( v[2].x, v[2].y );
	v2[3] = vec2( v[3].x, v[3].y );
	v2[4] = vec2( v[4].x, v[4].y );
	v2[5] = vec2( v[5].x, v[5].y );

	glm::vec2 a2[3];
	a2[0] = vec2( a[0].x, a[0].y );
	a2[1] = vec2( a[1].x, a[1].y );
	a2[2] = vec2( a[2].x, a[2].y );

	glm::vec2 o2 = vec2( o.x, o.y );

    // draw

    // draw local axis
    setLineColour( vec3(1.0f, 0.5f, 0.0f) );
    drawLine( o2, a2[0] );
    setLineColour( vec3(0.0f, 1.0f, 0.5f) );
    drawLine( o2, a2[1] );
    setLineColour( vec3(0.5f, 0.0f, 1.0f) );
    drawLine( o2, a2[2] );

    // draw shape
    setLineColour( vec3(0.0f, 0.0f, 0.0f) );
    drawLine( v2[0], v2[2] );
    drawLine( v2[0], v2[3] );
    drawLine( v2[0], v2[4] );
    drawLine( v2[0], v2[5] );
    drawLine( v2[1], v2[2] );
    drawLine( v2[1], v2[3] );
    drawLine( v2[1], v2[4] );
    drawLine( v2[1], v2[5] );
    drawLine( v2[2], v2[4] );
    drawLine( v2[2], v2[5] );
    drawLine( v2[3], v2[4] );
    drawLine( v2[3], v2[5] );

}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A2::appLogic()
{
	// Place per frame, application logic here ...

	// Call at the beginning of frame, before drawing lines:
	initLineData();

	/*
	// Draw outer square:
	setLineColour(vec3(1.0f, 0.7f, 0.8f));
	drawLine(vec2(-0.5f, -0.5f), vec2(0.5f, -0.5f));
	drawLine(vec2(0.5f, -0.5f), vec2(0.5f, 0.5f));
	drawLine(vec2(0.5f, 0.5f), vec2(-0.5f, 0.5f));
	drawLine(vec2(-0.5f, 0.5f), vec2(-0.5f, -0.5f));


	// Draw inner square:
	setLineColour(vec3(0.2f, 1.0f, 1.0f));
	drawLine(vec2(-0.25f, -0.25f), vec2(0.25f, -0.25f));
	drawLine(vec2(0.25f, -0.25f), vec2(0.25f, 0.25f));
	drawLine(vec2(0.25f, 0.25f), vec2(-0.25f, 0.25f));
	drawLine(vec2(-0.25f, 0.25f), vec2(-0.25f, -0.25f));
    */

	// Draw world axis
    glm::vec4 a[3];
    a[0] = glm::vec4( glm::vec3( 0.1f, 0.0f, 0.0f ), 1 );
    a[1] = glm::vec4( glm::vec3( 0.0f, 0.1f, 0.0f ), 1 );
    a[2] = glm::vec4( glm::vec3( 0.0f, 0.0f, 0.1f ), 1 );
    // Origin
    glm::vec4 Ow( 0.0f, 0.0f, 0.0f, 1.0f );

    // Apply transformations and change to 2d points
    glm::vec2 a2[3];
    for( int i = 0; i < 3; i++ ) {
        a[i] = wMT * a[i];
        a2[i] = vec2( a[i].x, a[i].y );
    }
    Ow = wMT * Ow;
    glm::vec2 Ow2 = glm::vec2( Ow.x, Ow.y );

    // Draw world axis
    setLineColour( vec3(1.0f, 0.0f, 0.0f) );
    drawLine( Ow2, a2[0] );
    setLineColour( vec3(0.0f, 1.0f, 0.0f) );
    drawLine( Ow2, a2[1] );
    setLineColour( vec3(0.0f, 0.0f, 1.0f) );
    drawLine( Ow2, a2[2] );


	// Draw octahedron
    drawOctahedron();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A2::guiLogic()
{
	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);


		// Add more gui elements here here ...


		// Create Button, and check if it was clicked:
		if( ImGui::Button( "[Q]uit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

        ImGui::PushID( 0 );
        if( ImGui::RadioButton( "R[o]tate View", &mode, O ) ) {
            // Select this.
        }
        if( ImGui::RadioButton( "Translat[e] View", &mode, E ) ) {
            // Select this.
        }
        if( ImGui::RadioButton( "[P]erspective", &mode, P ) ) {
            // Select this.
        }
		if( ImGui::RadioButton( "[R]otate Model", &mode, R ) ) {
			// Select this.
		}
		if( ImGui::RadioButton( "[T]ranslate Model", &mode, T ) ) {
			// Select this.
		}
		if( ImGui::RadioButton( "[S]cale Model", &mode, S ) ) {
			// Select this.
		}
		if( ImGui::RadioButton( "[V]iewport", &mode, V ) ) {
			// Select this.
		}
        ImGui::PopID();

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();
}

//----------------------------------------------------------------------------------------
void A2::uploadVertexDataToVbos() {

	//-- Copy vertex position data into VBO, m_vbo_positions:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec2) * m_vertexData.numVertices,
				m_vertexData.positions.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	//-- Copy vertex colour data into VBO, m_vbo_colours:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * m_vertexData.numVertices,
				m_vertexData.colours.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A2::draw()
{
	uploadVertexDataToVbos();

	glBindVertexArray(m_vao);

	m_shader.enable();
		glDrawArrays(GL_LINES, 0, m_vertexData.numVertices);

	m_shader.disable();

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A2::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A2::cursorEnterWindowEvent (
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
bool A2::mouseMoveEvent (
		double xPos,
		double yPos
) {
	bool eventHandled(false);
    (void) yPos;

    double dist = xPos - lastX;

	if( mode == R ) {

		// TODO: Change 768 to be relative to viewport.
		double rotation_amt = 360 * dist / 768;
		auto angle = (float) radians( rotation_amt );

		// Make transformation matrix depending on which mouse button is down.
		if( mL ) {
			glm::mat4 rML(1.0f);
			rML[1][1] = cos( angle );
			rML[2][1] = -sin( angle );
			rML[1][2] = sin( angle );
			rML[2][2] = cos( angle );
			MT = MT * rML;
		}
		if( mM ) {
			glm::mat4 rMM(1.0f);
			rMM[0][0] = cos( angle );
			rMM[0][2] = -sin( angle );
			rMM[2][0] = sin( angle );
			rMM[2][2] = cos( angle );
			MT = MT * rMM;
		}
		if( mR ) {
			glm::mat4 rMR(1.0f);
			rMR[0][0] = cos( angle );
			rMR[1][0] = -sin( angle );
			rMR[0][1] = sin( angle );
			rMR[1][1] = cos( angle );
			MT = MT * rMR;
		}
	} else if( mode == S ) {
	    // TODO: same thing
	    double scale_amt = dist / 768; 

	    if( mL && ( scale_factors[0] > 0.05f || scale_amt > 0.0f ) ) {
	        scale_factors[0] += scale_amt;
	        //if( scale_factors[0] < 0.05f ) scale_factors[0] = 0.1f;
	    }
	    if( mM && ( scale_factors[1] > 0.05f || scale_amt > 0.0f ) ) {
	        scale_factors[1] += scale_amt;
            //if( scale_factors[1] < 0.05f ) scale_factors[1] = 0.1f;
	    }
        if( mR && ( scale_factors[2] > 0.05f || scale_amt > 0.0f ) ) {
            scale_factors[2] += scale_amt;
            //if( scale_factors[2] < 0.05f ) scale_factors[2] = 0.1f;
        }
	} else if( mode == T ) {
	    float dist_move = (float) dist / 768;

	    glm::mat4 MTr(1.0f);
	    if( mL ) {
	        glm::mat4 lMTr(1.0f);
	        MTr[3][0] = dist_move;
	    }
	    if( mM ) {
            glm::mat4 mMTr(1.0f);
            MTr[3][1] = dist_move;
	    }
        if( mR ) {
            glm::mat4 rMTr(1.0f);
            MTr[3][2] = dist_move;
        }
        MT = MT * MTr;
	} else if( mode == E ) {
        float dist_move = (float) dist / 768;

        glm::mat4 wMTr(1.0f);
        if( mL ) {
            glm::mat4 lMTr(1.0f);
            wMTr[3][0] = dist_move;
        }
        if( mM ) {
            glm::mat4 mMTr(1.0f);
            wMTr[3][1] = dist_move;
        }
        if( mR ) {
            glm::mat4 rMTr(1.0f);
            wMTr[3][2] = dist_move;
        }
        wMT = wMT * wMTr;
	} else if( mode == O ) {
        double rotation_amt = 360 * dist / 768;
        auto angle = (float) radians( rotation_amt );

        // Make transformation matrix depending on which mouse button is down.
        if( mL ) {
            glm::mat4 rML(1.0f);
            rML[1][1] = cos( angle );
            rML[2][1] = -sin( angle );
            rML[1][2] = sin( angle );
            rML[2][2] = cos( angle );
            wMT = wMT * rML;
        }
        if( mM ) {
            glm::mat4 rMM(1.0f);
            rMM[0][0] = cos( angle );
            rMM[0][2] = -sin( angle );
            rMM[2][0] = sin( angle );
            rMM[2][2] = cos( angle );
            wMT = wMT * rMM;
        }
        if( mR ) {
            glm::mat4 rMR(1.0f);
            rMR[0][0] = cos( angle );
            rMR[1][0] = -sin( angle );
            rMR[0][1] = sin( angle );
            rMR[1][1] = cos( angle );
            wMT = wMT * rMR;
        }
	}


	lastX = xPos;

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A2::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		if ( actions == GLFW_PRESS ) {
			if ( button == GLFW_MOUSE_BUTTON_LEFT ) {
				mL = true;

				eventHandled = true;
			} else if( button == GLFW_MOUSE_BUTTON_MIDDLE ) {
				mM = true;

				eventHandled = true;
			} else if( button == GLFW_MOUSE_BUTTON_RIGHT ) {
				mR = true;

				eventHandled = true;
			}
		} else if ( actions == GLFW_RELEASE ) {
			if ( button == GLFW_MOUSE_BUTTON_LEFT ) {
				mL = false;

				eventHandled = true;
			} else if( button == GLFW_MOUSE_BUTTON_MIDDLE ) {
				mM = false;

				eventHandled = true;
			} else if( button == GLFW_MOUSE_BUTTON_RIGHT ) {
				mR = false;

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
bool A2::mouseScrollEvent (
		double xOffSet,
		double yOffSet
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A2::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A2::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if( action == GLFW_PRESS ) {
		if( key == GLFW_KEY_Q ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);

			eventHandled = true;
		} else if( key == GLFW_KEY_O ) {
			mode = O;

			eventHandled = true;
		} else if( key == GLFW_KEY_E ) {
			mode = E;

			eventHandled = true;
		} else if( key == GLFW_KEY_P ) {
			mode = P;

			eventHandled = true;
		} else if( key == GLFW_KEY_R ) {
			mode = R;

			eventHandled = true;
		} else if( key == GLFW_KEY_T ) {
			mode = T;

			eventHandled = true;
		} else if( key == GLFW_KEY_S ) {
			mode = S;

			eventHandled = true;
		} else if( key == GLFW_KEY_V ) {
			mode = V;

			eventHandled = true;
		}
	}

	return eventHandled;
}
