-- test for hierarchical ray-tracers.
-- Thomas Pflaum 1996

gold = gr.material({0.9, 0.8, 0.4}, {0.8, 0.8, 0.4}, 25)
grass = gr.material({0.1, 0.7, 0.1}, {0.0, 0.0, 0.0}, 0)

red = gr.material({0.7, 0.1, 0.1}, {0.3, 0.3, 0.3}, 25)
blue = gr.material({0.1, 0.1, 0.7}, {0.3, 0.3, 0.3}, 25)
white = gr.material({0.8, 0.8, 0.8}, {0.3, 0.3, 0.3}, 25)
yellow = gr.material({1.0, 1.0, 0.0}, {0.3, 0.3, 0.3}, 25)
green = gr.material({0.1, 0.7, 0.1}, {0.3, 0.3, 0.3}, 25)
orange = gr.material({1.0, 0.5, 0}, {0.3, 0.3, 0.3}, 25)
gray = gr.material({0.4, 0.4, 0.4}, {0.3, 0.3, 0.3}, 25)
black = gr.material({0.0, 0.0, 0.0}, {0.3, 0.3, 0.3}, 25)

glass = gr.material({0.8, 0.8, 0.8}, {1.0, 1.0, 1.0}, 0)

scene = gr.node('scene')

-- floor
plane0 = gr.mesh( 'plane4', 'plane.obj' )
scene:add_child(plane0)
plane0:set_material(white)
--plane0:scale( 0.5, 0.5, 0.5)
plane0:translate(0, -1, 0)

-- back wall
plane1 = gr.mesh( 'plane1', 'plane.obj' )
scene:add_child(plane1)
plane1:set_material(gray)
--plane1:scale( 0.5, 0.5, 0.5)
plane1:rotate('X', 90)
plane1:translate(0, 0, -1)

-- right wall
plane2 = gr.mesh( 'plane2', 'plane.obj' )
scene:add_child(plane2)
plane2:set_material(gray)
plane2:rotate('X', 90)
plane2:translate(0, 0, -1)
plane2:rotate('Y', -90)

-- left wall
plane3 = gr.mesh( 'plane3', 'plane.obj' )
scene:add_child(plane3)
plane3:set_material(gray)
--plane3:scale( 0.5, 0.5, 0.5)
plane3:rotate('X', 90)
plane3:translate(0, 0, -1)
plane3:rotate('Y', 90)

-- ceiling
plane4 = gr.mesh( 'plane4', 'plane.obj' )
scene:add_child(plane4)
plane4:set_material(white)
plane4:scale( 1.1, 1.1, 1)
plane4:translate(0, 1, 0)

--[[-- cube
c = gr.cube('c')
scene:add_child(c)
c:scale(0.5, 0.5, 0.5)
c:translate(0.4, -1, -0.6)
c:set_material(gray)
c:set_metallic(1);
c:set_transparency(0)
c:set_reflectiveness(0.05)
c:set_refractiveness(1)]]

--[[-- cube2
c2 = gr.cube('c2')
scene:add_child(c2)
c2:scale(0.5, 0.5, 0.5)
c2:translate(-0.25, -1, -0.4)
c2:set_material(gray)
c2:set_metallic(1);
c2:set_transparency(0)
c2:set_reflectiveness(0.05)
c2:set_refractiveness(1)]]

--[[-- cube3
c3 = gr.cube('c3')
scene:add_child(c3)
c3:scale(0.5, 0.5, 0.5)
c3:translate(-0.9, -1, -0.6)
c3:set_material(gray)
c3:set_metallic(1);
c3:set_transparency(0)
c3:set_reflectiveness(0.05)
c3:set_refractiveness(1)]]


-- Buster Sword
busterSword = gr.node('buster')
scene:add_child(busterSword)
--busterSword:rotate('X', 20)
busterSword:rotate('Z', -160)
busterSword:rotate('Y', 60)
busterSword:scale(0.2, 0.2, 0.2)
busterSword:translate(-0.7, -0.5, 1.85)

