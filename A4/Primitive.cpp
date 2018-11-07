// Fall 2018

#include "Primitive.hpp"
#include "polyroots.hpp"
#include <iostream>

Primitive::~Primitive()
{
}

surface Primitive::intersection( ray r ) {}

Sphere::Sphere() {
    sphere = new NonhierSphere( glm::vec3(0.0f), 1.0f );
}

Sphere::~Sphere()
{
    delete sphere;
}

surface Sphere::intersection(ray r) {
    return sphere->intersection( r );
}

Cube::Cube() {
    box = new NonhierBox( glm::vec3(0.0f), 1.0f );
}

Cube::~Cube()
{
    delete box;
}

surface Cube::intersection(ray r) {
    return box->intersection( r );
}

NonhierSphere::~NonhierSphere()
{
}

surface NonhierSphere::intersection( ray r ) {

    double roots[2];

    size_t retval =  quadraticRoots(glm::dot(r.C, r.C),
                                    glm::dot( 2.0f * r.C, (r.E - m_pos) ),
                                    glm::dot( ( r.E - m_pos ), ( r.E - m_pos ) ) - m_radius * m_radius,
                                    roots );
    //std::cout << retval << std::endl;

    surface s;
    if ( retval == 0 ) {
        return s;
    } else if ( retval == 1 ) {
        s.t = roots[0];
        if ( s.t - r.tmin > Epsilon ) s.intersected = true;
    } else if ( retval == 2 ) {
        s.t = glm::min( roots[0], roots[1] );
        if ( s.t - r.tmin > Epsilon ) s.intersected = true;
    }

    // calc normal and intersection point
    s.intersect_pt = r.E + (float)s.t * r.C;
    s.n = glm::normalize( s.intersect_pt - m_pos );

    return s;
}

NonhierBox::~NonhierBox()
{
}

surface NonhierBox::faceIntersection( glm::vec3 minPts, glm::vec3 maxPts, ray r ) {
    double tmin = -INFINITY;
    double tmax = INFINITY;

    double minX = ( minPts.x - r.E.x ) / r.C.x;
    double maxX = ( maxPts.x - r.E.x ) / r.C.x;
    double minY = ( minPts.y - r.E.y ) / r.C.y;
    double maxY = ( maxPts.y - r.E.y ) / r.C.y;
    double minZ = ( minPts.z - r.E.z ) / r.C.z;
    double maxZ = ( maxPts.z - r.E.z ) / r.C.z;

    tmin = glm::max( tmin, glm::min( minX, maxX ) );
    tmin = glm::max( tmin, glm::min( minY, maxY ) );
    tmin = glm::max( tmin, glm::min( minZ, maxZ ) );
    tmax = glm::min( tmax, glm::max( minX, maxX ) );
    tmax = glm::min( tmax, glm::max( minY, maxY ) );
    tmax = glm::min( tmax, glm::max( minZ, maxZ ) );


    surface s;
    if ( tmax >= tmin && tmin - r.tmin > Epsilon ) {
        s.intersected = true;
        s.t = tmin;
        s.intersect_pt = r.E + (float)s.t * r.C;
    }
    return s;

}

surface NonhierBox::intersection( ray r ) {
    glm::vec3 min[6];
    glm::vec3 max[6];

    // front
    min[0] = { m_pos.x, m_pos.y, m_pos.z + m_size };
    max[0] = { m_pos.x + m_size, m_pos.y + m_size, m_pos.z + m_size };

    // left
    min[1] = { m_pos.x, m_pos.y, m_pos.z };
    max[1] = { m_pos.x, m_pos.y + m_size, m_pos.z + m_size };

    // right
    min[2] = { m_pos.x + m_size, m_pos.y, m_pos.z };
    max[2] = { m_pos.x + m_size, m_pos.y + m_size, m_pos.z + m_size };

    // top
    min[3] = { m_pos.x, m_pos.y + m_size, m_pos.z };
    max[3] = { m_pos.x + m_size, m_pos.y + m_size, m_pos.z + m_size };

    // bottom
    min[4] = { m_pos.x, m_pos.y, m_pos.z };
    max[4] = { m_pos.x + m_size, m_pos.y, m_pos.z + m_size };

    // back
    min[5] = { m_pos.x, m_pos.y, m_pos.z };
    max[5] = { m_pos.x + m_size, m_pos.y + m_size, m_pos.z };

    surface ret;
    surface s[6];
    int num = -1;
    for( int i = 0; i < 6; i++ ) {
        s[i] = faceIntersection( min[i], max[i], r );
        if ( !s[i].intersected ) {
            continue;
        }
        else if( !ret.intersected ) {
            ret = s[i];
            num = i;
        }
        else if( s[i].t < ret.t ) {
            ret = s[i];
            num = i;
        }
    }
    if ( num == 0 ) ret.n = {0, 0, 1};
    else if ( num == 1 ) ret.n = {-1, 0, 0};
    else if ( num == 2 ) ret.n = {1, 0, 0};
    else if ( num == 3 ) ret.n = {0, 1, 0};
    else if ( num == 4 ) ret.n = {0, -1, 0};
    else if ( num == 5 ) ret.n = {0, 0, -1};

    return ret;
}