#ifndef DEFS_H
#define DEFS_H

#include "CImg.h"
#include "color.h"
#include <limits>
#define img_type short
typedef cimg_library::CImg<img_type> image;
typedef cimg_library::CImgList<img_type> image_list; //currently not in-use
typedef string_wind::color<img_type> col;
#define max_img_val 255//std::numeric_limits<img_type>::max()
#define min_img_val std::numeric_limits<img_type>::min()
#endif