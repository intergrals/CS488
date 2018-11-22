// Fall 2018

#include <glm/ext.hpp>

#include "A5.hpp"
#include "GeometryNode.hpp"

// Check every node for intersection.
surface checkIntersect( const SceneNode &node, ray r ) {
	surface ret;

    r.E = glm::vec3( glm::inverse( node.trans ) * glm::vec4( r.E, 1.0f ) );
    r.P = glm::vec3( glm::inverse( node.trans ) * glm::vec4( r.P, 1.0f ) );
    r.trans = r.trans * node.trans;

	for( SceneNode *n: node.children ) {
		// Recursive intersection check
        surface s;

        // Child intersection
		s = checkIntersect( *n, r );
        if( !s.intersected ) {
        } else if( !ret.intersected ) {
            ret = s;
        } else if( s.t < ret.t ) {
            ret = s;
        }

		// Check intersection for geometry nodes only
		if( n->m_nodeType == NodeType::GeometryNode ) {
			GeometryNode *gNode =  static_cast<GeometryNode *>(n);
			s = gNode->intersection( r );
			if( !s.intersected ) {
				continue;
			} else if( !ret.intersected ) {
				ret = s;
			} else if( s.t < ret.t ) {
				ret = s;
			}
		}
	}
	return ret;
}

/*//
void setTransMat( const SceneNode &node ) {
    for( SceneNode *n: node.children ) {
        //n->hiertrans = n->trans * node.hiertrans;
        n->hiertrans = node.hiertrans * n->trans;
        setTransMat( *n );
    }
}*/


