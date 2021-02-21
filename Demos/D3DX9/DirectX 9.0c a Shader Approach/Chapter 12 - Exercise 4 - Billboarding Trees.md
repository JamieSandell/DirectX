[![](http://img.youtube.com/vi/_tK2XkkMNSQ/0.jpg)](http://www.youtube.com/watch?v=_tK2XkkMNSQ "Chapter 12 - Exercise 4 - Billboarding Trees")

When trees are far away, a billboarding technique is used for efficiency. That is, instead of rendering the geometry for a fully 3D tree, a 2D
polygon is used with a picture of a 3D tree painted on it (see Figure 12.12). From a distance, you cannot tell that a billboard is being used.
However, the trick is to make sure that the billboard always faces the camera (otherwise the illusion would break). For trees, assuming the y-axis is
up, billboards will generally be aligned with the y-axis and just face the camera in the xz-plane. For other kinds of billboards, such as particles, the
billboards may fully face the camera.

Introduction to 3D Game Programming with DirectX 9.0c: A Shader Approach by Frank D. Luna.