b_deco = gr.mesh( 'bdecoration', 'buster-handle-decoration.obj' )
busterSword:add_child(b_deco)
b_deco:set_material(black)
b_deco:set_metallic(1)
b_deco:set_reflectiveness(0.05)

b_handle = gr.mesh( 'bhandle', 'buster-handle.obj' )
busterSword:add_child(b_handle)
b_handle:set_material(gray)
b_handle:set_metallic(1)
b_handle:translate(-0.8, 0, 0)

b_hilt = gr.mesh( 'bhilt', 'buster-hilt.obj')
busterSword:add_child(b_hilt)
b_hilt:set_material(black)
b_hilt:set_metallic(1)
b_hilt:rotate('X', 90)
b_hilt:translate(-1.6, 0, 0)

--[[
b_blade = gr.mesh( 'blade', 'buster.obj')
busterSword:add_child(b_blade)
b_blade:set_material(gray)
b_blade:set_metallic(1)
--b_blade:set_reflectiveness(0.2)
b_blade:rotate('X', 90)
b_blade:translate(-4.25, 0, 0)
]]
b_blade = gr.mesh( 'blade', 'buster-hilt.obj')
busterSword:add_child(b_blade)
b_blade:set_material(gray)
b_blade:set_metallic(1)
b_blade:set_reflectiveness(0.2)
b_blade:scale(13, 0.8, 0.8)
b_blade:rotate('X', 90)
b_blade:translate(-4.25, 0, 0)


-- Keyblade
keyblade = gr.node('keyblade')
scene:add_child(keyblade)
keyblade:scale(0.15, 0.15, 0.15)
keyblade:rotate('X', 90)
keyblade:rotate('Z', -25)
keyblade:rotate('X', -25)
--keyblade:rotate('Y', 5)
keyblade:translate( 0.75, 0, -0.8)

k_handle = gr.mesh( 'khandle', 'handle.obj' )
keyblade:add_child(k_handle)
k_handle:set_material(yellow)

k_between = gr.mesh( 'kbetween', 'pole-between.obj' )
keyblade:add_child(k_between)
k_between:set_material(black)
k_between:set_reflectiveness(0.2)
k_between:set_metallic(1)

k_top = gr.mesh('ktop', 'pole-top.obj')
keyblade:add_child(k_top)
k_top:set_material(blue)
k_top:translate(0, 0, 1.5)
k_top:set_reflectiveness(0.2)
k_top:set_metallic(1)

k_pole = gr.mesh('kpole', 'pole.obj')
keyblade:add_child(k_pole)
k_pole:set_material(gray)
k_pole:translate(0, 0, 1.7)
k_pole:set_reflectiveness(0.2)
k_pole:set_metallic(1)

k_teeth = gr.mesh('kteeth', 'teeth.obj')
keyblade:add_child(k_teeth)
k_teeth:set_material(gray)
k_teeth:translate(-0.7, 0, 6.8)
k_teeth:set_reflectiveness(0.2)
k_teeth:set_metallic(1)


 -- sphere
s = gr.sphere('s')
scene:add_child(s)
s:set_material(orange)
s:scale(0.2, 0.2, 0.2)
s:translate(-0.2, -0.8, 0.3)
s:set_reflectiveness(0.1)
s:set_refractiveness(1.1)
s:set_transparency(0.5)

s2 = gr.sphere('s2')
scene:add_child(s2)
s2:set_material(orange)
s2:scale(0.1, 0.1, 0.1)
s2:translate(0.3, -1, 0.5)
s2:set_reflectiveness(0.1)
s2:set_refractiveness(1.1)
s2:set_transparency(0.2)

s3 = gr.sphere('s3')
scene:add_child(s3)
s3:set_material(orange)
s3:scale(0.15, 0.15, 0.15)
s3:translate(1, -0.85, 0.7)
s3:set_reflectiveness(0.1)
s3:set_refractiveness(1.1)
s3:set_transparency(0.2)

-- The light
l2 = gr.light({0, 0.5, 0.99}, {1, 1, 1}, {1, 0, 0})

gr.render(scene, 'scene.png', 256*3, 256*3,
	  {0, 0, 3}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, {l2})

