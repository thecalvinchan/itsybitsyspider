CS174A Assignment 2 Part 2 (Project)
Calvin Chan - 304144970

For this project, I created an animation of the Itsy Bitsy Spider Nursery Rhyme.

I created hierarchical objects through the spider legs, and the hollow water
spout. For the water spout, I wrote a function called drawRing() that used
drawCube() rotated around a central point. drawRing() was then called
iteratively in combination with a higher-level rotation to create the curved
part of the water spout. The spider legs were created similar to the bee legs
from Assignment 1, with the end of the upper leg acting as an anchor to the
lower leg.

I ran into issues with the ppmtompeg encoder. Although my animation played
successfully in XCode, it seems that the end of the animation has been cut off
from the mpeg. I think this is an issue with the encoder I used (ppmtompeg via
the Mac OS X Netpbm package from homebrew). Please take a look at the compiled
code to see the full animation. I've uploaded a version of the video onto 
youtube, if the provided mpg does not work 

https://www.youtube.com/watch?v=4DJQm7iuL1U
