#include "image_manipulation.h"

using namespace string_wind;
#include <iostream>
// Create 1-channel image slices representing how different a color is from the input image.
//   in_image and colors are expected to be in RGB format.
image image_manipulation::calculate_difference_slices(const image &in_image, col *colors, const int num_colors, bool values_are_Lab)
{
    image slices(in_image.width(), in_image.height(), num_colors, 1);
    cimg_forZ(slices, c)
    {
        slices.get_shared_slice(c) = calculate_color_difference(in_image, colors[c], values_are_Lab);
    }

    slices.normalize(0, max_img_val);
    return slices;
}

image image_manipulation::calculate_color_difference(const image &in_image, col in_color, bool values_are_Lab)
{
    image in_image_Lab = values_are_Lab ? (image)in_image.get_RGBtoLab() : in_image;
    col color_Lab = values_are_Lab ? in_color : image_manipulation::col_RGBtoLab(in_color);
    image color_image_Lab = solid_color_img(in_image.width(), in_image.height(), in_image.depth(), color_Lab);

    return calculate_image_difference(in_image_Lab, color_image_Lab, true);
}

image image_manipulation::calculate_image_difference(const image &image_a, const image &image_b, bool images_are_Lab)
{
    image image_a_Lab = images_are_Lab ? image_a.get_shared() : image(image_a.get_RGBtoLab());
    image image_b_Lab = images_are_Lab ? image_b.get_shared() : image(image_b.get_RGBtoLab());
    image difference3D = image_a_Lab - image_b_Lab;
    image difference = difference3D.get_shared_channel(0).get_sqr() +
                       difference3D.get_shared_channel(1).get_sqr() +
                       difference3D.get_shared_channel(2).get_sqr();
    difference.sqrt();
    return difference;
}

// Create a solid-color image with the given color / dimensions.
image image_manipulation::solid_color_img(int width, int height, int depth, const col fill_color)
{

    image color_image(width, height, depth, 3);
    fill_with_color(color_image, fill_color);
    return color_image;
}

col image_manipulation::col_at_position(const image img, const point<int> coord)
{
    return col(img(coord.x, coord.y, 0, 0),
               img(coord.x, coord.y, 0, 1),
               img(coord.x, coord.y, 0, 2));
}

col image_manipulation::col_at_position(const image img, const point<float> coord)
{
    return col(img.cubic_atXY(coord.x, coord.y, 0, 0),
               img.cubic_atXY(coord.x, coord.y, 0, 1),
               img.cubic_atXY(coord.x, coord.y, 0, 2));
}

// Converts the given rgb color to Lab space.
// Full disclosure: I barely understand this, and I cobbled it together from CImg.h's rgbtolab() and xyztolab() functions.
col image_manipulation::col_RGBtoLab(const col rgb)
{
#define Labf(x) (24389 * (x) > 216 ? std::cbrt(x) : (24389 * (x) / 27 + 16) / 116)

    float r_flt, g_flt, b_flt;
    float x, y, z;

    color<float> white_xyz(0.95047, 1.0000001, 1.08893);

    // Scale rgb values to 0 -> 1
    r_flt = (float)rgb.r / 255.0f;
    g_flt = (float)rgb.g / 255.0f;
    b_flt = (float)rgb.b / 255.0f;
    // Convert to xyz color space
    x = (0.4124564f * r_flt + 0.3575761f * g_flt + 0.1804375f * b_flt);
    y = (0.2126729f * r_flt + 0.7151522f * g_flt + 0.0721750f * b_flt);
    z = (0.0193339f * r_flt + 0.1191920f * g_flt + 0.9503041f * b_flt);
    // Divide values by the white xyz values.
    x /= 0.95047f;
    z /= 1.08893f;
    // Do this magic that I don't understand (taken from the CImg xyztorgb() function)
    x = Labf(x);
    y = Labf(y);
    z = Labf(z);
    // A little more magic
    col lab(std::clamp((int)(116.f * y - 16), 0, 100),
            500 * (x - y),
            200 * (y - z));

    return lab;
}

