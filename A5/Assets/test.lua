-- A simple scene with some miscellaneous geometry.

mat1 = gr.material({0.7, 1.0, 0.7}, {0.5, 0.7, 0.5}, 25)
mat2 = gr.material({0.5, 0.5, 0.5}, {0.5, 0.7, 0.5}, 25)
mat3 = gr.material({1.0, 0.6, 0.1}, {0.5, 0.7, 0.5}, 25)
mat4 = gr.material({0.7, 0.6, 1.0}, {0.5, 0.4, 0.8}, 25)
gold = gr.material({0.9, 0.8, 0.4}, {0.8, 0.8, 0.4}, 25)
grass = gr.material({0.1, 0.7, 0.1}, {0.0, 0.0, 0.0}, 0)
blue = gr.material({0, 1, 1}, {0.5, 0.5, 0.5}, 25)

scene_root = gr.node('root')
--scene_root:rotate('X', 23)
scene_root:translate(0, 0, -5)
--[[
p1 = gr.sphere('p1')
scene_root:add_child(p1)
p1:set_material(blue)
p1:translate(0, 0, -10)]]

--[[s1 = gr.cube('s1')
scene_root:add_child(s1)
s1:set_material(mat1)
s1:scale(20, 20, 20)
s1:translate(-10, -10, -50)]]

--[[s6 = gr.nh_sphere('s6', {0, 0, -20}, 5)
scene_root:add_child(s6)
s6:set_material(mat1)
s6:set_transparency(0.75)
s6:set_refractiveness(1.33)

b1 = gr.cube('b1')
scene_root:add_child(b1)
b1:set_material(mat4)
b1:scale( 4, 20, 4)
b1:translate(-2, -10, -26)]]

-- A small stellated dodecahedron.

b_blade = gr.mesh( 'blade', 'buster-test.obj')
scene_root:add_child(b_blade)
b_blade:set_material(mat4)
b_blade:set_metallic(1)
b_blade:scale(0.2, 0.2, 0.2)
--b_blade:rotate('z', 90)
b_blade:translate(0, 0, -3)

--steldodec = gr.mesh( 'dodec', 'smstdodeca.obj' )
--steldodec:set_material(mat3)
--scene_root:add_child(steldodec)

--plane = gr.mesh( 'plane', 'plane.obj' )
--scene_root:add_child(plane)
--plane:set_material(mat4)
--plane:scale(30, 30, 30)
--plane:rotate('X', 23)
--plane:translate(6, -2, -15)


white_light = gr.light({0, 0, 400.0}, {0.9, 0.9, 0.9}, {1, 0, 0})
--orange_light = gr.light({400.0, 100.0, 150.0}, {0.7, 0.0, 0.7}, {1, 0, 0})

gr.render(scene_root, 'test.png', 256, 256,
	  {0, 0, 0}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.3, 0.3, 0.3}, {white_light, orange_light})
