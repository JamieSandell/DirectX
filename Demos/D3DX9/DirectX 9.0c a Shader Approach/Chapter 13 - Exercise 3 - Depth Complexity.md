[![](http://img.youtube.com/vi/SDFZ4a5FepE/0.jpg)](http://www.youtube.com/watch?v=SDFZ4a5FepE "Chapter 13 - Exercise 3 - Depth Complexity")

Depth complexity refers to the number of pixels that compete, via the depth test, to be written to a particular entry in the back buffer. For example,
a pixel we have drawn may be overwritten by a pixel that is closer to the camera (and this can happen several times before the closest pixel is
actually figured out once the entire scene has been drawn). Thus, potentially, the graphics card could fill a pixel several times each frame. This
overdraw has performance implications, as the graphics card is wasting time processing pixels that eventually get overridden and are never seen.
Consequently, it is useful to measure the depth complexity in a scene for performance analysis. In addition, some games have used depth
complexity to render obscured pixels in a special way; for instance, in a strategy game, if some of your troops are behind a tree or building, you do
not want to forget about them, and therefore, the game might render those obscured objects in a special way so that you can still see them, but it
is implied that they are behind another object (the depth complexity can be used to indicate which pixels need to be specially rendered).
You can measure the depth complexity as follows: Render the scene and use the stencil buffer as a counter; that is, each pixel in the stencil buffer
is originally cleared to zero, and every time a pixel is processed, you increment its count with the D3DSTENCILOP_INCR state. Then, for example,
after the frame has been drawn, if the ijth pixel has a corresponding entry of five in the stencil buffer, then you know that that pixel was processed
five times that frame (i.e., the pixel has a depth complexity of five).
To visualize the depth complexity (stored in the stencil buffer), proceed as follows:
a. Associate a color ck for each level of depth complexity k. For example, blue for a depth complexity of one, green for a depth
complexity of two, red for a depth complexity of three, and so on. (In very complex scenes where the depth complexity for a pixel
could get very large, you probably do not want to associate a color for each level. Instead, you could associate a color for a range
of disjoint levels. For example, pixels with depth complexity 1–10 are colored blue, pixels with depth complexity 11–20 are
colored green, and so on.)
b. Set the stencil buffer operation to keep so that we do not modify it anymore. (We modify the stencil buffer with
D3DSTENCILOP_INCR when we are counting the depth complexity as the scene is rendered, but when writing the code to
visualize the stencil buffer, we only need to read from the stencil buffer and we should not write to it.)

Introduction to 3D Game Programming with DirectX 9.0c: A Shader Approach by Frank D. Luna.
