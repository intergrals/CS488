// Fall 2018

#include <iostream>
#include <fstream>

#include <glm/ext.hpp>

// #include "cs488-framework/ObjFileDecoder.hpp"
#include "Mesh.hpp"

Mesh::Mesh( const std::string& fname )
	: m_vertices()
	, m_faces()
{

    glm::dvec3 min = {  INFINITY,  INFINITY,  INFINITY };
    glm::dvec3 max = { -INFINITY, -INFINITY, -INFINITY };

	std::string code;
	double vx, vy, vz;
	size_t s1, s2, s3;
	std::ifstream ifs( fname.c_str() );
	if( ifs.fail() ) ifs = std::ifstream( "./Assets/" + fname );
	if( ifs.fail() ) throw( "Unable to find file " + fname );
	while( ifs >> code ) {
		if( code == "v" ) {
			ifs >> vx >> vy >> vz;
			m_vertices.push_back( glm::vec3( vx, vy, vz ) );

			min[0] = glm::min( min[0], vx );
			min[1] = glm::min( min[1], vy );
			min[2] = glm::min( min[2], vz );
            max[0] = glm::max( max[0], vx );
            max[1] = glm::max( max[1], vy );
            max[2] = glm::max( max[2], vz );
		} else if( code == "f" ) {
			ifs >> s1 >> s2 >> s3;
			m_faces.push_back( Triangle( s1 - 1, s2 - 1, s3 - 1 ) );
		}
	}

	// make bounding box
	double size = -INFINITY;
	glm::vec3 mid;
	for ( int i = 0; i < 3; i++ ) {
	    size = glm::max( size, max[i] - min[i] );
	}

	boundingBox = new NonhierBox( min, size );
}

// destructor
Mesh::~Mesh() {
	delete boundingBox;
}

surface Mesh::tri_intersection( glm::vec3 &a, glm::vec3 &b, glm::vec3 &c, ray r ) {
	//std::cout << to_string(C) << std::endl;
	surface s;

	glm::mat3 mA = {a.x - b.x, a.x - c.x, r.C.x,
				 	a.y - b.y, a.y - c.y, r.C.y,
				 	a.z - b.z, a.z - c.z, r.C.z };
	mA = glm::transpose(mA);
	double A = glm::determinant( mA );

	glm::mat3 mGamma = {a.x - b.x, a.x - r.E.x, r.C.x,
					 	a.y - b.y, a.y - r.E.y, r.C.y,
					 	a.z - b.z, a.z - r.E.z, r.C.z };
	mGamma = glm::transpose(mGamma);
	double Gamma = glm::determinant( mGamma ) / A;
	if( Gamma < 0 || Gamma > 1 ) return s;
    //std::cout << Gamma << std::endl;

	glm::mat3 mBeta = { a.x - r.E.x, a.x - c.x, r.C.x,
						a.y - r.E.y, a.y - c.y, r.C.y,
						a.z - r.E.z, a.z - c.z, r.C.z };
	mBeta = glm::transpose(mBeta);
	double Beta = glm::determinant( mBeta ) / A;
	if( Beta < 0 || Beta > 1 - Gamma ) return s;
    //std::cout << Beta << std::endl;

	glm::mat3 mt = {a.x - b.x, a.x - c.x, a.x - r.E.x,
				  	a.y - b.y, a.y - c.y, a.y - r.E.y,
				  	a.z - b.z, a.z - c.z, a.z - r.E.z };
    mt = glm::transpose(mt);
	s.t = glm::determinant( mt ) / A;

	if( s.t - r.tmin < Epsilon ) return s;

    s.intersect_pt = r.E + (float)s.t * r.C;
	s.intersected = true;

	// calc normal
	glm::vec3 ab = b - a;
	glm::vec3 ac = c - a;
	s.n = glm::cross( ab, ac );

	/*glm::vec3 v01 = v1 - v0;
	glm::vec3 v02 = v2 - v0;
	glm::vec3 N = glm::cross( v01, v02 );

	float NRDir = glm::dot( N, C );
	//if( fabs(NRDir) < 0.1 ) return false;

	float d = glm::dot( N, v0 );
	float t = glm::dot( N, E ) + d;
	if( t < 0 ) return false;

	glm::vec3 P = E + t * C;

	glm::vec3 e0 = v1 - v0;
	glm::vec3 vp0 = P - v0;
	glm::vec3 perp = glm::cross( e0, vp0 );
	if( glm::dot( N, perp ) < 0 ) return false;

	glm::vec3 e1 = v2 - v1;
	glm::vec3 vp1 = P - v1;
	perp = glm::cross( e1, vp1 );
	if( glm::dot( N, perp ) < 0 ) return false;

    glm::vec3 e2 = v0 - v2;
	glm::vec3 vp2 = P - v2;
	perp = glm::cross( e2, vp2 );
	if( glm::dot( N, perp ) < 0 ) return false;

    std::cout << "made it" << std::endl;
*/

    return s;

}

surface Mesh::intersection( ray r ) {
	surface ret;

	surface bounding = boundingBox->intersection( r );

	if( bound ) {
		return bounding;
	}

	if( !bounding.intersected ) return bounding;

	for( const auto f: m_faces ) {
		surface s = tri_intersection( m_vertices[f.v1], m_vertices[f.v2], m_vertices[f.v3], r );
		if ( !s.intersected ) {
			continue;
		}
		else if( !ret.intersected ) {
			ret = s;
		}
		else if( s.t < ret.t ) {
			ret = s;
		}
	}
	return ret;
}

std::ostream& operator<<(std::ostream& out, const Mesh& mesh)
{
  out << "mesh {";
  /*
  
  for( size_t idx = 0; idx < mesh.m_verts.size(); ++idx ) {
  	const MeshVertex& v = mesh.m_verts[idx];
  	out << glm::to_string( v.m_position );
	if( mesh.m_have_norm ) {
  	  out << " / " << glm::to_string( v.m_normal );
	}
	if( mesh.m_have_uv ) {
  	  out << " / " << glm::to_string( v.m_uv );
	}
  }

*/
  out << "}";
  return out;
}
