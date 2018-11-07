// Fall 2018

#include "GeometryNode.hpp"

//---------------------------------------------------------------------------------------
GeometryNode::GeometryNode(
	const std::string & name, Primitive *prim, Material *mat )
	: SceneNode( name )
	, m_material( mat )
	, m_primitive( prim )
{
	m_nodeType = NodeType::GeometryNode;
}

void GeometryNode::setMaterial( Material *mat )
{
	// Obviously, there's a potential memory leak here.  A good solution
	// would be to use some kind of reference counting, as in the 
	// C++ shared_ptr.  But I'm going to punt on that problem here.
	// Why?  Two reasons:
	// (a) In practice we expect the scene to be constructed exactly
	//     once.  There's no reason to believe that materials will be
	//     repeatedly overwritten in a GeometryNode.
	// (b) A ray tracer is a program in which you compute once, and 
	//     throw away all your data.  A memory leak won't build up and
	//     crash the program.

	m_material = mat;
}

surface GeometryNode::intersection( ray r ) {
    glm::vec3 oldE = r.E;
    glm::vec4 newE = glm::inverse( hiertrans ) * glm::vec4( r.E, 1.0f );
    glm::vec4 newP = glm::inverse( hiertrans ) * glm::vec4( r.P, 1.0f );

    r.E = glm::vec3( newE );
    r.P = glm::vec3( newP );

    r.C = glm::normalize( r.P - r.E );

	//std::cout << r.C.x << " " << r.C.y << " " << r.C.z << std::endl;

	surface s = m_primitive->intersection( r );
	if( s.intersected ) {
	    s.mat = ( PhongMaterial * )m_material;
	    s.n = glm::normalize( s.n );
	}
	s.trans = hiertrans;
	s.intersect_pt = glm::vec3( s.trans * glm::vec4( s.intersect_pt, 1.0f ) );
	s.n = glm::normalize( glm::vec3( glm::vec4( s.n, 0.0f ) * glm::inverse( s.trans ) ) );

	// re-calculate t
	s.t = glm::distance( oldE, s.intersect_pt );

	//if( s.intersected ) std::cout << m_name << ": " << s.t << std::endl;

	return s;
}
