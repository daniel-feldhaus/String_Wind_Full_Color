#include "path_gen.h"
#include <iostream>

using namespace string_wind;
int main(int argc, char** argv)
{
    if(argc != 3)
    {
        std::cout << "Incorrect input format. Should be: './string_wind <input_image_name> <config_file_name>'";
        return 0;
    }
    char* image_file_name = argv[1];
    char* config_file_name = argv[2];

    path_gen pg(config_file_name);
    path_iterator pit = pg.generate_path();

    return 0;
}