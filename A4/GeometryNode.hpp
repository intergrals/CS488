// Fall 2018

#pragma once

#include "SceneNode.hpp"
#include "Primitive.hpp"
#include "Material.hpp"

class GeometryNode : public SceneNode {
public:
	GeometryNode( const std::string & name, Primitive *prim, 
		Material *mat = nullptr );

	void setMaterial( Material *material );
	surface intersection( glm::vec3 E, glm::vec3 C );

	Material *m_material;
	Primitive *m_primitive;
};
