// Fall 2018

#include "A3.hpp"
#include "scene_lua.hpp"
using namespace std;

#include "cs488-framework/GlErrorCheck.hpp"
#include "cs488-framework/MathUtils.hpp"
#include "GeometryNode.hpp"
#include "JointNode.hpp"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <random>

using namespace glm;

static bool show_gui = true;

const size_t CIRCLE_PTS = 48;

//----------------------------------------------------------------------------------------
// Constructor
A3::A3(const std::string & luaSceneFile)
	: m_luaSceneFile(luaSceneFile),
	  m_positionAttribLocation(0),
	  m_normalAttribLocation(0),
	  m_vao_meshData(0),
	  m_vbo_vertexPositions(0),
	  m_vbo_vertexNormals(0),
	  m_vao_arcCircle(0),
	  m_vbo_arcCircle(0)
{

}

//----------------------------------------------------------------------------------------
// Destructor
A3::~A3()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A3::init()
{
	// Set the background colour.
	glClearColor(0.85, 0.85, 0.85, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao_arcCircle);
	glGenVertexArrays(1, &m_vao_meshData);
	enableVertexShaderInputSlots();

	initViewMatrix();

	matStack.push( m_view );

	processLuaSceneFile(m_luaSceneFile);

	initRotation( *m_rootNode );

	// Load and decode all .obj files at once here.  You may add additional .obj files to
	// this list in order to support rendering additional mesh types.  All vertex
	// positions, and normals will be extracted and stored within the MeshConsolidator
	// class.
	unique_ptr<MeshConsolidator> meshConsolidator (new MeshConsolidator{
			getAssetFilePath("cube.obj"),
			getAssetFilePath("sphere.obj"),
			getAssetFilePath("suzanne.obj")
	});


	// Acquire the BatchInfoMap from the MeshConsolidator.
	meshConsolidator->getBatchInfoMap(m_batchInfoMap);

	// Take all vertex data within the MeshConsolidator and upload it to VBOs on the GPU.
	uploadVertexDataToVbos(*meshConsolidator);

	mapVboDataToVertexShaderInputLocations();

	initLightSources();

	// NOTE: Do not move these to reset
	lmb = false;
	mmb = false;
	rmb = false;

	lastX = 0.0f;
	lastY = 0.0f;
	cLastX = 0.0f;
	cLastY = 0.0f;

	options = optionFlags{ false, true, false, false };

    initPerspectiveMatrix();

    initViewMatrix();

    initLightSources();

    do_picking = false;
    // END NOTE

	reset( A );

	// Exiting the current scope calls delete automatically on meshConsolidator freeing
	// all vertex data resources.  This is fine since we already copied this data to
	// VBOs on the GPU.  We have no use for storing vertex data on the CPU side beyond
	// this point.
}

//----------------------------------------------------------------------------------------
void A3::reset( resetTypes r ) {

	// Reset view, perspective, and light
	if( r == A || r == I ) {
		initPerspectiveMatrix();
		initViewMatrix();

		// reset camera location
		for ( double &i : c_loc ) i = 0.0f;
        updateViewMatrix();
	}
	if( r == A || r == O ) {
        resetTransform( *m_rootNode );
        xRot = 0.0f;
        yRot = 0.0f;
	}
	if( r == A || r == S ) {
		resetJoints( *m_rootNode );
		undoAmt = 0;
		redoAmt = 0;
	}
    if( r == A ) mode = P;

}

//----------------------------------------------------------------------------------------
void A3::resetTransform( SceneNode &node ) {

    for ( SceneNode * n : node.children ) {
        //n->rotate( 'y', -xRot );
        //n->rotate( 'x', -yRot );
        n->resetRot();

    }
}

//----------------------------------------------------------------------------------------
void A3::initRotation( SceneNode &node ) {
	node.initRotMat();
	for ( SceneNode * n : node.children ) {
		initRotation( *n );
	}
}


