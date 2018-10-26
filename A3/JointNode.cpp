// Fall 2018

#include "JointNode.hpp"

//---------------------------------------------------------------------------------------
JointNode::JointNode(const std::string& name)
	: SceneNode(name)
{
	m_nodeType = NodeType::JointNode;

	//material = red;
	//meshId = "sphere";

	xCurRot = 0.0f;
	yCurRot = 0.0f;

}

//---------------------------------------------------------------------------------------
JointNode::~JointNode() {

}

//---------------------------------------------------------------------------------------
void JointNode::reset() {
    joint_rotate( 'x', float ( m_joint_x.init - xCurRot ) );
    joint_rotate( 'y', float ( m_joint_y.init - yCurRot ) );

    xCurRot = m_joint_x.init;
    yCurRot = m_joint_y.init;

    while( !undoStack.empty() ) {
    	undoStack.pop();
    }
	while( !redoStack.empty() ) {
		redoStack.pop();
	}
}

//---------------------------------------------------------------------------------------
void JointNode::set_joint_x(double min, double init, double max) {
	m_joint_x.min = min;
	m_joint_x.init = init;
	m_joint_x.max = max;

	joint_rotate( 'x', (float) init );
	xCurRot = init;
}

//---------------------------------------------------------------------------------------
void JointNode::set_joint_y(double min, double init, double max) {
	m_joint_y.min = min;
	m_joint_y.init = init;
	m_joint_y.max = max;

	joint_rotate( 'y', (float) init );
	yCurRot = init;
}

void JointNode::joint_rotate(char axis, float angle) {
	//if ( !isSelected ) return;

	if ( axis == 'x' ) {
		if ( !( xCurRot + angle > m_joint_x.max || xCurRot + angle < m_joint_x.min ) ) {
            jrotate('z', angle);
            xCurRot += angle;
        }
	}
	if ( axis == 'y' ) {
		if ( !( yCurRot + angle > m_joint_y.max || yCurRot + angle < m_joint_y.min ) ) {
            jrotate('x', angle);
            yCurRot += angle;
        }
	}

}

//---------------------------------------------------------------------------------------
void JointNode::step() {
	// push current transformation matrix onto stack.
    undoStack.push( angles{ xCurRot, yCurRot } );

    // clear redo stack.
    while ( !redoStack.empty() ) {
    	redoStack.pop();
    }
}

void JointNode::undo() {
	// undo 1 step
	if( undoStack.empty() ) {
	    //std::cout << "undo limit" << std::endl;
	    return;
	}

	//std::cout << undoStack.size() << std::endl;

	// push current step to redo stack
    redoStack.push( angles{ xCurRot, yCurRot } );

	// undo
	if( undoStack.top().x - xCurRot != 0 ) {
        //std::cout << m_name << " : " << xCurRot << std::endl;
        float angle =  undoStack.top().x - xCurRot;
	    jrotate( 'z', float( undoStack.top().x - xCurRot ) );
	    xCurRot += angle;
	}
    if( undoStack.top().y - yCurRot != 0 ) {
        //std::cout << m_name << " : " << yCurRot << std::endl;
        auto angle = float( undoStack.top().y - yCurRot );
        jrotate( 'x', float( undoStack.top().y - yCurRot ) );
        yCurRot += angle;
    }

    // remove step from undo stack
	undoStack.pop();
}

void JointNode::redo() {
	// redo 1 step
	if( redoStack.empty() ) {
	    //std::cout << "redo limit" << std::endl;
        return;
	}

    //std::cout << redoStack.size() << std::endl;

	// push current step to undo stack
    undoStack.push( angles{ xCurRot, yCurRot } );

    // redo
    if( redoStack.top().x - xCurRot != 0 ) {
        auto angle = float( redoStack.top().x - xCurRot );
        jrotate( 'z', float( redoStack.top().x - xCurRot ) );
        xCurRot += angle;
    }
    if( redoStack.top().y - yCurRot != 0 ) {
        auto angle = float( redoStack.top().y - yCurRot );
        jrotate( 'x', float( redoStack.top().y - yCurRot ) );
        xCurRot += angle;
    }

	// remove step from redo stack
	redoStack.pop();
}