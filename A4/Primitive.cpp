// Fall 2018

#include "Primitive.hpp"
#include <iostream>

Primitive::~Primitive()
{
}

bool Primitive::intersection(glm::vec3 E, glm::vec3 C) {}

Sphere::~Sphere()
{
}

Cube::~Cube()
{
}

NonhierSphere::~NonhierSphere()
{
}

bool NonhierSphere::intersection( glm::vec3 E, glm::vec3 C ) {
    double roots[2];

    size_t retval =  quadraticRoots(glm::dot(C, C),
                                    glm::dot( 2.0f * C, (E - m_pos) ),
                                    glm::dot( ( E - m_pos ), ( E - m_pos ) ) - m_radius * m_radius,
                                    roots );
    //std::cout << retval << std::endl;
    return retval != 0;
}

NonhierBox::~NonhierBox()
{
}

bool NonhierBox::faceIntersection( glm::vec3 minPts, glm::vec3 maxPts, glm::vec3 E, glm::vec3 C ) {
    double tmin = -INFINITY;
    double tmax = INFINITY;

    double minX = ( minPts.x - E.x ) / C.x;
    double maxX = ( maxPts.x - E.x ) / C.x;
    double minY = ( minPts.y - E.y ) / C.y;
    double maxY = ( maxPts.y - E.y ) / C.y;
    double minZ = ( minPts.z - E.z ) / C.z;
    double maxZ = ( maxPts.z - E.z ) / C.z;

    tmin = glm::max( tmin, glm::min( minX, maxX ) );
    tmin = glm::max( tmin, glm::min( minY, maxY ) );
    tmin = glm::max( tmin, glm::min( minZ, maxZ ) );
    tmax = glm::min( tmax, glm::max( minX, maxX ) );
    tmax = glm::min( tmax, glm::max( minY, maxY ) );
    tmax = glm::min( tmax, glm::max( minZ, maxZ ) );

    return tmax >= tmin;
}

bool NonhierBox::intersection(glm::vec3 E, glm::vec3 C) {
    // front
    glm::vec3 min1( m_pos.x - m_size/2, m_pos.y - m_size/2, m_pos.z + m_size/2 );
    glm::vec3 max1( m_pos.x + m_size/2, m_pos.y + m_size/2, m_pos.z + m_size/2 );

    // left
    glm::vec3 min2( m_pos.x - m_size/2, m_pos.y - m_size/2, m_pos.z - m_size/2 );
    glm::vec3 max2( m_pos.x - m_size/2, m_pos.y + m_size/2, m_pos.z + m_size/2 );

    // right
    glm::vec3 min3( m_pos.x + m_size/2, m_pos.y - m_size/2, m_pos.z - m_size/2 );
    glm::vec3 max3( m_pos.x + m_size/2, m_pos.y + m_size/2, m_pos.z + m_size/2 );

    // top
    glm::vec3 min4( m_pos.x - m_size/2, m_pos.y + m_size/2, m_pos.z - m_size/2 );
    glm::vec3 max4( m_pos.x + m_size/2, m_pos.y + m_size/2, m_pos.z + m_size/2 );

    // bottom
    glm::vec3 min5( m_pos.x - m_size/2, m_pos.y - m_size/2, m_pos.z - m_size/2 );
    glm::vec3 max5( m_pos.x + m_size/2, m_pos.y - m_size/2, m_pos.z + m_size/2 );

    // back
    glm::vec3 min6( m_pos.x - m_size/2, m_pos.y - m_size/2, m_pos.z - m_size/2 );
    glm::vec3 max6( m_pos.x + m_size/2, m_pos.y + m_size/2, m_pos.z - m_size/2 );

    return  faceIntersection( min1, max1, E, C ) ||
            faceIntersection( min2, max2, E, C ) ||
            faceIntersection( min3, max3, E, C ) ||
            faceIntersection( min4, max4, E, C ) ||
            faceIntersection( min5, max5, E, C ) ||
            faceIntersection( min6, max6, E, C );
}