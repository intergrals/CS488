// Fall 2018

#pragma once

#include <glm/glm.hpp>

#include "SceneNode.hpp"
#include "Light.hpp"
#include "Image.hpp"
#include "PhongMaterial.hpp"

void A5_Render(
		// What to render
		SceneNode * root,

		// Image to write to, set to a given width and height
		Image & image,

		// Viewing parameters
		const glm::vec3 & eye,
		const glm::vec3 & view,
		const glm::vec3 & up,
		double fovy,

		// Lighting parameters
		const glm::vec3 & ambient,
		const std::list<Light *> & lights
);

static const double Epsilon = 0.01;
static const bool super = false;
static const bool bound = false;
static const bool reflect = true;
static const bool mthread = true;
static const bool showAdaptive = false;
static const uint Adaptive = 16;
static const uint SoftShad = 16;
static const uint GlossAmt = 64;


class surface {
public:
    //Primitive &shape;
    bool intersected = false;
    double t;
    glm::vec3 intersect_pt;
    glm::vec3 n;
    glm::vec3 v;
    PhongMaterial *mat = nullptr;
    //glm::mat4 trans;
};

class ray {
public:
    glm::vec3 origE;                        // Original eye position
    glm::vec3 E;                            // Eye / starting point
    glm::vec3 P;			                // Screen point
    glm::vec3 C;                            // Ray direction
    double tmin = 0;                        // minimum t
    double tmax = INFINITY;                 // maximum t
    glm::mat4 trans = glm::mat4(1.0f);      // transformations applied to ray
};
