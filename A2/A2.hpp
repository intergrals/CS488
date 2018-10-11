// Fall 2018

#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include <glm/glm.hpp>

#include <vector>

// Set a global maximum number of vertices in order to pre-allocate VBO data
// in one shot, rather than reallocating each frame.
const GLsizei kMaxVertices = 1000;


// Convenience class for storing vertex data in CPU memory.
// Data should be copied over to GPU memory via VBO storage before rendering.
class VertexData {
public:
	VertexData();

	std::vector<glm::vec2> positions;
	std::vector<glm::vec3> colours;
	GLuint index;
	GLsizei numVertices;
};


class A2 : public CS488Window {
public:
	A2();
	virtual ~A2();

protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;

	void createShaderProgram();
	void enableVertexAttribIndices();
	void generateVertexBuffers();
	void mapVboDataToVertexAttributeLocation();
	void uploadVertexDataToVbos();

	void initLineData();

	void setLineColour(const glm::vec3 & colour);

	void drawLine (
			const glm::vec2 & v0,
			const glm::vec2 & v1
	);

	void drawViewLine( glm::vec4 A, glm::vec4 B, glm::vec3 col );
	void drawWorldLine( glm::vec4 A, glm::vec4 B, glm::vec3 col );
	void drawOctahedron();
	glm::mat4 setupPMat();
	void reset();
    void clipTo( glm::vec4 &A, glm::vec4 &B, glm::vec4 P, glm::vec4 n );
    void clipping( glm::vec4 &A, glm::vec4 &B );

	// Interaction mode classifiers
	enum interaction_modes { O, E, P, R, T, S, V };

	ShaderProgram m_shader;

	GLuint m_vao;            // Vertex Array Object
	GLuint m_vbo_positions;  // Vertex Buffer Object
	GLuint m_vbo_colours;    // Vertex Buffer Object

	VertexData m_vertexData;

	glm::vec3 m_currentLineColour;


	// interaction mode
	int mode;
	// flags for mouse hold
	bool mL;
	bool mM;
	bool mR;

	// mouse position
	double lastX;

	// Frame matrices
	glm::mat4 F_w; // World frame
	glm::mat3 F_v; // View frame

	// Transformation variables
	glm::mat4 MT;           // local translation and rotation matrix
	glm::mat4 wMT;          // world transformation matrix
	float scale_factors[3]; // scaling factors for shape

	// display variables
	double viewRot[3];
	double viewTrans[3];
	double modRot[3];
	double modTrans[3];

    // viewport variables
    double startX, startY;
    double endX, endY;
    bool firstClick;
    glm::mat4 vMT;
    glm::vec4 normals[4];      // north, east, south, and west normals

    // projection variables
    glm::mat4 pMT;          // projection matrix
    double n;               // nearplane
    double f;               // farplane
    double FoV;             // Field of View
};
