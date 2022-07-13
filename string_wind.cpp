#include "path_gen.h"
#include <iostream>

using namespace string_wind;
int main(int argc, char** argv)
{

    char config_file_name[20] = "config.json";

    path_gen pg(config_file_name);
    path_iterator pit = pg.generate_path();

    return 0;
}