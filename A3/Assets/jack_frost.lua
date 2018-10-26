-- jack_frost.lua
-- A model resembling the video game character "Jack Frost"
-- looks roughly humanoid.

-- Color material
red = gr.material({1.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
blue = gr.material({0.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, 10)
green = gr.material({0.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)
white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)
black = gr.material({0.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
yellow = gr.material({1.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)

--* Root *--
rootnode = gr.node('root')
--rootnode:rotate('y', -20.0)
rootnode:scale( 0.25, 0.25, 0.25 )
rootnode:translate(0.0, 0.0, -1.0)

--[ Torso ]--
torso = gr.mesh('sphere', 'torso')
rootnode:add_child(torso)
torso:set_material(white)
torso:scale(0.5, 0.5, 0.5)

-------------------------------------[HEAD]---------------------------------------

--< Neck Joint >--
neckJoint = gr.joint('neckJoint', {-45, 0, 45}, {-45, 0, 0})
torso:add_child(neckJoint)
neckJoint:scale(0.85, 0.85, 0.85)
neckJoint:translate(0.0, 1.6, 0.0)

--[ Head ]--
head = gr.mesh('sphere', 'head')
neckJoint:add_child(head)
head:set_material(white)

--( Eyes )--
leftEye = gr.mesh('sphere', 'leftEye')
head:add_child(leftEye)
leftEye:set_material(black)
leftEye:scale(0.1, 0.15, 0.1)
leftEye:translate(-0.25, 0.15, 0.9)

rightEye = gr.mesh('sphere', 'leftEye')
head:add_child(rightEye)
rightEye:set_material(black)
rightEye:scale(0.1, 0.15, 0.1)
rightEye:translate(0.25, 0.15, 0.9)

--( Mouth )--
mouth = gr.mesh('mouth', 'mouth')
head:add_child(mouth)
mouth:set_material(black)
mouth:scale(0.4, 0.5, 0.4)
mouth:rotate('x', 30)
mouth:translate(0, -0.4, 0.8)

--[ Hat ]--
hat = gr.mesh('jack_frost_hat', 'hat')
head:add_child(hat)
hat:set_material(blue)
hat:scale(0.88, 0.88, 0.88)

--( Emblem )--
emblem = gr.mesh('sphere', 'emblem')
hat:add_child(emblem)
emblem:set_material(yellow)
emblem:scale(0.2, 0.1, 0.2)
emblem:rotate('x', 35)
emblem:translate(0, 1, 0.75)

--( Hat Extensions )--

--[ Left Side ]--
leftHatBase = gr.mesh('hat_cone', 'leftHatBase')
hat:add_child(leftHatBase)
leftHatBase:set_material(blue)
leftHatBase:scale(0.3, 0.6, 0.3)
leftHatBase:rotate('x', -15)
leftHatBase:rotate('z', 40)
leftHatBase:translate(-0.9, 1.2, -0.1)

--< Left Hat Joint 1 >--
hatJoint1 = gr.joint('hatJoint1', {0, 0, 0}, {-110, -110, 0})
leftHatBase:add_child(hatJoint1)
hatJoint1:scale(1, 0.5, 1)
hatJoint1:translate(-0.8, 1, 0.1)

leftExt1 = gr.mesh('hat_extend', 'leftExt1')
hatJoint1:add_child(leftExt1)
leftExt1:set_material(blue)
leftExt1:rotate('z', 45)
leftExt1:translate(0, 1, 0)

--< Left Hat Joint 2 >--
hatJoint2 = gr.joint('hatJoint2', {0, 0, 0}, {-90, 0, 0})
leftExt1:add_child(hatJoint2)
hatJoint2:translate( 0.1, 1, 0.1)

leftExt2 = gr.mesh('hat_extend', 'leftExt2')
hatJoint2:add_child(leftExt2)
leftExt2:set_material(blue)
leftExt2:scale(0.7, 0.7, 0.7)
leftExt2:rotate('x', 90)
leftExt2:translate(-0.05, 0, 0.8)

--< Left Hat Joint 3 >--
hatJoint3 = gr.joint('hatJoint3', {0, 0, 0}, {0, 0, 90})
leftExt2:add_child(hatJoint3)
hatJoint3:scale(2, 2, 2)
hatJoint3:rotate('x', -90)
hatJoint3:rotate('z', -45)
hatJoint3:translate( 0.1, 1, 0.1)

leftTip = gr.mesh('hat_tip', 'leftTip')
hatJoint3:add_child(leftTip)
leftTip:set_material(blue)
leftTip:scale(0.5, 0.7, 0.5)
leftTip:rotate('y', 90)
leftTip:translate(-0.1, 0.4, 0)



--[ Right side ]--
rightHatBase = gr.mesh('hat_cone', 'rightHatBase')
hat:add_child(rightHatBase)
rightHatBase:set_material(blue)
rightHatBase:scale(0.3, 0.6, 0.3)
rightHatBase:rotate('x', -15)
rightHatBase:rotate('z', -40)
rightHatBase:translate(0.9, 1.2, -0.1)

--< Right Hat Joint 1 >--
hatJoint4 = gr.joint('hatJoint4', {0, 0, 0}, {-110, -110, 0})
rightHatBase:add_child(hatJoint4)
hatJoint4:scale(1, 0.5, 1)
hatJoint4:translate(0.8, 1, 0.1)

rightExt1 = gr.mesh('hat_extend', 'rightExt1')
hatJoint4:add_child(rightExt1)
rightExt1:set_material(blue)
rightExt1:rotate('z', -45)
rightExt1:translate(0, 1, 0)

--< Right Hat Joint 2 >--
hatJoint5 = gr.joint('hatJoint5', {0, 0, 0}, {-90, 0, 0})
rightExt1:add_child(hatJoint5)
hatJoint5:translate( 0.1, 1, 0.1)

rightExt2 = gr.mesh('hat_extend', 'rightExt2')
hatJoint5:add_child(rightExt2)
rightExt2:set_material(blue)
rightExt2:scale(0.7, 0.7, 0.7)
rightExt2:rotate('x', 90)
rightExt2:translate(0.05, 0, 0.8)

--< Right Hat Joint 3 >--
hatJoint6 = gr.joint('hatJoint6', {0, 0, 0}, {0, 0, 90})
rightExt2:add_child(hatJoint6)
hatJoint6:scale(2, 2, 2)
hatJoint6:rotate('x', -90)
hatJoint6:rotate('z', -45)
hatJoint6:translate( -0.1, 1, 0.1)

rightTip = gr.mesh('hat_tip', 'rightTip')
hatJoint6:add_child(rightTip)
rightTip:set_material(blue)
rightTip:scale(0.5, 0.7, 0.5)
rightTip:rotate('y', 90)
rightTip:translate(0.1, 0.4, 0)

-------------------------------------[ARMS]---------------------------------------

--[ left arm ]--
--< left shoulder >--
leftShoulder = gr.joint("leftShoulder", {0, 0, 0}, {-90, 0, 0})
torso:add_child(leftShoulder)
leftShoulder:translate(-0.7, 0.7, 0)

--( Upper Arm )--
leftUpper = gr.mesh('tube', 'leftUpper')
leftShoulder:add_child(leftUpper)
leftUpper:set_material(white)
leftUpper:scale(0.3, 0.3, 0.3)
leftUpper:rotate('z', -55)
leftUpper:translate(-0.25, -0.3, 0)

--< left elbow >--
leftElbow = gr.joint("leftElbow", {0, 0, 0}, {-90, 0, 0})
leftUpper:add_child(leftElbow)
leftElbow:rotate('z', 55)
leftElbow:translate( 0, -1, 0)

--( Lower Arm )--
leftLower = gr.mesh('growing_tube', 'leftLower')
leftElbow:add_child(leftLower)
leftLower:set_material(white)
leftLower:scale(0.6, 0.45, 0.6)
leftLower:translate(-0.25, -1.1, 0)

--( hand )--
leftHand = gr.mesh('hand2', 'leftHand')
leftLower:add_child(leftHand)
leftHand:set_material(white)
leftHand:scale(1.2, 1.4, 1.3)
leftHand:rotate('z', 150)
leftHand:rotate('y', -90)
leftHand:rotate('x', -25)
leftHand:translate(0, -4, 0)

--[ right arm ]--
--< right shoulder >--
rightShoulder = gr.joint("rightShoulder", {0, 0, 0}, {-90, 0, 0})
torso:add_child(rightShoulder)
rightShoulder:translate(0.7, 0.7, 0)

--( Upper Arm )--
rightUpper = gr.mesh('tube', 'rightUpper')
rightShoulder:add_child(rightUpper)
rightUpper:set_material(white)
rightUpper:scale(0.3, 0.3, 0.3)
rightUpper:rotate('z', 55)
rightUpper:translate(0.2, -0.3, 0)

--< right elbow >--
rightElbow = gr.joint("rightElbow", {0, 0, 0}, {-90, 0, 0})
rightUpper:add_child(rightElbow)
rightElbow:rotate('z', -55)
rightElbow:translate( 0, -1, 0)

--( Lower Arm )--
rightLower = gr.mesh('growing_tube', 'rightLower')
rightElbow:add_child(rightLower)
rightLower:set_material(white)
rightLower:scale(0.6, 0.45, 0.6)
rightLower:translate(0.25, -1.1, 0)

--( hand )--
rightHand = gr.mesh('hand', 'rightHand')
rightLower:add_child(rightHand)
rightHand:set_material(white)
rightHand:scale(1.2, 1.4, 1.3)
rightHand:rotate('z', 150)
rightHand:rotate('y', -90)
rightHand:rotate('x', -25)
rightHand:translate(0, -4, 0)

-------------------------------------[LEGS]---------------------------------------

--[ left leg ]--

--< left hip >--
leftHip = gr.joint("leftHip", {0, 0, 0}, {-90, 0, 0})
torso:add_child(leftHip)
leftHip:translate(-0.15, -0.65, 0)

--( left thigh )--
leftThigh = gr.mesh('tube', 'leftThigh')
leftHip:add_child(leftThigh)
leftThigh:set_material(white)
leftThigh:scale(0.5, 0.2, 0.5)
leftThigh:translate(-0.2, -0.4, 0)

--< left knee >--
leftKnee = gr.joint("leftKnee", {0, 0, 0}, {0, 0, 90})
leftThigh:add_child(leftKnee)
leftKnee:scale(1/0.5, 1/0.25, 1/0.5)
leftKnee:translate( 0, -0.9, 0)

--( left leg )--
leftLeg = gr.mesh('shrinking_tube', 'leftLeg')
leftKnee:add_child(leftLeg)
leftLeg:set_material(white)
leftLeg:scale(0.24, 0.15, 0.24)
leftLeg:translate(0, -0.3, 0)

--( left shoe )--
leftShoe = gr.mesh('shoe', 'leftShoe')
leftLeg:add_child(leftShoe)
leftShoe:set_material(blue)
leftShoe:scale(1/0.24, 1/0.15, 1/0.24)
leftShoe:scale(0.2, 0.2, 0.2)
leftShoe:translate(-1.2, -3, 0)
leftShoe:rotate('y', 90)


--[ right leg ]--

--< right hip >--
rightHip = gr.joint("leftHip", {0, 0, 0}, {-90, 0, 0})
torso:add_child(rightHip)
rightHip:translate(0.15, -0.65, 0)

--( right thigh )--
rightThigh = gr.mesh('tube', 'rightThigh')
rightHip:add_child(rightThigh)
rightThigh:set_material(white)
rightThigh:scale(0.5, 0.2, 0.5)
rightThigh:translate(0.2, -0.4, 0)

--< right knee >--
rightKnee = gr.joint("rightKnee", {0, 0, 0}, {0, 0, 90})
rightThigh:add_child(rightKnee)
rightKnee:scale(1/0.5, 1/0.25, 1/0.5)
rightKnee:translate( 0, -0.9, 0)

--( right leg )--
rightLeg = gr.mesh('shrinking_tube', 'rightLeg')
rightKnee:add_child(rightLeg)
rightLeg:set_material(white)
rightLeg:scale(0.24, 0.15, 0.24)
rightLeg:translate(0, -0.3, 0)

--( right shoe )--
rightShoe = gr.mesh('shoe', 'rightShoe')
rightLeg:add_child(rightShoe)
rightShoe:set_material(blue)
rightShoe:scale(1/0.24, 1/0.15, 1/0.24)
rightShoe:scale(0.2, 0.2, 0.2)
rightShoe:translate(-1.2, -3, 0)
rightShoe:rotate('y', 90)

------------------------------------[COLLAR]--------------------------------------

--( Front )--
fCollar = gr.mesh('triangle', 'fCollar')
torso:add_child(fCollar)
fCollar:set_material(blue)
fCollar:scale(0.25, 0.4, 0.25)
fCollar:rotate('x', 125)
fCollar:translate(0, 0.75, 0.65)
--( Ball )--
fBall = gr.mesh('sphere', 'fBall')
fCollar:add_child(fBall)
fBall:set_material(yellow)
fBall:scale(0.2, 0.2, 0.2)
fBall:translate(0, 1, 0)

--( Front-Right )--
rCollar = gr.mesh('triangle', 'rCollar')
torso:add_child(rCollar)
rCollar:set_material(blue)
rCollar:scale(0.25, 0.4, 0.25)
rCollar:rotate('x', 125)
rCollar:translate(0, 0.75, 0.65)
rCollar:rotate('y', 45)
--( Ball )--
rBall = gr.mesh('sphere', 'rBall')
rCollar:add_child(rBall)
rBall:set_material(yellow)
rBall:scale(0.2, 0.2, 0.2)
rBall:translate(0, 1, 0)

--( Right )--
rCollar = gr.mesh('triangle', 'rCollar')
torso:add_child(rCollar)
rCollar:set_material(blue)
rCollar:scale(0.25, 0.4, 0.25)
rCollar:rotate('x', 125)
rCollar:translate(0, 0.75, 0.65)
rCollar:rotate('y', 90)
--( Ball )--
rBall = gr.mesh('sphere', 'rBall')
rCollar:add_child(rBall)
rBall:set_material(yellow)
rBall:scale(0.2, 0.2, 0.2)
rBall:translate(0, 1, 0)

--( Back-Right )--
rCollar = gr.mesh('triangle', 'rCollar')
torso:add_child(rCollar)
rCollar:set_material(blue)
rCollar:scale(0.25, 0.4, 0.25)
rCollar:rotate('x', 125)
rCollar:translate(0, 0.75, 0.65)
rCollar:rotate('y', 135)
--( Ball )--
rBall = gr.mesh('sphere', 'rBall')
rCollar:add_child(rBall)
rBall:set_material(yellow)
rBall:scale(0.2, 0.2, 0.2)
rBall:translate(0, 1, 0)

--( Back )--
rCollar = gr.mesh('triangle', 'rCollar')
torso:add_child(rCollar)
rCollar:set_material(blue)
rCollar:scale(0.25, 0.4, 0.25)
rCollar:rotate('x', 125)
rCollar:translate(0, 0.75, 0.65)
rCollar:rotate('y', 180)
--( Ball )--
rBall = gr.mesh('sphere', 'rBall')
rCollar:add_child(rBall)
rBall:set_material(yellow)
rBall:scale(0.2, 0.2, 0.2)
rBall:translate(0, 1, 0)

--( Front-Left )--
rCollar = gr.mesh('triangle', 'rCollar')
torso:add_child(rCollar)
rCollar:set_material(blue)
rCollar:scale(0.25, 0.4, 0.25)
rCollar:rotate('x', 125)
rCollar:translate(0, 0.75, 0.65)
rCollar:rotate('y', -45)
--( Ball )--
rBall = gr.mesh('sphere', 'rBall')
rCollar:add_child(rBall)
rBall:set_material(yellow)
rBall:scale(0.2, 0.2, 0.2)
rBall:translate(0, 1, 0)

--( Left )--
rCollar = gr.mesh('triangle', 'rCollar')
torso:add_child(rCollar)
rCollar:set_material(blue)
rCollar:scale(0.25, 0.4, 0.35)
rCollar:rotate('x', 120)
rCollar:translate(0, 0.75, 0.65)
rCollar:rotate('y', -90)
--( Ball )--
rBall = gr.mesh('sphere', 'rBall')
rCollar:add_child(rBall)
rBall:set_material(yellow)
rBall:scale(0.2, 0.2, 0.2)
rBall:translate(0, 1, 0)

--( Back-Left )--
rCollar = gr.mesh('triangle', 'rCollar')
torso:add_child(rCollar)
rCollar:set_material(blue)
rCollar:scale(0.25, 0.4, 0.25)
rCollar:rotate('x', 125)
rCollar:translate(0, 0.75, 0.65)
rCollar:rotate('y', -135)
--( Ball )--
rBall = gr.mesh('sphere', 'rBall')
rCollar:add_child(rBall)
rBall:set_material(yellow)
rBall:scale(0.2, 0.2, 0.2)
rBall:translate(0, 1, 0)

-------------------------------------[TAIL]---------------------------------------
tail = gr.mesh('hat_tip', 'tail')
torso:add_child(tail)
tail:set_material(white)
tail:scale(0.5, 0.3, 0.5)
tail:rotate('x', 240)
tail:translate(0, -0.7, -0.8)





return rootnode;

