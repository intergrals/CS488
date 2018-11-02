-- puppet.lua
-- A simplified puppet without posable joints, but that
-- looks roughly humanoid.

rootnode = gr.node('root')
rootnode:rotate('y', -20.0)
rootnode:scale( 0.25, 0.25, 0.25 )
rootnode:translate(0.0, 0.0, -1.0)

red = gr.material({1.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
blue = gr.material({0.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, 10)
green = gr.material({0.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)
white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)

torso = gr.mesh('cube', 'torso')
rootnode:add_child(torso)
torso:set_material(white)
torso:scale(0.5,1.0,0.5);

leftHip = gr.mesh('sphere', 'leftHip')
torso:add_child(leftHip)
leftHip:scale(1/0.5,1.0,1/0.5);
leftHip:scale(0.21, 0.21, 0.21)
leftHip:translate(-0.38, -0.5, 0.0)
leftHip:set_material(blue)

leftHipJoint = gr.joint('leftHipJoint', { -45, 0, 0 }, {0, 0, 90})
leftHip:add_child(leftHipJoint)
--leftHipJoint:scale(1/0.5,1.0,1/0.5);
--leftHipJoint:scale(0.21, 0.21, 0.21)
--leftHipJoint:translate(-0.38, -0.5, 0.0)

leftLeg = gr.mesh('cube', 'leftLeg')
leftHipJoint:add_child(leftLeg)
leftLeg:scale(0.5,4,0.5)
leftLeg:translate(0,-2.8,0)
leftLeg:set_material(red)
return rootnode
