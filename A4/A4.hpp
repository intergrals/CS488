// Fall 2018

#pragma once

#include <glm/glm.hpp>

#include "SceneNode.hpp"
#include "Light.hpp"
#include "Image.hpp"
#include "PhongMaterial.hpp"

void A4_Render(
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


class surface {
public:
    //Primitive &shape;
    bool intersected = false;
    double t;
    glm::vec3 intersect_pt;
    glm::vec3 n;
    PhongMaterial *mat = nullptr;
};

class ray {
public:
    glm::vec3 E;            // Eye / starting point
    glm::vec3 C;            // Ray direction
    double tmin = 0;        // minimum t
    double tmax = INFINITY; // maximum t
};