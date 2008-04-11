The osgstereomatch example illustrates how multiple render passes and multiple render targets on the GPU can be used in stereo vision disparity map calculation.

A simple algorithm is implemented in both a single- and multi-pass way to show the differences in speed of execution.

The normal osgviewer is used for display and one can obtain the time spent on the GPU by pressing the "s" key twice.


Usage examples:
---------------

Using sample data in OpenSceneGraph-Data distribution:

  osgstereomatch --left Images/dog_left_eye.jpg --right Images/dog_right_eye.jpg --min 0 --max 31 --window 9
  osgstereomatch --left Images/dog_left_eye.jpg --right Images/dog_right_eye.jpg --min 0 --max 31 --window 9 --single

Using sample data available from the Middlebury Stereo Vision Page at http://vision.middlebury.edu/stereo/ :

  osgstereomatch --left tsukubaL.png --right tsukubaR.png --min 0 --max 15 --window 5
  osgstereomatch --left tsukubaL.png --right tsukubaR.png --min 0 --max 15 --window 5 --single
  osgstereomatch --left teddyL.png --right teddyR.png --min 10 --max 57 --window 7


Algorithm and implementation details:
-------------------------------------
The algorithm uses the sum of absolute differences between pixels (often referred to as SAD in the literature) to find matching areas in the left and right stereo images. The algorithm is definitely not state of the art, but illustrates stereo matching concepts.

The algorithm assumes that the input images are already rectified and undistorted.

Inputs to the algorithm are:
* Window size -- The size (width and height) of a rectangular window around the current pixel that is used for comparison.
* Minimum disparity -- The minimum pixel offset at which to start searching for matching areas in the right image.
* Maximum disparity -- The maximum pixel offset at which to stop searching for matching areas in the right image.

The offsets can be visualised as shifting the right stereo image to the right by the specified number of pixels.

The following pseudo-code describes the algorithm:
for every pixel in the left image:
 for the complete range of disparity values:
  pixel-wise sum the absolute differences between the left and right windows
  (the right window is shifted by the current disparity value evaluated)
  store the disparity value where the sum is the smallest
 assign the stored disparity value to the disparity image pixel

The algorithm was implemented using GLSL shaders. Two implementation of the algorithm was created to compare the execution speed using two different approaches. The first version uses a single shader and a single rendering pass, while the second approach utilises several shaders and rendering passes to create a disparity image.

Single Pass
-----------

The single pass algorithm implements the pseudo-code directly as presented earlier. The code for the algorithm can be found in the ``stereopass.frag'' source file.

Multiple Pass
-------------

The single pass algorithm often re-computes the same absolute differences between left and right image pixels. For example, when shifting a 3x3 window one pixel horizontally, 6 absolute difference values would be recomputed. In the multi-pass algorithm, the absolute differences between pixels are pre-computed before doing the summation over a window.

The algorithm consists of 4 passes:

1) Clear pass -- Two floating point textures are created for storing the minimum sum (SAD) and best disparity value calculated so far (called disparity textures in the following discussion). The minimum sum is stored in the first channel of the images and the best disparity is stored in the second channel. Two textures are needed, because shaders cannot read and write to the same texture during calculation. One texture is used as input and the other as output. After a calculation, the input and output textures are swapped. The technique is known as ping-ponging or flip-flopping the textures. The clear pass initialises the two textures. The code can be found in ``clear.frag''.

2) Subtract pass -- Sixteen disparity values are considered and the shifted right image is subtracted from the left image. The absolute difference of the pixel values are stored in one of the four channels of one of four output images. The multiple render target (MRT) capabilities of modern graphics cards are utilised during this pass. The code can be found in ``subtract.frag''.

3) Aggregate pass -- The sixteen images generated during the subtract pass is used as input for this pass. For each of the images, a sum over the window centered at the current output pixel is computed and stored. The minimum sum of the sixteen images is then compared with the lowest sum calculated during previous passes (read from the input disparity texture). If it is lower, the output disparity texture is updated with the new sum and the new disparity value, if not, the old values are copied. The code can be found in ``aggregate.frag''.

4) Select pass -- The final disparity texture contains the best disparity value found in the second (green) channel. This pass merely copies the second channel to all the colour channels of the output image. The code can be found in ``select.frag''.

The subtract and aggregate passes are executed multiple times to cover the complete requested disparity range. Disparity range divided by 16 combinations of these passes are needed. Despite the apparent complexity of the algorithm, it executes faster than the single pass algorithm.

J.P. Delport 2008/04
