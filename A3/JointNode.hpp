// Fall 2018

#pragma once

#include "SceneNode.hpp"

#include <stack>

class JointNode : public SceneNode {
public:
	JointNode(const std::string & name);
	virtual ~JointNode();

	void set_joint_x(double min, double init, double max);
	void set_joint_y(double min, double init, double max);

    void reset();
    void joint_rotate(char axis, float angle);
    void step();
    void undo();
    void redo();
    void fullUndo();

	struct JointRange {
		double min, init, max;
	};

	struct angles {
	    double x, y;
	};


	JointRange m_joint_x, m_joint_y;

	Material material;
    std::string meshId;

    double xCurRot, yCurRot;

    //undo redo stack
	std::stack<angles> undoStack;
	std::stack<angles> redoStack;
};
