[![](http://img.youtube.com/vi/1GtB946Q23s/0.jpg)](http://www.youtube.com/watch?v=1GtB946Q23s "Chapter 16 - Exercise 1 - Animation Sets")

An animated character can contain the data for several animation sequences, which we recall the D3DX animation API calls animation sets. For
example, a character may have walking, running, gun firing, jumping, and death sequences. Of course, to play multiple animation sequences, the
.X file must contain the animation sets defining each sequence. The DirectX SDK MultiAnimation sample comes with a file called tiny_4anim.x,
which contains four different animation sequences (animation sets) as shown in Figure 16.10.
For this exercise, load tiny_4anim.x, and allow the user to switch between the four animation sequences using keyboard input (i.e., key 1 plays the
first sequence, key 2 plays the second sequence, key 3 plays the third sequence, and key 4 plays the fourth sequence). The following methods will
help you (research them in the DirectX documentation):
ID3DXAnimationController::SetTrackAnimationSet
ID3DXAnimationController::GetAnimationSet
ID3DXAnimationController::ResetTime

Introduction to 3D Game Programming with DirectX 9.0c: A Shader Approach by Frank D. Luna.
