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
    //std::cout <<  << std::endl;
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
		} else if( code == "f" ) {
			ifs >> s1 >> s2 >> s3;
			m_faces.push_back( Triangle( s1 - 1, s2 - 1, s3 - 1 ) );
		}
	}
}

bool Mesh::tri_intersection( glm::vec3 &a, glm::vec3 &b, glm::vec3 &c, glm::vec3 &E, glm::vec3 &C ) {
	std::cout << to_string(C) << std::endl;

	glm::mat3 mA = {a.x - b.x, a.x - c.x, C.x,
				 	a.y - b.y, a.y - c.y, C.y,
				 	a.z - b.z, a.z - c.z, C.z };
	mA = glm::transpose(mA);
	double A = glm::determinant( mA );

	glm::mat3 mGamma = {a.x - b.x, a.x - E.x, C.x,
					 	a.y - b.y, a.y - E.y, C.y,
					 	a.z - b.z, a.z - E.z, C.z };
	mGamma = glm::transpose(mGamma);
	double Gamma = glm::determinant( mGamma ) / A;
	if( Gamma < 0 || Gamma > 1 ) return false;
    //std::cout << Gamma << std::endl;

	glm::mat3 mBeta = { a.x - E.x, a.x - c.x, C.x,
						a.y - E.y, a.y - c.y, C.y,
						a.z - E.z, a.z - c.z, C.z };
	mBeta = glm::transpose(mBeta);
	double Beta = glm::determinant( mBeta ) / A;
	if( Beta < 0 || Beta > 1 - Gamma ) return false;
    //std::cout << Beta << std::endl;

	glm::mat3 mt = {a.x - b.x, a.x - c.x, a.x - E.x,
				  	a.y - b.y, a.y - c.y, a.y - E.y,
				  	a.z - b.z, a.z - c.z, a.z - E.z };
    mt = glm::transpose(mt);
	double t = glm::determinant( mt ) / A;

	std::cout << t << std::endl;

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

    return true;

}

bool Mesh::intersection(glm::vec3 E, glm::vec3 C) {
	//std::cout << "Face size:" << m_faces.size() << std::endl;

	for( const auto f: m_faces ) {
        if ( tri_intersection( m_vertices[f.v1], m_vertices[f.v2], m_vertices[f.v3], E, C ) ) return true;
        //std::cout << "a" << std::endl;
	}
	return false;
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