//----------------------------------------------------------------------------------------
void A3::processLuaSceneFile(const std::string & filename) {
	// This version of the code treats the Lua file as an Asset,
	// so that you'd launch the program with just the filename
	// of a puppet in the Assets/ directory.
	// std::string assetFilePath = getAssetFilePath(filename.c_str());
	// m_rootNode = std::shared_ptr<SceneNode>(import_lua(assetFilePath));

	// This version of the code treats the main program argument
	// as a straightforward pathname.
	m_rootNode = std::shared_ptr<SceneNode>(import_lua(filename));
	if (!m_rootNode) {
		std::cerr << "Could Not Open " << filename << std::endl;
	}
}

//----------------------------------------------------------------------------------------
void A3::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();

	m_shader_arcCircle.generateProgramObject();
	m_shader_arcCircle.attachVertexShader( getAssetFilePath("arc_VertexShader.vs").c_str() );
	m_shader_arcCircle.attachFragmentShader( getAssetFilePath("arc_FragmentShader.fs").c_str() );
	m_shader_arcCircle.link();
}

//----------------------------------------------------------------------------------------
void A3::enableVertexShaderInputSlots()
{
	//-- Enable input slots for m_vao_meshData:
	{
		glBindVertexArray(m_vao_meshData);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_positionAttribLocation = m_shader.getAttribLocation("position");
		glEnableVertexAttribArray(m_positionAttribLocation);

		// Enable the vertex shader attribute location for "normal" when rendering.
		m_normalAttribLocation = m_shader.getAttribLocation("normal");
		glEnableVertexAttribArray(m_normalAttribLocation);

		CHECK_GL_ERRORS;
	}


	//-- Enable input slots for m_vao_arcCircle:
	{
		glBindVertexArray(m_vao_arcCircle);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_arc_positionAttribLocation = m_shader_arcCircle.getAttribLocation("position");
		glEnableVertexAttribArray(m_arc_positionAttribLocation);

		CHECK_GL_ERRORS;
	}

	// Restore defaults
	glBindVertexArray(0);
}