glm::vec3 getColour( surface s, Light *l, SceneNode *root ) {
    // Check light ray intersection.
    ray lR;
    lR.E = s.intersect_pt;
    lR.origE = s.intersect_pt;
    //std::cout << glm::to_string( s.intersect_pt ) << std::endl;
    lR.P = l->position;
    //lR.C = l_dir;
    if( checkIntersect( *root, lR ).intersected ) return glm::vec3(0);

    glm::vec3 l_dir = glm::normalize( lR.P - lR.E );
    double light_dist = glm::length( lR.P - lR.E );

    glm::vec3 r = -l_dir + 2 * glm::dot( l_dir, s.n ) * s.n;
    glm::vec3 Lout =  s.mat->get_kd() * glm::dot( l_dir, s.n ) * l->colour
                      + s.mat->get_ks() * pow( glm::dot( r, s.v ), s.mat->get_shininess() ) * l->colour;

    Lout /= ( l->falloff[0] + l->falloff[1] * light_dist + l->falloff[2] * light_dist * light_dist );

    Lout[0] = glm::max( Lout[0], 0.0f );
    Lout[1] = glm::max( Lout[1], 0.0f );
    Lout[2] = glm::max( Lout[2], 0.0f );

    return Lout;
}

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
) {

  // Fill in raytracing code here...
  double aspect = (double)image.width() / image.height();
  glm::vec3 nView = glm::normalize( view );
  const glm::vec3 &right = glm::cross( view, up ) / glm::length( glm::cross( view, up ) );

  // get virtual screen parameters
  double screenDist = 1;
  double vHeight = 2 * tan(glm::radians(fovy)/2) * screenDist;
  double vWidth = vHeight * aspect;
  
  //std::cout << vHeight << std::endl;

  double pixelSize = vWidth / image.width();

  //glm::vec3 topLeft = eye + view - vWidth/2 * right + vHeight/2 * up + pixelSize / 2 * right + pixelSize / 2 * up;
  glm::vec3 topLeft = eye + nView - vWidth/2 * right + vHeight/2 * up;

  if( !super ) {
      topLeft += pixelSize / 2 * right + pixelSize / 2 * up;
  }

  std::cout << "Calling A5_Render(\n" <<
		  "\t" << *root <<
		  "\t" << "Supersampling: " << ( super? "ON" : "OFF" ) << std::endl <<
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

    glm::vec3 superPoints[w+1][h+1];
	if ( super ) {
        h++;
        w++;
	}

	// Set hierarchical transformation matrices for all nodes
	/*root->hiertrans = root->trans;
	setTransMat( *root );*/

	for (uint y = 0; y < h; ++y) {
	    //std::cout << "y = " << y << "\t/" << h << std::endl;
		for (uint x = 0; x < w; ++x) {
		    ray R;
			//std::cout << topLeft.x + pixelSize * x << " - " << topLeft.y + pixelSize * y << std::endl;
			glm::vec3 P( topLeft.x + pixelSize * x, topLeft.y - pixelSize * y, topLeft.z );
			R.origE = eye;
			R.E = eye;
			R.P = P;
			// Check if ray intersects object
			surface s = checkIntersect( *root, R );
			// if it intersects, calculate the colour through lighting
            if( s.intersected ) {
                if( super ) {
                    superPoints[x][y][0] = s.mat->get_kd().x * ambient.x;
                    superPoints[x][y][1] = s.mat->get_kd().y * ambient.y;
                    superPoints[x][y][2] = s.mat->get_kd().z * ambient.z;
                } else {
                    image(x, y, 0) = s.mat->get_kd().x * ambient.x;
                    image(x, y, 1) = s.mat->get_kd().y * ambient.y;
                    image(x, y, 2) = s.mat->get_kd().z * ambient.z;
                }

				for( auto *l : lights ) {

				    glm::vec3 Lout = getColour( s, l, root );


                    // Do reflection
                    glm::vec3 cReflect = glm::normalize( eye - s.intersect_pt );
                    cReflect = -cReflect + 2 * glm::dot( cReflect, s.n ) * s.n;

                    // reflected ray
                    ray rR;
                    rR.E = s.intersect_pt;
                    rR.origE = s.intersect_pt;
                    rR.P = s.intersect_pt + cReflect;

                    // check intersection of reflected intersection
                    surface rS = checkIntersect( *root, rR );
                    if( rS.intersected ) {
                        glm::vec3 Lout2 = getColour( rS, l, root );

                        Lout = ( glm::vec3(1) - s.mat->get_ks() ) * Lout + s.mat->get_ks() * Lout2;
                    }

				    //std::cout << glm::to_string(v) << std::endl;

				    if( super ) {
						superPoints[x][y][0] += Lout.x;
						superPoints[x][y][1] += Lout.y;
						superPoints[x][y][2] += Lout.z;
					} else {
						image( x, y, 0 ) += Lout.x;
						image( x, y, 1 ) += Lout.y;
						image( x, y, 2 ) += Lout.z;

						//std::cout << image( x, y, 0 ) << " " << image( x, y, 1 ) << " " << image( x, y, 2 ) << std::endl;
				    }
				}


				//std::cout << "1" << std::endl;
            } else {
				//std::cout << "0" << std::endl;

				/*// Red: increasing from Top to Bottom
                image(x, y, 0) = (double)y / h;
                // Green: increasing from Left to Right
                image(x, y, 1) = (double)x / w;
                // Blue: in Lower-Left and Upper-Right corners
                image(x, y, 2) = ((y < h/2 && x < w/2)
                              || (y >= h/2 && x >= w/2)) ? 1.0 : 0.0;*/
                if( super ) {
                    superPoints[x][y][0] = 0;
                    superPoints[x][y][1] = glm::max(1 - ((float)y/h*1.3), 0.2);
                    superPoints[x][y][2] = glm::max(1 - ((float)y/h*1.3), 0.2);

                    //std::cout << y << " " << h << " " << superPoints[x][y][2] << std::endl;

                    size_t invX = w - x;

                    if ( invX*invX + y*y < (w/5)*(w/5) ) {
                        superPoints[x][y][0] = 1;
                        superPoints[x][y][1] = 1;
                        superPoints[x][y][2] = 0;
                    }
                } else {
                    image(x, y, 0) = 0;
                    image(x, y, 1) = glm::max(1 - ((float)y/h*1.3), 0.2);
                    image(x, y, 2) = glm::max(1 - ((float)y/h*1.3), 0.2);

                    size_t invX = w - x;

                    if ( invX*invX + y*y < (w/5)*(w/5) ) {
                        image(x, y, 0) = 1;
                        image(x, y, 1) = 1;
                        image(x, y, 2) = 0;
                    }
                }
			}

			if ( super && x >= 1 && y >= 1 ) {
				for (uint i = 0; i < 3; i++) {
					image(x - 1, y - 1, i) = (superPoints[x - 1][y - 1][i]
											  + superPoints[x - 1][y][i]
											  + superPoints[x][y - 1][i]
											  + superPoints[x][y][i]) / 4;
					//std::cout << image( x-1, y-1, i ) << " ";
				}
				//std::cout << std::endl;
			}

		}
	}

	if( super ) {
		// set width and height back to correct values
		h--;
		w--;
	}

}
