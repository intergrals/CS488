// Fall 2018

#include "JointNode.hpp"

//---------------------------------------------------------------------------------------
JointNode::JointNode(const std::string& name)
	: SceneNode(name)
{
	m_nodeType = NodeType::JointNode;

	//material = red;
	meshId = "sphere";

	xCurRot = 0.0f;
	yCurRot = 0.0f;

	// TEST rotation
	isSelected = true;
}

//---------------------------------------------------------------------------------------
JointNode::~JointNode() {

}

//---------------------------------------------------------------------------------------
void JointNode::reset() {
    jrotate( 'x', -(float) xCurRot );
    jrotate( 'y', -(float) yCurRot );

    xCurRot = m_joint_x.init;
    yCurRot = m_joint_y.init;
}

//---------------------------------------------------------------------------------------
void JointNode::set_joint_x(double min, double init, double max) {
	m_joint_x.min = min;
	m_joint_x.init = init;
	m_joint_x.max = max;

	jrotate( 'x', (float) init );
	xCurRot = init;
}

//---------------------------------------------------------------------------------------
void JointNode::set_joint_y(double min, double init, double max) {
	m_joint_y.min = min;
	m_joint_y.init = init;
	m_joint_y.max = max;

	jrotate( 'y', (float) init );
	yCurRot = init;
}

void JointNode::joint_rotate(char axis, float angle) {
	if ( !isSelected ) return;

	if ( axis == 'x' ) {
		if ( xCurRot + angle > m_joint_x.max || xCurRot + angle < m_joint_x.min ) return;
		xCurRot += angle;
	} else if ( axis == 'y' ) {
		if ( yCurRot + angle > m_joint_y.max || yCurRot + angle < m_joint_y.min ) return;
		yCurRot += angle;
	}

	jrotate( axis, angle );
}