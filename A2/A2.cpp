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

#define PI 3.141592653589

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

	reset();
}

void A2::reset() {
    // Initialize frames and transformation matrix

    F_w = glm::mat4(1.0f);
    F_v = glm::mat3(1.0f);
    MT = glm::mat4(1.0f);

    scale_factors[0] = 0.8f;
    scale_factors[1] = 0.8f;
    scale_factors[2] = 0.8f;


    glm::mat4 rM(1.0f);

    MT = rM * MT;
    wMT = glm::mat4(1.0f);
    vMT = glm::mat4(1.0f);

    mode = R;
    mL = false;
    mM = false;
    mR = false;
    firstClick = false;

    // default window size
    startX = -0.95;
    startY = -0.95;
    endX = 0.95;
    endY = 0.95;

    // set perspective
    n = 1.0f;
    f = 10.0f;
    FoV = 50.0f;
    pMT = setupPMat();

    // set display variables
    viewRot[0] = 0.0f;
	viewRot[1] = 0.0f;
	viewRot[2] = 0.0f;
	viewTrans[0] = 0.0f;
	viewTrans[1] = 0.0f;
	viewTrans[2] = 0.0f;
	modRot[0] = 0.0f;
	modRot[1] = 0.0f;
	modRot[2] = 0.0f;
	modTrans[0] = 0.25f;
	modTrans[1] = -0.25f;
	modTrans[2] = 0.0f;

	glm::mat4 temp(1.0f);
	temp[3][2] = -3;
	wMT = temp * wMT;

	glm::mat4 temp2(1.0f);
	temp2[3][0] = 0.25f;
	temp2[3][1] = -0.25f;
	MT = temp2 * MT;

	normals[0] = glm::vec4( glm::vec3(  0,  1, 0 ), 1 );
	normals[1] = glm::vec4( glm::vec3( -1,  0, 0 ), 1 );
	normals[2] = glm::vec4( glm::vec3(  0, -1, 0 ), 1 );
	normals[3] = glm::vec4( glm::vec3(  1,  0, 0 ), 1 );
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
void A2::clipTo( glm::vec4 &A, glm::vec4 &B, glm::vec4 P, glm::vec4 n ) {

	float wecA = glm::dot( ( A - P ), n );
	float wecB = glm::dot( ( B - P ), n );
	if( wecA < 0 && wecB < 0 ) A = B;
	if( wecA >= 0 && wecB >= 0 ) return;
	float t = wecA / ( wecA - wecB );
	if( wecA < 0 ) A += t * ( B - A );
	else B = A + t * ( B - A );
}

//---------------------------------------------------------------------------------------
void A2::clipping( glm::vec4 &A, glm::vec4 &B ) {
	clipTo( A, B, glm::vec4(      0, startY, 0, 1 ), normals[0] );
	clipTo( A, B, glm::vec4(   endX,      0, 0, 1 ), normals[1] );
	clipTo( A, B, glm::vec4(      0,   endY, 0, 1 ), normals[2] );
	clipTo( A, B, glm::vec4( startX,      0, 0, 1 ), normals[3] );
}

//---------------------------------------------------------------------------------------
void A2::drawWorldLine( glm::vec4 A, glm::vec4 B, glm::vec3 col ) {

	A = vMT * pMT * wMT * A;
	B = vMT * pMT * wMT * B;

    A = A / A.w;
	B = B / B.w;

	clipping( A, B );

	glm::vec2 A2 = vec2( A.x, A.y );
	glm::vec2 B2 = vec2( B.x, B.y );

	// draw line
	setLineColour( col );
	drawLine( A2, B2 );

}

//---------------------------------------------------------------------------------------
void A2::drawViewLine( glm::vec4 A, glm::vec4 B, glm::vec3 col ) {
	// make scaling matrix
	glm::mat4 MS(1.0f);
	MS[0][0] = scale_factors[0];
	MS[1][1] = scale_factors[1];
	MS[2][2] = scale_factors[2];

	A = vMT * pMT * wMT * MT * MS * A;
	B = vMT * pMT * wMT * MT * MS * B;

    A = A / A.w;
    B = B / B.w;

    clipping( A, B );

	glm::vec2 A2 = vec2( A.x, A.y );
	glm::vec2 B2 = vec2( B.x, B.y );

	// draw line
	setLineColour( col );
	drawLine( A2, B2 );

}

//---------------------------------------------------------------------------------------
void A2::drawOctahedron() {
	// vertices
	glm::vec4 v[6];
	v[0] = glm::vec4( glm::vec3(  0.0f,  0.0f,  1.0f ), 1 );
	v[1] = glm::vec4( glm::vec3(  0.0f,  0.0f, -1.0f ), 1 );
	v[2] = glm::vec4( glm::vec3(  0.0f,  1.0f,  0.0f ), 1 );
	v[3] = glm::vec4( glm::vec3(  0.0f, -1.0f,  0.0f ), 1 );
	v[4] = glm::vec4( glm::vec3(  1.0f,  0.0f,  0.0f ), 1 );
	v[5] = glm::vec4( glm::vec3( -1.0f,  0.0f,  0.0f ), 1 );


	//drawEdge( glm::vec4( glm::vec3( 0.0f, 0.0f, 1.0f ), 1 ), glm::vec4( glm::vec3(  0.0f, -1.0f,  0.0f ), 1 ) );

	// local coordinate axis
	glm::vec4 a[3];
	a[0] = glm::vec4( glm::vec3( 0.1f, 0.0f, 0.0f ), 1 );
	a[1] = glm::vec4( glm::vec3( 0.0f, 0.1f, 0.0f ), 1 );
	a[2] = glm::vec4( glm::vec3( 0.0f, 0.0f, 0.1f ), 1 );

	// origin
	glm::vec4 o =  glm::vec4( glm::vec3( 0.0f, 0.0f, 0.0f ), 1 );

	// draw origin
	drawViewLine( o, a[0], glm::vec3(1.0f, 0.5f, 0.0f) );
	drawViewLine( o, a[1], glm::vec3(0.0f, 1.0f, 0.5f) );
	drawViewLine( o, a[2], glm::vec3(0.5f, 0.0f, 1.0f) );


	// draw shape
	drawViewLine( v[0], v[2], glm::vec3(0.0f, 0.0f, 0.0f) );
	drawViewLine( v[0], v[3], glm::vec3(0.0f, 0.0f, 0.0f) );
	drawViewLine( v[0], v[4], glm::vec3(0.0f, 0.0f, 0.0f) );
	drawViewLine( v[0], v[5], glm::vec3(0.0f, 0.0f, 0.0f) );
	drawViewLine( v[1], v[2], glm::vec3(0.0f, 0.0f, 0.0f) );
	drawViewLine( v[1], v[3], glm::vec3(0.0f, 0.0f, 0.0f) );
	drawViewLine( v[1], v[4], glm::vec3(0.0f, 0.0f, 0.0f) );
	drawViewLine( v[1], v[5], glm::vec3(0.0f, 0.0f, 0.0f) );
	drawViewLine( v[2], v[4], glm::vec3(0.0f, 0.0f, 0.0f) );
	drawViewLine( v[2], v[5], glm::vec3(0.0f, 0.0f, 0.0f) );
	drawViewLine( v[3], v[4], glm::vec3(0.0f, 0.0f, 0.0f) );
	drawViewLine( v[3], v[5], glm::vec3(0.0f, 0.0f, 0.0f) );

}

glm::mat4 A2::setupPMat() {
    double aspect = 1;
    glm::mat4 retPMat(0.0f);
    retPMat[0][0] = (float) ( ( 1 / glm::tan( glm::radians( FoV ) / 2.0f ) ) * aspect );
    retPMat[1][1] = (float) ( 1 / tan( glm::radians( FoV ) / 2.0f ) );
    retPMat[2][2] = (float) -( ( f + n ) / ( f - n ) );
    retPMat[2][3] = -1.0f;
    retPMat[3][2] = (float) ( ( -2 * f * n ) / ( f - n ) );

    return retPMat;
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

	// Draw world axis
    glm::vec4 a[3];
    a[0] = glm::vec4( glm::vec3( 0.1f, 0.0f, 0.0f ), 1 );
    a[1] = glm::vec4( glm::vec3( 0.0f, 0.1f, 0.0f ), 1 );
    a[2] = glm::vec4( glm::vec3( 0.0f, 0.0f, 0.1f ), 1 );
    // Origin
    glm::vec4 Ow( 0.0f, 0.0f, 0.0f, 1.0f );

    // Draw world axis
    drawWorldLine( Ow, a[0], glm::vec3(1.0f, 0.0f, 0.0f) );
    drawWorldLine( Ow, a[1], glm::vec3(0.0f, 1.0f, 0.0f) );
    drawWorldLine( Ow, a[2], glm::vec3(0.0f, 0.0f, 1.0f) );


	// Draw octahedron
    drawOctahedron();

    // Draw window
    setLineColour( vec3( 0.0f, 0.0f, 0.0f ) );
    drawLine( glm::vec2( startX, startY ), glm::vec2( startX, endY ) );
    drawLine( glm::vec2( startX, startY ), glm::vec2( endX, startY ) );
    drawLine( glm::vec2( startX, endY ), glm::vec2( endX, endY ) );
    drawLine( glm::vec2( endX, startY ), glm::vec2( endX, endY ) );
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

		ImGui::Text( "View Rotation: x:%.2f | y:%.2f | z:%.2f", viewRot[0], viewRot[1], viewRot[2] );
		ImGui::Text( "View Translation: x:%.2f | y:%.2f | z:%.2f", viewTrans[0], viewTrans[1], viewTrans[2] );
		ImGui::Text( "FoV:%.2f | Near:%.2f | Far:%.2f", FoV, n, f );
		ImGui::Text( "Model Rotation: x:%.2f | y:%.2f | z:%.2f", modRot[0], modRot[1], modRot[2] );
		ImGui::Text( "Model Translation: x:%.2f | y:%.2f | z:%.2f", modTrans[0], modTrans[1], modTrans[2] );
		ImGui::Text( "Model Scale: x:%.2f | y:%.2f | z:%.2f", scale_factors[0], scale_factors[1], scale_factors[2] );


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
		double rotation_amt = 360 * dist / CS488Window::m_windowWidth;
		auto angle = (float) radians( rotation_amt );

		// Make transformation matrix depending on which mouse button is down.
		if( mL ) {
			modRot[0] += angle;
			if ( modRot[0] > PI ) modRot[0] -= PI;
			if ( modRot[0] < -PI ) modRot[0] += PI;
			glm::mat4 rML(1.0f);
			rML[1][1] = cos( angle );
			rML[2][1] = -sin( angle );
			rML[1][2] = sin( angle );
			rML[2][2] = cos( angle );
			MT = MT * rML;
		}
		if( mM ) {
			modRot[1] += angle;
			if ( modRot[1] > PI ) modRot[1] -= PI;
			if ( modRot[1] < -PI ) modRot[1] += PI;
			glm::mat4 rMM(1.0f);
			rMM[0][0] = cos( angle );
			rMM[0][2] = -sin( angle );
			rMM[2][0] = sin( angle );
			rMM[2][2] = cos( angle );
			MT = MT * rMM;
		}
		if( mR ) {
			modRot[2] += angle;
			if ( modRot[2] > PI ) modRot[2] -= PI;
			if ( modRot[2] < -PI ) modRot[2] += PI;
			glm::mat4 rMR(1.0f);
			rMR[0][0] = cos( angle );
			rMR[1][0] = -sin( angle );
			rMR[0][1] = sin( angle );
			rMR[1][1] = cos( angle );
			MT = MT * rMR;
		}
	} else if( mode == S ) {
	    double scale_amt = dist / CS488Window::m_windowWidth;

	    if( mL && ( scale_factors[0] > 0.05f || scale_amt > 0.0f ) ) {
	        scale_factors[0] += scale_amt;
	    }
	    if( mM && ( scale_factors[1] > 0.05f || scale_amt > 0.0f ) ) {
	        scale_factors[1] += scale_amt;
	    }
        if( mR && ( scale_factors[2] > 0.05f || scale_amt > 0.0f ) ) {
            scale_factors[2] += scale_amt;
        }
	} else if( mode == T ) {
	    float dist_move = (float) dist / CS488Window::m_windowWidth * 2;

	    glm::mat4 MTr(1.0f);
	    if( mL ) {
	    	modTrans[0] += dist_move;
	        glm::mat4 lMTr(1.0f);
	        MTr[3][0] = dist_move;
	    }
	    if( mM ) {
			modTrans[1] += dist_move;
            glm::mat4 mMTr(1.0f);
            MTr[3][1] = dist_move;
	    }
        if( mR ) {
			modTrans[2] += dist_move;
            glm::mat4 rMTr(1.0f);
            MTr[3][2] = dist_move;
        }
        MT = MT * MTr;
	} else if( mode == E ) {
        float dist_move = (float) dist / CS488Window::m_windowWidth * 2;

        glm::mat4 wMTr(1.0f);
        if( mL ) {
			viewTrans[0] += dist_move;
            glm::mat4 lMTr(1.0f);
            wMTr[3][0] = dist_move;
        }
        if( mM ) {
			viewTrans[1] += dist_move;
            glm::mat4 mMTr(1.0f);
            wMTr[3][1] = dist_move;
        }
        if( mR ) {
			viewTrans[2] += dist_move;
            glm::mat4 rMTr(1.0f);
            wMTr[3][2] = dist_move;
        }
        wMT = wMT * wMTr;
	} else if( mode == O ) {
        double rotation_amt = 360 * dist / CS488Window::m_windowWidth;
        auto angle = (float) radians( rotation_amt );

        // Make transformation matrix depending on which mouse button is down.
        if( mL ) {
			viewRot[0] += angle;
			if ( viewRot[0] > PI ) viewRot[0] -= PI;
			if ( viewRot[0] < -PI ) viewRot[0] += PI;
            glm::mat4 rML(1.0f);
            rML[1][1] = cos( angle );
            rML[2][1] = -sin( angle );
            rML[1][2] = sin( angle );
            rML[2][2] = cos( angle );
            wMT = wMT * rML;
        }
        if( mM ) {
			viewRot[1] += angle;
			if ( viewRot[1] > PI ) viewRot[1] -= PI;
			if ( viewRot[1] < -PI ) viewRot[1] += PI;
            glm::mat4 rMM(1.0f);
            rMM[0][0] = cos( angle );
            rMM[0][2] = -sin( angle );
            rMM[2][0] = sin( angle );
            rMM[2][2] = cos( angle );
            wMT = wMT * rMM;
        }
        if( mR ) {
			viewRot[2] += angle;
			if ( viewRot[2] > PI ) viewRot[2] -= PI;
			if ( viewRot[2] < -PI ) viewRot[2] += PI;
            glm::mat4 rMR(1.0f);
            rMR[0][0] = cos( angle );
            rMR[1][0] = -sin( angle );
            rMR[0][1] = sin( angle );
            rMR[1][1] = cos( angle );
            wMT = wMT * rMR;
        }
	} else if( mode == V ) {
	    if( mL ) {
	        // change X pos to be in terms of a 2x2 grid ( -1 to +1 )
	        double xRel = xPos / CS488Window::m_windowWidth * 2 - 1;
            double yRel = -( yPos / CS488Window::m_windowHeight * 2 - 1 );

            if (firstClick) {
                // Ensure that when the mouse is first clicked, the window size is 0
                firstClick = false;
                startX = xRel;
                startY = yRel;
            }
            // update window corner
            endX = xRel;
            endY = yRel;

            // scale world accordingly
            vMT = glm::mat4(1.0f);

            vMT[0][0] = (float) ( endX - startX ) / 2;
            vMT[1][1] = (float) ( endY - startY ) / 2;

            // translate accordingly
            vMT[3][0] = (float) ( endX + startX ) / 2;
            vMT[3][1] = (float) ( endY + startY ) / 2;

            float xNorm = ( endX > startX )? 1 : -1;
            float yNorm = ( endY > startY )? 1 : -1;

            normals[1] = glm::vec4( glm::vec3( -xNorm,      0, 0 ), 1 );
            normals[3] = glm::vec4( glm::vec3(  xNorm,      0, 0 ), 1 );
            normals[0] = glm::vec4( glm::vec3(      0,  yNorm, 0 ), 1 );
            normals[2] = glm::vec4( glm::vec3(      0, -yNorm, 0 ), 1 );
        }
	} else if( mode == P ) {
		double relDist = dist / CS488Window::m_windowWidth;
		if( mL ) {
			if ( FoV + relDist * 155 < 160 && FoV + relDist * 155 > 5 )
				FoV += relDist * 155;
		}
		if( mM ) {
			if( n + relDist * 2 < f && n > -relDist * 2 ) n += relDist * 2;
		}
		if( mR ) {
			if( n < f + relDist * 2 ) f += relDist * 2;
		}
		pMT = setupPMat();
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
				firstClick = true;

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

				if( mode == V ) {
					cout << endX << endl;
					if( endX >  1 ) endX = 1;
					if( endX < -1 ) endX = -1;
					if( endY >  1 ) endY = 1;
					if( endY < -1 ) endY = -1;

					// scale world accordingly
					vMT = glm::mat4(1.0f);

					vMT[0][0] = (float) ( endX - startX ) / 2;
					vMT[1][1] = (float) ( endY - startY ) / 2;

					// translate accordingly
					vMT[3][0] = (float) ( endX + startX ) / 2;
					vMT[3][1] = (float) ( endY + startY ) / 2;
				}

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
		} else if( key == GLFW_KEY_A ) {
		    reset();

		    eventHandled = true;
		}
	}

	return eventHandled;
}
