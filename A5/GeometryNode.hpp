// Fall 2018

#pragma once

#include "SceneNode.hpp"
#include "Primitive.hpp"
#include "Material.hpp"

class GeometryNode : public SceneNode {
public:
	GeometryNode( const std::string & name, Primitive *prim, 
		Material *mat = nullptr );
		bool metallic = false;
        double reflectiveness = 0;
		double refractiveness = 0;

        double transparency = 0;

	void setMaterial( Material *material );
	void setMetallic( uint n );
    void setReflectiveness( double n );
    void setRefractiveness( double n );
    void setTransparency( double n );
	surface intersection( ray r );

	Material *m_material;
	Primitive *m_primitive;
};
