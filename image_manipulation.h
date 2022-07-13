#ifndef IMAGE_MANIPULATION_H
#define IMAGE_MANIPULATION_H

#include "defs.h"
#include "math.h"
#include "point.h"
#include "CImg.h"
namespace string_wind
{
    class image_manipulation
    {
    public:
        //Create a 3D grayscale image with each slice corresponding to a difference map between the given image, and one of the given colors.
        static image calculate_difference_slices(const image& in_image, col* colors,const int num_colors, bool values_are_Lab);
        //Create a 2D grayscale image representing the difference between the given image and color.
        static image calculate_color_difference(const image &in_image, col in_color, bool values_are_Lab = false);
        //Create a 2D grayscale image representing the difference between the two given images.
        static image calculate_image_difference(const image &image_a, const image &image_b, bool images_are_Lab = false);
        //Create a solid-color image with the given dimensions
        static image solid_color_img(int width, int height, int depth,const col color);
        //Convert a color to LAB color space using the CImg Library
        static col col_RGBtoLab(const col rgb);
        //Return the color at the given pixel coordinate as a col object
        static col col_at_position(const image img, const point<int> coord);
        //Return the color at the given float coordinate as a col object, using bicubic interpolation
        static col col_at_position(const image img, const point<float> coord);
        //Calculate the dfference between colors in LAB color space
        static img_type color_difference(const col& color_a_Lab, const col& color_b_Lab);

        //Fill the given image with the given color 
        static void fill_with_color(image& to_fill, col fill_color);
        //Calculate the average value in a straight line between from and to, using bicubic interpolation.
        static img_type average_grayscale_along_line(const image& gray_image,const point<float> from,const point<float> to,const float step_amount);
        //Calculate the average difference between the given color and image, along the given line, using bicubic interpolation.
        static img_type average_color_difference_along_line(const image& diff_img, const col& diff_color, const point<float> from, const point<float> to, const float step_amount);
        //Draw a line with the same bicubic method used in the line averaging functions.
        static void draw_color_along_line(image& draw_img, const col draw_col, const point<float> from, const point<float> to, const float step_amount);
    };
}

#endif