//----------------------------------------------------------------------------------------
void A3::uploadVertexDataToVbos (
		const MeshConsolidator & meshConsolidator
) {
	// Generate VBO to store all vertex position data
	{
		glGenBuffers(1, &m_vbo_vertexPositions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexPositionBytes(),
				meshConsolidator.getVertexPositionDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store all vertex normal data
	{
		glGenBuffers(1, &m_vbo_vertexNormals);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexNormalBytes(),
				meshConsolidator.getVertexNormalDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store the trackball circle.
	{
		glGenBuffers( 1, &m_vbo_arcCircle );
		glBindBuffer( GL_ARRAY_BUFFER, m_vbo_arcCircle );

		float *pts = new float[ 2 * CIRCLE_PTS ];
		for( size_t idx = 0; idx < CIRCLE_PTS; ++idx ) {
			float ang = 2.0 * M_PI * float(idx) / CIRCLE_PTS;
			pts[2*idx] = cos( ang );
			pts[2*idx+1] = sin( ang );
		}

		glBufferData(GL_ARRAY_BUFFER, 2*CIRCLE_PTS*sizeof(float), pts, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A3::mapVboDataToVertexShaderInputLocations()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_meshData);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexPositions" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);
	glVertexAttribPointer(m_positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexNormals" into the
	// "normal" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);
	glVertexAttribPointer(m_normalAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;

	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_arcCircle);

	// Tell GL how to map data from the vertex buffer "m_vbo_arcCircle" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_arcCircle);
	glVertexAttribPointer(m_arc_positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A3::initPerspectiveMatrix()
{
	float aspect = ((float)m_windowWidth) / m_windowHeight;
	m_perpsective = glm::perspective(degreesToRadians(60.0f), aspect, 0.1f, 100.0f);
}


//----------------------------------------------------------------------------------------
void A3::initViewMatrix() {
	m_view = glm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f),
			vec3(0.0f, 1.0f, 0.0f));
}

void A3::updateViewMatrix() {
	m_view = glm::lookAt(vec3(c_loc[0], c_loc[1], c_loc[2]),
						 vec3(c_loc[0], c_loc[1], c_loc[2] - 1.0f),
						 vec3(0.0f, 1.0f, 0.0f));

	if ( matStack.size() != 1 ) throw "What the heck!! Why is the size not 1?!";

	matStack.pop();
	matStack.push( m_view );
}

//----------------------------------------------------------------------------------------
void A3::initLightSources() {
	// World-space position
	m_light.position = vec3(-2.0f, 5.0f, 0.5f);
	m_light.rgbIntensity = vec3(0.8f); // White light
}

//----------------------------------------------------------------------------------------
void A3::uploadCommonSceneUniforms() {
    m_shader.enable();
    {
        //-- Set Perpsective matrix uniform for the scene:
        GLint location = m_shader.getUniformLocation("Perspective");
        glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perpsective));
        CHECK_GL_ERRORS;

        location = m_shader.getUniformLocation("picking");
        glUniform1i( location, do_picking ? 1 : 0 );

        if( !do_picking ) {
            //-- Set LightSource uniform for the scene:
            {
                location = m_shader.getUniformLocation("light.position");
                glUniform3fv(location, 1, value_ptr(m_light.position));
                location = m_shader.getUniformLocation("light.rgbIntensity");
                glUniform3fv(location, 1, value_ptr(m_light.rgbIntensity));
                CHECK_GL_ERRORS;
            }

            //-- Set background light ambient intensity
            {
                location = m_shader.getUniformLocation("ambientIntensity");
                vec3 ambientIntensity(0.05f);
                glUniform3fv(location, 1, value_ptr(ambientIntensity));
                CHECK_GL_ERRORS;
            }
        }
    }
    m_shader.disable();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A3::appLogic()
{
	// Place per frame, application logic here ...

	uploadCommonSceneUniforms();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A3::guiLogic()
{
	if( !show_gui ) {
		return;
	}

	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize /*+ ImGuiWindowFlags_MenuBar*/ );
	float opacity(0.5f);

	ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags );


		// Add more gui elements here here ...
		string undoText = "[U]ndo (" + to_string(undoAmt) + ")";
        string redoText = "[R]edo (" + to_string(redoAmt) + ")";

        char uText[ undoText.length() + 1 ];
        for( int i = 0; i <= undoText.length(); i++ ) {
            uText[i] = undoText[i];
        }
        char rText[ redoText.length() + 1 ];
        for( int i = 0; i <= redoText.length(); i++ ) {
            rText[i] = redoText[i];
        }


		// Menu bar items
		if( ImGui::BeginMainMenuBar() ) {
			if( ImGui::BeginMenu( "Application" ) ) {
				if( ImGui::MenuItem( "Reset Pos[i]tion" ) ) {
					reset( I );
				}
				if( ImGui::MenuItem( "Reset [O]rientation") ) {
					reset( O );
				}
				if( ImGui::MenuItem( "Reset Joint[s]") ) {
					reset( S );
				}
				if( ImGui::MenuItem( "Reset [A]ll") ) {
					reset( A );
				}
				if( ImGui::MenuItem( "[Q]uit" ) ) {
					glfwSetWindowShouldClose(m_window, GL_TRUE);
				}
				ImGui::EndMenu();
			}
			if( ImGui::BeginMenu( "Edit" ) ) {
				if( ImGui::MenuItem( uText ) ) {
                    undoJoints( *m_rootNode );
				}
				if( ImGui::MenuItem( rText ) ) {
                    redoJoints( *m_rootNode );
				}
				ImGui::EndMenu();
			}
			if( ImGui::BeginMenu( "Options" ) ) {
				if( ImGui::MenuItem( "[C]ircle", nullptr, &options.circle ) ) {

				}
				if( ImGui::MenuItem( "[Z]-Buffer", nullptr, &options.zbuff ) ) {

				}
				if( ImGui::MenuItem( "[B]ackface Culling", nullptr, &options.bcull ) ) {

				}
				if( ImGui::MenuItem( "[F]rontface Culling", nullptr, &options.fcull ) ) {

				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		// Radio buttons to select mode
		ImGui::Text( "Interaction Mode" );
		ImGui::PushID( 0 );
		ImGui::RadioButton( "[P]osition/Orientation", &mode, P );
		ImGui::RadioButton( "[J]oints", &mode, J );
		ImGui::PopID();

		// Create Button, and check if it was clicked:
		if( ImGui::Button( "Hide [M]enu" ) ) {
			show_gui = false;
		}

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();
}

//----------------------------------------------------------------------------------------
// Update mesh specific shader uniforms:
void A3::updateShaderUniforms(
		const ShaderProgram & shader,
		const GeometryNode & node,
		const glm::mat4 & viewMatrix
) {

	shader.enable();
    if( do_picking ) {
		//-- Set ModelView matrix:
		GLint location = m_shader.getUniformLocation("ModelView");
		mat4 modelView = viewMatrix * node.trans;
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));

        float r = float(   node.m_nodeId         & 0xff ) / 255.0f;
        float g = float( ( node.m_nodeId >> 8 )  & 0xff ) / 255.0f;
        float b = float( ( node.m_nodeId >> 16 ) & 0xff ) / 255.0f;

        location = m_shader.getUniformLocation("material.kd");
        glUniform3f( location, r, g, b );
        CHECK_GL_ERRORS;
    } else {
		//-- Set ModelView matrix:
		GLint location = shader.getUniformLocation("ModelView");
		mat4 modelView = viewMatrix * node.trans;
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
		CHECK_GL_ERRORS;

		//-- Set NormMatrix:
		location = shader.getUniformLocation("NormalMatrix");
		mat3 normalMatrix = glm::transpose(glm::inverse(mat3(modelView)));
		glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(normalMatrix));
		CHECK_GL_ERRORS;


		//-- Set Material values:
		location = shader.getUniformLocation("material.kd");
		vec3 kd = node.material.kd;

		// TODO: Change this color back to red.
		if( node.isSelected ) glUniform3fv(location, 1, value_ptr( vec3( 1.0f, 1.0f, 0.0f ) ) );
		else glUniform3fv(location, 1, value_ptr(kd));
		CHECK_GL_ERRORS;
		location = shader.getUniformLocation("material.ks");
		vec3 ks = node.material.ks;
		if( node.isSelected ) glUniform3fv( location, 1, value_ptr( vec3( 0.0f, 0.0f, 0.0f ) ) );
		else glUniform3fv(location, 1, value_ptr(ks));
		CHECK_GL_ERRORS;
		location = shader.getUniformLocation("material.shininess");
		glUniform1f(location, node.material.shininess);
		CHECK_GL_ERRORS;

	}
	shader.disable();

}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A3::draw() {

    // depth buffering if enabled
	if( options.zbuff ) glEnable( GL_DEPTH_TEST );

	// cull front and back if enabled
	if( options.fcull && options.bcull ) {
        glEnable( GL_CULL_FACE );
        glCullFace( GL_FRONT_AND_BACK );
    } else if ( options.fcull ) {
	        glEnable( GL_CULL_FACE );
	        glCullFace( GL_FRONT );
	} else if( options.bcull ) {
        glEnable( GL_CULL_FACE );
	    glCullFace( GL_BACK );
	}
	renderSceneGraph(*m_rootNode);

    if( options.fcull || options.bcull ) glDisable( GL_DEPTH_TEST );
    if( options.zbuff ) glDisable( GL_DEPTH_TEST );

	if( options.circle ) renderArcCircle();
}

//
void A3::renderNode( const SceneNode &n ) {

    // This is emphatically *not* how you should be drawing the scene graph in
    // your final implementation.  This is a non-hierarchical demonstration
    // in which we assume that there is a list of GeometryNodes living directly
    // underneath the root node, and that we can draw them in a loop.  It's
    // just enough to demonstrate how to get geometry and materials out of
    // a GeometryNode and onto the screen.

    // You'll want to turn this into recursive code that walks over the tree.
    // You can do that by putting a method in SceneNode, overridden in its
    // subclasses, that renders the subtree rooted at every node.  Or you
    // could put a set of mutually recursive functions in this class, which
    // walk down the tree from nodes of different types.

    matStack.push( matStack.top() * n.trans );

    for (const SceneNode * node : n.children) {

        if (node->m_nodeType == NodeType::GeometryNode) {

            const GeometryNode *geometryNode = static_cast<const GeometryNode *>(node);

            updateShaderUniforms(m_shader, *geometryNode, matStack.top());

            // Get the BatchInfo corresponding to the GeometryNode's unique MeshId.
            BatchInfo batchInfo = m_batchInfoMap[geometryNode->meshId];

            //-- Now render the mesh:
            m_shader.enable();
            glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
            m_shader.disable();

            renderNode(*node);
        } else if (node->m_nodeType == NodeType::JointNode) {
            const JointNode *jointNode = static_cast<const JointNode *>(node);

            mat4 modelView = matStack.top() * jointNode->trans;

            renderNode(*node);
        }
    }

    matStack.pop();
}

//----------------------------------------------------------------------------------------
void A3::renderSceneGraph(const SceneNode & root) {
    // Bind the VAO once here, and reuse for all GeometryNode rendering below.
    glBindVertexArray(m_vao_meshData);

    renderNode( root );


    glBindVertexArray(0);
    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A3::updateNodePicking( SceneNode &n, int id ) {

    if( n.m_nodeType != NodeType::JointNode ) {
        for ( SceneNode * node : n.children ) {
            updateNodePicking( *node, id );
        }
        return;
    }

    for ( SceneNode * node : n.children ) {

        if( node->m_nodeId == id ) {

            node->isSelected = !node->isSelected;
            n.isSelected = node->isSelected;
            return;
        }
        updateNodePicking( *node, id );
    }
}


//----------------------------------------------------------------------------------------
// Draw the trackball circle.
void A3::renderArcCircle() {
	glBindVertexArray(m_vao_arcCircle);

	m_shader_arcCircle.enable();
		GLint m_location = m_shader_arcCircle.getUniformLocation( "M" );
		float aspect = float(m_framebufferWidth)/float(m_framebufferHeight);
		glm::mat4 M;
		if( aspect > 1.0 ) {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5/aspect, 0.5, 1.0 ) );
		} else {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5, 0.5*aspect, 1.0 ) );
		}
		glUniformMatrix4fv( m_location, 1, GL_FALSE, value_ptr( M ) );
		glDrawArrays( GL_LINE_LOOP, 0, CIRCLE_PTS );
	m_shader_arcCircle.disable();

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
// Rotate all selected joints
void A3::rotateJoints( SceneNode &node, double rotAmt ) {
	// TEST rotate
	for ( SceneNode * n : node.children ) {
		if ( n->m_nodeType == NodeType::JointNode && n->isSelected ) {
			JointNode *jointNode = static_cast<JointNode *>(n);
			jointNode->joint_rotate( 'x', (float)rotAmt );
		}

		rotateJoints( *n, rotAmt );
	}
}

//----------------------------------------------------------------------------------------
// Reset all joints
void A3::resetJoints( SceneNode &node ) {
	// TODO: Change x to whatever axis joint rotates on
	for ( SceneNode * n : node.children ) {
		if ( n->m_nodeType == NodeType::JointNode ) {
			JointNode *jointNode = static_cast<JointNode *>(n);
			//jointNode->joint_rotate( 'x', (float) ( jointNode->m_joint_x.init - jointNode->xCurRot ) );
			jointNode->reset();
		}
		n->isSelected = false;

		resetJoints( *n );
	}
}

//----------------------------------------------------------------------------------------
// Step all joints
void A3::stepJoints( SceneNode &node ) {

    for ( SceneNode * n : node.children ) {
        if( n->m_nodeType == NodeType::JointNode ) {
            JointNode *jointNode = static_cast<JointNode *>(n);
            jointNode->step();
            undoAmt = (int)jointNode->undoStack.size();
            redoAmt = (int)jointNode->redoStack.size();
            cout << "step" << endl;
        }
        stepJoints( *n );
    }
}

//----------------------------------------------------------------------------------------
// Undo all joints
void A3::undoJoints(SceneNode &node) {
    for ( SceneNode * n : node.children ) {
        if( n->m_nodeType == NodeType::JointNode ) {
            JointNode *jointNode = static_cast<JointNode *>(n);
            jointNode->undo();
            undoAmt = (int)jointNode->undoStack.size();
            redoAmt = (int)jointNode->redoStack.size();
        }
        undoJoints( *n );
    }
}

//----------------------------------------------------------------------------------------
// Redo all joints
void A3::redoJoints(SceneNode &node) {
    for ( SceneNode * n : node.children ) {
        if( n->m_nodeType == NodeType::JointNode ) {
            JointNode *jointNode = static_cast<JointNode *>(n);
            jointNode->redo();
            undoAmt = (int)jointNode->undoStack.size();
            redoAmt = (int)jointNode->redoStack.size();
        }
        redoJoints( *n );
    }
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A3::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A3::cursorEnterWindowEvent (
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
bool A3::mouseMoveEvent (
		double xPos,
		double yPos
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	double changeX = xPos - lastX;
	double changeY = yPos - lastY;

    // Change of coordinates of mouse location.
    double relX, relY;
    if( m_windowHeight < m_windowWidth ) {
        double clippedX = xPos - ( m_windowWidth - m_windowHeight ) / 2.0f;

        relX = ( clippedX - m_windowHeight / 2.0f ) / m_windowHeight * 4;
        relY = ( yPos - m_windowHeight / 2.0f ) / m_windowHeight * 4;
    } else {
        double clippedY = yPos - ( m_windowHeight - m_windowWidth ) / 2.0f;

        relX = ( xPos - m_windowWidth / 2.0f ) / m_windowWidth * 4;
        relY = ( clippedY - m_windowWidth / 2.0f ) / m_windowWidth * 4;
    }

	if( mode == P && ( lmb || mmb || rmb ) ) {
		if (lmb) {
			double deltaX = -changeX / CS488Window::m_windowWidth * 5;
			double deltaY = changeY / CS488Window::m_windowHeight * 5;

			c_loc[0] += deltaX;
			c_loc[1] += deltaY;

			eventHandled = true;
		}
		if (mmb) {
			double deltaZ = -changeY / CS488Window::m_windowHeight * 5;

			if (c_loc[2] + deltaZ > -10 && c_loc[2] + deltaZ < 100) c_loc[2] += deltaZ;

			eventHandled = true;
		}
		if (rmb) {

            double m_Z = 1.0f - relX * relX - relY * relY;

            if( m_Z > 0 ) {
                int minAspect = glm::min( m_windowWidth, m_windowHeight );

                auto rotXAmt = (float) changeX / minAspect * 360 * 2;
                auto rotYAmt = (float) changeY / minAspect * 360 * 2;

                for (SceneNode *node : m_rootNode->children) {
                    node->rotate('y', rotXAmt);
                    node->rotate('x', rotYAmt);

                    xRot += rotXAmt;
                    yRot += rotYAmt;
                }
            } else {
                glm::vec3 lastPos( cLastX, cLastY, 0 );
                glm::vec3 curPos( relX, relY, 0 );
                float rotZAmt = glm::length( glm::cross( lastPos, curPos ) ) / ( glm::length( lastPos ) * glm::length( curPos ) );
                rotZAmt = glm::degrees( glm::asin( rotZAmt ) );

                if( glm::cross( lastPos, curPos ).z > 0 ) rotZAmt *= -1;

                for (SceneNode *node : m_rootNode->children) {
                    node->rotate('z', rotZAmt );

                    //xRot += rotXAmt;
                }
            }


			eventHandled = true;
		}
		updateViewMatrix();
	} else if( mode == J && ( lmb || mmb || rmb ) ) {
	    if (lmb) {
	        // TODO: implement selection
	    } else if (mmb) {
			auto rotAmt = (float) changeY / m_windowHeight * 180;

			rotateJoints( *m_rootNode, rotAmt );
	    }
	}

	// update previous mouse positions
	lastX = xPos;
	lastY = yPos;

    cLastX = relX;
    cLastY = relY;

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A3::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if( actions == GLFW_PRESS ) {
		if( button == GLFW_MOUSE_BUTTON_LEFT ) {
			lmb = true;

            if( mode == J ) {
                // Handle picking
                double xpos, ypos;
                glfwGetCursorPos(m_window, &xpos, &ypos);

                do_picking = true;

                uploadCommonSceneUniforms();
                glClearColor(1.0, 1.0, 1.0, 1.0);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glClearColor(0.85, 0.85, 0.85, 1.0);

                draw();

                // I don't know if these are really necessary anymore.
                // glFlush();
                // glFinish();

                CHECK_GL_ERRORS;

                // Ugly -- FB coordinates might be different than Window coordinates
                // (e.g., on a retina display).  Must compensate.
                xpos *= double(m_framebufferWidth) / double(m_windowWidth);
                // WTF, don't know why I have to measure y relative to the bottom of
                // the window in this case.
                ypos = m_windowHeight - ypos;
                ypos *= double(m_framebufferHeight) / double(m_windowHeight);

                GLubyte buffer[4] = {0, 0, 0, 0};
                // A bit ugly -- don't want to swap the just-drawn false colours
                // to the screen, so read from the back buffer.
                glReadBuffer(GL_BACK);
                // Actually read the pixel at the mouse location.
                glReadPixels(int(xpos), int(ypos), 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
                CHECK_GL_ERRORS;

                // Reassemble the object ID.
                unsigned int what = buffer[0] + (buffer[1] << 8) + (buffer[2] << 16);

                updateNodePicking(*m_rootNode, what);

                do_picking = false;

                CHECK_GL_ERRORS;
            }


			eventHandled = true;
		} else if( button == GLFW_MOUSE_BUTTON_MIDDLE ) {
			mmb = true;

			if( mode == J ) stepJoints( *m_rootNode );

			eventHandled = true;
		} else if( button == GLFW_MOUSE_BUTTON_RIGHT ) {
			rmb = true;
			eventHandled = true;
		}
	} else if( actions == GLFW_RELEASE ) {
		if( button == GLFW_MOUSE_BUTTON_LEFT ) {
			lmb = false;
			eventHandled = true;
		} else if( button == GLFW_MOUSE_BUTTON_MIDDLE ) {
			mmb = false;
			eventHandled = true;
		} else if( button == GLFW_MOUSE_BUTTON_RIGHT ) {
			rmb = false;
			eventHandled = true;
		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A3::mouseScrollEvent (
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
bool A3::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);
	initPerspectiveMatrix();
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A3::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);

	if( action == GLFW_PRESS ) {
		if( key == GLFW_KEY_M ) {
			show_gui = !show_gui;
			eventHandled = true;
		} else if( key == GLFW_KEY_Q ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
			eventHandled = true;
		} else if( key == GLFW_KEY_P ) {
			mode = P;
			eventHandled = true;
		} else if( key == GLFW_KEY_J ) {
			mode = J;
			eventHandled = true;
		} else if( key == GLFW_KEY_I ) {
			reset( I );
			eventHandled = true;
		} else if( key == GLFW_KEY_O ) {
			reset( O );
			eventHandled = true;
		} else if( key == GLFW_KEY_S ) {
			reset( S );
			eventHandled = true;
		} else if( key == GLFW_KEY_A ) {
			reset( A );
			eventHandled = true;
		} else if( key == GLFW_KEY_C ) {
			options.circle = !options.circle;
			eventHandled = true;
		} else if( key == GLFW_KEY_Z ) {
			options.zbuff = !options.zbuff;
			eventHandled = true;
		} else if( key == GLFW_KEY_B ) {
			options.bcull = !options.bcull;
			eventHandled = true;
		} else if( key == GLFW_KEY_F ) {
			options.fcull = !options.fcull;
			eventHandled = true;
		} else if( key == GLFW_KEY_U ) {
		    undoJoints( *m_rootNode );
		    eventHandled = true;
		} else if( key == GLFW_KEY_R ) {
		    redoJoints( *m_rootNode );
		    eventHandled = true;
		}
	}
	// Fill in with event handling code...

	return eventHandled;
}
