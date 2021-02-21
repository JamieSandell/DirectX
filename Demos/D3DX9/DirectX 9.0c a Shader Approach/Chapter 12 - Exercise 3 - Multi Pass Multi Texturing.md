[![](http://img.youtube.com/vi/uIwqCpoC478/0.jpg)](http://www.youtube.com/watch?v=uIwqCpoC478 "Chapter 12 - Exercise 3 - Multi Pass Multi Texturing")

Multi-texturing can be implemented via blending using a multi-pass technique. The idea is as follows: We draw the geometry once on the first pass
with a base texture; we then draw the geometry again with alpha blending enabled and a second texture, where the alpha channel and blend
operations specify how the secondary texture should be blended with the geometry drawn in the first pass; we next draw the geometry again with
alpha blending enabled and a third texture, where the alpha channel and blend operations specify how the third texture should be blended with the
previously drawn geometry. We can keep doing this repeatedly, each pass layering on more and more details. The benefit of this technique is that
you are not limited by any hardware restrictions that might limit how many textures you can sample simultaneously in a single pass. The downside
of this technique is that it requires you to render the same geometry repeatedly.
Reimplement the Multi-texturing demo from Chapter 11, but instead of sampling multiple textures simultaneously in one pass, use a multi-pass
technique as described above (i.e., you can only use one texture per pass).

Introduction to 3D Game Programming with DirectX 9.0c: A Shader Approach by Frank D. Luna.
