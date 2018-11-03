// Fall 2018

#pragma once

#include <glm/glm.hpp>
#include "polyroots.hpp"

class Primitive {
public:
  virtual ~Primitive();
  virtual bool intersection( glm::vec3 E, glm::vec3 C );
};

class Sphere : public Primitive {
public:
  virtual ~Sphere();
};

class Cube : public Primitive {
public:
  virtual ~Cube();
};

class NonhierSphere : public Primitive {
public:
  NonhierSphere(const glm::vec3& pos, double radius)
    : m_pos(pos), m_radius(radius)
  {
  }
  virtual ~NonhierSphere();
  virtual bool intersection( glm::vec3 E, glm::vec3 C );

private:
  glm::vec3 m_pos;
  double m_radius;
};

class NonhierBox : public Primitive {
public:
  NonhierBox(const glm::vec3& pos, double size)
    : m_pos(pos), m_size(size)
  {
  }
  
  virtual ~NonhierBox();
  virtual bool faceIntersection( glm::vec3 p1, glm::vec3 p2, glm::vec3 E, glm::vec3 C );
  virtual bool intersection( glm::vec3 E, glm::vec3 C );

private:
  glm::vec3 m_pos;
  double m_size;
};
