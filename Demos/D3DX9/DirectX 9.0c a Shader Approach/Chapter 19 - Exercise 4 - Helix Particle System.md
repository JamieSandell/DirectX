[![](http://img.youtube.com/vi/SvEUvMKVpuc/0.jpg)](http://www.youtube.com/watch?v=SvEUvMKVpuc "Chapter 19 - Exercise 4 - Helix Particle System")

Many times a particle system is attached to an object. For example, we attached the gun system to the camera object in the Gun demo (i.e., the
camera position became the source of the gun bullet particles). As another example, you might attach a smoke particle system to the tail of a
rocket to implement a smoke trail. For this exercise, attach a particle system to a moving point as shown in Figure 19.13. In this figure, the
following parametric equations were used to move the point along a circular helix that is aligned with the z-axis as a function of time:
x = r cos(kt)
y = r sin(kt)
z = ct
The constant r is the radius of the helix, the constant k controls how fast the points rotate on the circle, the constant c controls how fast points
move down the z-axis, and t is time (i.e., the helix "grows" over time, or in other words, the particle system's source point "walks" along the helix
curve as a function of time). To implement this, you may wish to add a D3DXVECTOR3 pointer to the derived particle system class, and set it to
point to the moving point in the constructor. Thus, whenever the point moves, the particle system has a direct pointer to the updated point position
where new particles should be emitted.
