// Fall 2018

#include <glm/ext.hpp>

#include "A4.hpp"
#include "GeometryNode.hpp"

bool checkIntersect( const SceneNode &node, const glm::vec3 E, const glm::vec3 C ) {
	for( const SceneNode *n: node.children ) {
		// Recursive intersection check
		checkIntersect( *n, E, C );
		// Check intersection for geometry nodes only
		if( n->m_nodeType == NodeType::GeometryNode ) {
			const GeometryNode *gNode =  static_cast<const GeometryNode *>(n);
			if( gNode->m_primitive->intersection( E, C ) ) return true;
		}
	}
	//std::cout << "no" << std::endl;
	return false;
}

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
) {

  // Fill in raytracing code here...
  double aspect = (double)image.width() / image.height();
  const glm::vec3 &right = glm::cross( view, up ) / glm::length( glm::cross( view, up ) );

  // get virtual screen parameters
  double screenDist = -length(view);
  double vHeight = 2 * tan(fovy/2) * screenDist;
  double vWidth = vHeight * aspect;

  double pixelSize = vWidth / image.width();

  glm::vec3 topLeft = eye + view - vWidth/2 * right - vHeight/2 * up + pixelSize / 2 * right + pixelSize / 2 * up;

  std::cout << "Calling A4_Render(\n" <<
		  "\t" << *root <<
          "\t" << "Image(width:" << image.width() << ", height:" << image.height() << ")\n"
          "\t" << "eye:  " << glm::to_string(eye) << std::endl <<
		  "\t" << "view: " << glm::to_string(view) << std::endl <<
		  "\t" << "up:   " << glm::to_string(up) << std::endl <<
          "\t" << "right:" << glm::to_string(right) << std::endl <<
		  "\t" << "fovy: " << fovy << std::endl <<
          "\t" << "ambient: " << glm::to_string(ambient) << std::endl <<
		  "\t" << "lights{" << std::endl;

	for(const Light * light : lights) {
		std::cout << "\t\t" <<  *light << std::endl;
	}
	std::cout << "\t}" << std::endl;
	std:: cout <<")" << std::endl;

	size_t h = image.height();
	size_t w = image.width();

	for (uint y = 0; y < h; ++y) {
		for (uint x = 0; x < w; ++x) {

			//std::cout << topLeft.x + pixelSize * x << " - " << topLeft.y + pixelSize * y << std::endl;
			glm::vec3 P( topLeft.x + pixelSize * x, topLeft.y + pixelSize * y, topLeft.z );
			//std::cout << glm::to_string(P) << std::endl;
			glm::vec3 C = P - eye;
			//std::cout << to_string(C) << std::endl;
            if( checkIntersect( *root, eye, C ) ) {
            	image(x, y, 0) = 0;
				image(x, y, 1) = 0;
				image(x, y, 2) = 0;

				//std::cout << "1" << std::endl;
				continue;
            }
			
            //std::cout << "0" << std::endl;



			// Red: increasing from Top to Bottom  
			image(x, y, 0) = (double)y / h;
			// Green: increasing from Left to Right  
			image(x, y, 1) = (double)x / w;
			// Blue: in Lower-Left and Upper-Right corners
			image(x, y, 2) = ((y < h/2 && x < w/2)
						  || (y >= h/2 && x >= w/2)) ? 1.0 : 0.0;
		}
	}

}
