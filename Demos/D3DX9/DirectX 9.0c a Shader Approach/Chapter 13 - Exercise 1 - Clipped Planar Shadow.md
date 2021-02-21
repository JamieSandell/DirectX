[![](http://img.youtube.com/vi/2ZtrVLgsUlM/0.jpg)](http://www.youtube.com/watch?v=2ZtrVLgsUlM "Chapter 13 - Exercise 1 - Clipped Planar Shadow")

If you run the Shadow demo and move the teapot (using the "A" and "D" keys) such that the shadow goes off the floor, you will observe that the
shadow is still drawn. This can be fixed by employing the stencil technique used for the Mirror demo; that is, mark the stencil buffer pixels that
correspond with the floor and then only render the shadow pixels that coincide with the floor. Modify the Shadow demo by applying this described
fix.

Introduction to 3D Game Programming with DirectX 9.0c: A Shader Approach by Frank D. Luna.
