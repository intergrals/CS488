// Fall 2018

#pragma once

#include <glm/glm.hpp>

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include "maze.hpp"

class A1 : public CS488Window {
public:
	A1();
	virtual ~A1();

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



private:
	void initGrid();

	void reset();
	void dig();
	void initFloor();
	void initCube( float x, float y, float h );
	bool canMoveTo( int x, int y );
    bool outsideGrid( int x, int y );
	bool atBorder( int x, int y );
	void move( int dx, int dy );

	// Maze
	Maze m;

	// Fields related to the shader and uniforms.
	ShaderProgram m_shader;
	GLint P_uni; // Uniform location for Projection matrix.
	GLint V_uni; // Uniform location for View matrix.
	GLint M_uni; // Uniform location for Model matrix.
	GLint col_uni;   // Uniform location for cube colour.

	// Fields related to grid geometry.
	GLuint m_grid_vao; // Vertex Array Object
	GLuint m_grid_vbo; // Vertex Buffer Object

	// Fields related to floor geometry.
	GLuint floor_vao;	// Vertex Array Object
	GLuint floor_vbo;	// Vertex Buffer Object
	GLuint floor_ebo;	// Element Buffer Object

	// Fields related to cube
	float cube_h;		// Height of cube
	GLuint cube_vao;	// Vertex Array Object
	GLuint cube_vbo;	// Vertex Buffer Object
	GLuint cube_ebo;	// Element Buffer Object

	// Fields related to player
	int p_loc[2];        // player location
	bool chomp;          // whether or not the player can go through walls

	// Matrices controlling the camera and projection.
	glm::mat4 proj;
	glm::mat4 view;

	float colour[3][3];
	int current_col;

	// Fields controlling rotation
	bool mouse_held;        // Whether the mouse is being held down
	double prevX;           // Previous x position of mouse
	double cur_rotation;    // Net rotation
	double change_rotation; // Current change in rotation per frame

	// Fields controlling scaling
	double scale_factor;

	// Bonus content
	double c_loc[2];        // Camera translation {x, y} coordinates
	double c_follow[2];     // Camera translation {x, y} for following player
	bool follow;

	bool buggy;
};
