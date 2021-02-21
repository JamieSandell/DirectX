[![](http://img.youtube.com/vi/0Dypsgxun1U/0.jpg)](http://www.youtube.com/watch?v=0Dypsgxun1U "Chapter 16 - Exercise 2 - Blending Animation Sets")

Suppose your game character has a running sequence and a gun firing sequence. It is probably the case that you would like your character to be
able to fire his gun as he is running. Now obviously, you could have your 3D artist create a separate running-firing sequence, but why not save
some content production time and memory by letting the computer create the mixed sequence by mathematically blending between the two
animations? This is, in fact, what animation blending allows you to do: It allows you to take existing animation sequences and mix them together to
create new sequences, as shown in Figure 16.11.
Conveniently, the D3DX animation API readily supports animation blending. If you completed the previous exercise, then you know that the
animation controller has several different tracks that you can attach an animation set to, but thus far, we only have been using the first track —
track zero. The key idea to using animation blending, with D3DX is that the animation controller automatically blends all the enabled track
animations together. Therefore, to perform animation blending, all we need to do is attach several animation sets to several different tracks, and
enable the tracks. Then when ID3DXAnimationControl1er::AdvanceTime animates the bones, it will do so using a blend of all the animation
tracks.
For this exercise, use the four animation sequences from tiny_4anim.x to create and play the following new animations: run-wave, by blending the
running animation with the wave animation; loiter-wave, by blending the loiter animation with the wave animation; and walk-wave, by blending the
walk animation with the wave animation. Note that you can also specify how much weight each track contributes to the final blended animation; for
example, you may want one track to contribute 30% and another track to contribute 70% to the final blended animation. The following methods will
help you (research them in the DirectX documentation):
ID3DXAnimationController::SetTrackAnimationSet
ID3DXAnimationController::GetAnimationSet
ID3DXAnimationController::ResetTime
ID3DXAnimationController::SetTrackEnable
ID3DXAnimationController::SetTrackSpeed
ID3DXAnimationController::SetTrackWeight
ID3DXAnimationController::SetTrackPriority

Introduction to 3D Game Programming with DirectX 9.0c: A Shader Approach by Frank D. Luna.
