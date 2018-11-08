-- my final scene

gold = gr.material({0.9, 0.8, 0.4}, {0.8, 0.8, 0.4}, 25)
grass = gr.material({0.1, 0.7, 0.1}, {0.0, 0.0, 0.0}, 0)
blue = gr.material({0, 1, 1}, {0.5, 0.5, 0.5}, 25)
silver = gr.material({0.5, 0.5, 0.5}, {0.5, 0.5, 0.5}, 25)
white = gr.material({1, 1, 1}, {0, 0, 0}, 25)
black = gr.material({0, 0, 0}, {0, 0, 0}, 25)
red = gr.material({1, 0, 0}, {0, 0, 0}, 25)

scene = gr.node('scene')
scene:rotate('X', 10)
scene:translate(0, -2, -15)


-- slime
s1 = gr.mesh( 'slime', 'simple_slime.obj' )
scene:add_child(s1)
s1:translate(-0.8, 0.5, 1.5)
s1:set_material(blue)
s1:scale(2, 2, 2)

e1 = gr.sphere('e1')
s1:add_child(e1)
e1:set_material(white)
e1:rotate('X', -45)
e1:scale(0.15, 0.15, 0.1)
e1:translate(-0.1, 0.25, 0.85)

p1 = gr.sphere('p1')
e1:add_child(p1)
p1:set_material(black)
p1:scale(0.35, 0.35, 0.35)
p1:translate(0.1, -0.5, 0.9)

e2 = gr.sphere('e2')
s1:add_child(e2)
e2:set_material(white)
e2:rotate('X', -45)
e2:scale(0.15, 0.15, 0.1)
e2:translate(0.4, 0.25, 0.8)

p2 = gr.sphere('p2')
e2:add_child(p2)
p2:set_material(black)
p2:scale(0.35, 0.35, 0.35)
p2:translate(0.1, -0.4, 0.8)

m1 = gr.mesh('mouth', 'mouth.obj')
s1:add_child(m1)
m1:rotate('Y', 11)
m1:scale(0.4, 0.4, 0.4)
m1:translate(0.17, -0.1, 0.85)
m1:set_material(red)

-- slime 2
s2 = gr.mesh( 'slime', 'simple_slime.obj' )
scene:add_child(s2)
s2:rotate('Y', -20)
s2:translate(2, 0.5, -1.5)
s2:set_material(silver)
s2:scale(2, 2, 2)

e3 = gr.sphere('e3')
s2:add_child(e3)
e3:set_material(white)
e3:rotate('X', -45)
e3:scale(0.15, 0.15, 0.1)
e3:translate(-0.1, 0.25, 0.85)

p3 = gr.sphere('p3')
e3:add_child(p3)
p3:set_material(black)
p3:scale(0.35, 0.35, 0.35)
p3:translate(0.1, -0.5, 0.9)

e4 = gr.sphere('e4')
s2:add_child(e4)
e4:set_material(white)
e4:rotate('X', -45)
e4:scale(0.15, 0.15, 0.1)
e4:translate(0.4, 0.25, 0.8)

p4 = gr.sphere('p4')
e4:add_child(p4)
p4:set_material(black)
p4:scale(0.35, 0.35, 0.35)
p4:translate(0.1, -0.4, 0.8)

m2 = gr.mesh('mouth', 'mouth.obj')
s2:add_child(m2)
m2:rotate('Y', 11)
m2:scale(0.4, 0.4, 0.4)
m2:translate(0.17, -0.1, 0.85)
m2:set_material(red)

-- the floor
plane = gr.mesh( 'plane', 'plane.obj' )
scene:add_child(plane)
plane:set_material(grass)
plane:scale(30, 30, 30)


-- The lights
-- light from front
l1 = gr.light({200,200,400}, {0.8, 0.8, 0.8}, {1, 0, 0})
-- weak sunlight
l2 = gr.light({200, 200, -300}, {0.3, 0.3, 0.01}, {1, 0, 0})

gr.render(scene, 'sample.png', 256*2, 256*2, 
	  {0, 0, 0,}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, {l1, l2})
