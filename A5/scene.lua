-- test for hierarchical ray-tracers.
-- Thomas Pflaum 1996

gold = gr.material({0.9, 0.8, 0.4}, {0.8, 0.8, 0.4}, 25)
grass = gr.material({0.1, 0.7, 0.1}, {0.0, 0.0, 0.0}, 0)

red = gr.material({0.7, 0.1, 0.1}, {0.3, 0.3, 0.3}, 25)
green = gr.material({0.1, 0.7, 0.1}, {0.3, 0.3, 0.3}, 25)
blue = gr.material({0.1, 0.1, 0.7}, {0.3, 0.3, 0.3}, 25)
white = gr.material({0.8, 0.8, 0.8}, {0.3, 0.3, 0.3}, 25)

glass = gr.material({0.8, 0.8, 0.8}, {1.0, 1.0, 1.0}, 0)

scene = gr.node('scene')

-- floor
plane0 = gr.mesh( 'plane1', 'plane.obj' )
scene:add_child(plane0)
plane0:set_material(white)
--plane0:scale( 0.5, 0.5, 0.5)
plane0:translate(0, -1, 0)

-- back wall
plane1 = gr.mesh( 'plane1', 'plane.obj' )
scene:add_child(plane1)
plane1:set_material(green)
--plane1:scale( 0.5, 0.5, 0.5)
plane1:rotate('X', 90)
plane1:translate(0, 0, -1)

-- right wall
plane2 = gr.mesh( 'plane2', 'plane.obj' )
scene:add_child(plane2)
plane2:set_material(blue)
--plane2:scale( 0.5, 0.5, 0.5)
plane2:rotate('X', 90)
plane2:translate(0, 0, -1)
plane2:rotate('Y', -90)

-- left wall
plane3 = gr.mesh( 'plane3', 'plane.obj' )
scene:add_child(plane3)
plane3:set_material(red)
--plane3:scale( 0.5, 0.5, 0.5)
plane3:rotate('X', 90)
plane3:translate(0, 0, -1)
plane3:rotate('Y', 90)

-- ceiling
plane0 = gr.mesh( 'plane1', 'plane.obj' )
scene:add_child(plane0)
plane0:set_material(white)
--plane0:scale( 0.5, 0.5, 0.5)
plane0:translate(0, 1, 0)

-- sphere
s = gr.sphere('s')
scene:add_child(s)
s:scale(0.3, 0.3, 0.3)
s:translate(0.2, -0.5, 0.25)
s:set_material(glass)
s:set_transparency(0.5)
s:set_reflectiveness(0)
s:set_refractiveness(1)

-- The light
l2 = gr.light({0, 0.5, 0.99}, {1, 1, 1}, {1, 0, 0})

gr.render(scene, 'box.png', 256, 256,
	  {0, 0, 3}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, {l2})