// Fill the given image with the given color
void image_manipulation::fill_with_color(image &to_fill, col fill_color)
{
    cimg_forXYZC(to_fill, x, y, z, c)
    {
        to_fill(x, y, z, c) = fill_color[c];
    }
    return;
}

img_type image_manipulation::color_difference(const col &color_a_Lab, const col &color_b_Lab)
{
    long sqr_sum = pow(std::abs(color_a_Lab.l - color_b_Lab.l), 2) +
                   pow(std::abs(color_a_Lab.a - color_b_Lab.a), 2) +
                   pow(std::abs(color_a_Lab.b - color_b_Lab.b), 2);
    return (img_type)std::sqrt(sqr_sum);
}

// TODO: Implement a rolling averaging function that's better at avoiding overflows.
// Step in a straight line along the image, and calculate the average of the bicubic-interpolated values.
// Inputs:
//   gray_image: The grayscale image to find the average value for
//   from: End A of the line
//   to: End B of the line
//   step_amount: The amount to step the line forward, in unit space (1 = the image width)
img_type image_manipulation::average_grayscale_along_line(const image &gray_image, point<float> from, point<float> to, const float step_amount)
{
    // Keep the order of the from / to pins consistent - this way, regardles of what order they're given, the same positions are sampled.
    // Sort by x, or y if x is equal.
    point<float> temp;
    if ((to.x < from.x) || ((to.x == from.x) && (to.y < from.y)))
    {
        temp = from;
        from = to;
        to = temp;
    }

    // Start at the from position
    point<float> current_position(from);
    point<float> step_vector = (to - from).normalized() * (step_amount * gray_image.width());
    // The square of the length from start to end
    double from_to_sqr_mag = (to - from).sqr_manitude();
    // The sum of all measured points. Using a very big type to avoid overflows.
    __float128 sum = 0;
    // The total number of points sampled
    ushort sample_count = 0;
    // While the line from start to current is shorter than the line from start to end, step and add the
    while ((current_position - from).sqr_manitude() < from_to_sqr_mag)
    {
        sum += gray_image(current_position.x, current_position.y, 0, 0);
        // sum += gray_image.cubic_atXY_p(current_position.x, current_position.y);
        // std::cout << "Cubic Val: " << gray_image.cubic_atXY_p(current_position.x, current_position.y) << '\n';
        // std::cout << "Closest pixel val: " << gray_image.atXY((int)current_position.x, (int)current_position.y) << '\n';
        sample_count++;
        current_position += step_vector;
    }
    return (img_type)(sum / sample_count);
}

void image_manipulation::draw_color_along_line(image &draw_img, const col draw_col, const point<float> from, const point<float> to, const float step_amount)
{
    // Start at the from position
    point<float> current_position(from);
    point<float> step_vector = (to - from).normalized() * (step_amount * draw_img.width());
    // The length from start to end
    double from_to_sqr_mag = (to - from).sqr_manitude();
    // The sum of all measured points. Using a very big type to avoid overflows.
    // The total number of points sampled
    // While the line from start to current is shorter than the line from start to end, step and add the
    while ((current_position - from).sqr_manitude() < from_to_sqr_mag)
    {
        draw_img(current_position.x, current_position.y, 0, 0) = draw_col.r;
        draw_img(current_position.x, current_position.y, 0, 1) = draw_col.g;
        draw_img(current_position.x, current_position.y, 0, 2) = draw_col.b;

        // draw_img.set_linear_atXY(draw_col.r, current_position.x, current_position.y, 0, 0);
        // draw_img.set_linear_atXY(draw_col.g, current_position.x, current_position.y, 0, 1);
        // draw_img.set_linear_atXY(draw_col.b, current_position.x, current_position.y, 0, 2);

        current_position += step_vector;
    }
    return;
}