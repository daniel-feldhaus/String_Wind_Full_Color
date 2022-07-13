#include "path_gen.h"
#include <iostream>
using namespace string_wind;



//Constructor for the path iterator, allocates space to store the step instructions.
path_iterator::path_iterator(ushort _num_steps, col* _colors, ushort _num_colors, point<float>* _pins, ushort _num_pins)
{
    num_steps = _num_steps;
    steps = new path_step[_num_steps];
    colors = _colors;
    num_colors = num_colors;
    pins = _pins;
    num_pins = _num_pins;
    return;
}

//TODO: Shared memory should probably be deleted by the class that originally allocated it.
//Destructor, de-allocate the created arrays.
path_iterator::~path_iterator()
{
    if(steps) delete[] steps;
    if(colors) delete[] colors;
    if(pins) delete[] pins;
    return;
}


//Constructor, loads values from the given config file and gets everything ready for path generation.
path_gen::path_gen(const char* config_name)
{
    colors_Lab = nullptr;
    colors = nullptr;
    if(!load_config(config_name)) throw std::runtime_error("Config file incomplete or missing.");
    generate_pins();

    calculate_initial_grades();

} 

//Destructor. Currently doesn't do anything.
path_gen::~path_gen()
{
    if(colors_Lab) delete[] colors_Lab;
    if(colors) delete[] colors;
    for(int pin_index_x = 0; pin_index_x < pin_num; pin_index_x++)
    {
        for(int pin_index_y = 0; pin_index_y < pin_index_x; pin_index_y++)
        {
            delete[] color_grades[pin_index_x][pin_index_y];
        }
        if(pin_index_x)
        {
            delete[] current_grades[pin_index_x];
            delete[] color_grades[pin_index_x];
        }
    } 
    delete[] current_grades;
}


//Generates the instructions for the strings, and returns them in a path_iterator object.
//The bulk of the work happens within this funciton. 
path_iterator path_gen::generate_path()
{
    //The iterator object where the calculated steps are stored. 
    path_iterator pit(num_steps, colors, color_num, pins, pin_num);
    //Stores the latest pin visited by each color.
    ushort* prev_pin_foreach_color = new ushort[color_num];
    
    ushort prev_pin_index, cur_next_pin_index, next_pin_index, next_color_index;
    //The current score being evaluated
    double score;
    //The best score calculated each round
    double best_score;
    //Normalize the input image from zero to the maximum value possible.
    //input_img.normalize(0,max_img_val);
    //Calculate the LAB color difference between the image and the background color
   //current_difference_map = image_manipulation::calculate_color_difference(input_img, background_color,true);
    //Calculate the color difference maps for each string color.
    //color_difference_slices = image_manipulation::calculate_difference_slices(input_img, colors, color_num);
    //Create the string image, which at the start is just the background color in Lab.
    string_image_Lab = image_manipulation::solid_color_img(input_img.width(),
                                                           input_img.height(),
                                                           input_img.depth(), 
                                                           background_color_Lab);  
    string_image     = image_manipulation::solid_color_img(input_img.width(),
                                                           input_img.height(),
                                                           input_img.depth(), 
                                                           background_color);                

    //Set each color's starting pin index to zero.
    for(int c = 0; c < color_num; c++)
    {
        (*pit).pin_number = 0;
        prev_pin_foreach_color[c] = 0;
        (*pit).color_number = (ushort)c;
        pit++;
    }

    const img_type foreground[4]{1,1,1,1};
    for(int i = 0; i < pin_num; i++)
    {
        string_image.draw_text((int)pins[i].x,(int)pins[i].y, std::to_string(i).c_str(), foreground, 0, 0.8f, 24); 
    }

    //Main loop: Calculates the best-scoring step and adds it to the iterator until the desired step count is reached.
    for(int i = color_num; i < num_steps; i++)
    {
        best_score = max_img_val;
        next_pin_index = 0;
        next_color_index = 0;
        current_difference_map = image_manipulation::calculate_image_difference(string_image_Lab, input_img, true);

        //Score all step options for each color, and choose the best color / step to add to the iterator.
        for(int c = 0; c < color_num; c++)
        {
            prev_pin_index = prev_pin_foreach_color[c];
            cur_next_pin_index = best_target_from(prev_pin_index, c, score);
            //std::cout << "Score for color " << c << ": " << score << ", pin " <<  cur_next_pin_index << '\n';
            if(score < best_score)
            {
                best_score = score;
                next_pin_index = cur_next_pin_index;
                next_color_index = (ushort)c;
            }
        }
        (*pit).color_number = next_color_index;
        (*pit).pin_number = next_pin_index;
        prev_pin_index = prev_pin_foreach_color[next_color_index];
        prev_pin_foreach_color[next_color_index] = next_pin_index;
       // const img_type color_array[] = {colors_Lab[next_color_index].l,
        //                                colors_Lab[next_color_index].a,
         //                               colors_Lab[next_color_index].b};
        image_manipulation::draw_color_along_line(string_image_Lab, colors_Lab[next_color_index],pins[prev_pin_index], pins[next_pin_index], 0.001f);
        image_manipulation::draw_color_along_line(string_image, colors[next_color_index],pins[prev_pin_index], pins[next_pin_index], 0.001f);
            
        //string_image_Lab.draw_line((int)pins[prev_pin_index].x, (int)pins[prev_pin_index].y, 
        //                           (int)pins[next_pin_index].x, (int)pins[next_pin_index].y,
         //                          color_array);

       // const img_type color_array_debug[] = {colors[next_color_index].r,
        //                                colors[next_color_index].g,
         //                               colors[next_color_index].b};
       // string_image.draw_line((int)pins[prev_pin_index].x, (int)pins[prev_pin_index].y, 
        //                           (int)pins[next_pin_index].x, (int)pins[next_pin_index].y,
         //                          color_array_debug);

        
        std::cout << "Next pin: " << next_pin_index << ", Color Index: " << next_color_index << ", Grade: " << best_score << '\n';
        pit++;

        //TEMPORARY
        string_image.save("string_img.png");
        string_image_Lab.save("string_img_Lab.png");
        current_difference_map.save("s_cur_difference.png");
        (current_difference_map - color_difference_slices.get_slice(2)).save("scores_blue.png");
    }

    //De-allocate dynamic memory and return the iterator.

    delete[] prev_pin_foreach_color;
    return pit;
}


//Loads config values from file, and stores them in class variables.
bool path_gen::load_config(const char* file_name)
{
    std::ifstream i(file_name);
    nlohmann::json config_json;
    nlohmann::json j_string_colors, j_background_color;
    const char* image_path;
    i >> config_json;
    //Check for missing fields in the json file
    if(
        config_json.empty() ||
        !config_json.contains("string_colors") ||
        !config_json.contains("background_color") ||
        !config_json.contains("pin_count") ||
        !config_json.contains("pin_radius") ||
        !config_json.contains("overlap_modifier") ||
        !config_json.contains("step_count") ||
        !config_json.contains("min_pin_separation") ||
        !config_json.contains("image_path")
    ) return false;

    j_string_colors = config_json["string_colors"];

    j_background_color = config_json["background_color"];

    color_num = j_string_colors.size();
    colors = new col[color_num];
    colors_Lab = new col[color_num];
    for(int i=0; i < color_num; i++)
    {
        colors[i].r = j_string_colors[i][0].get<img_type>();
        colors[i].g = j_string_colors[i][1].get<img_type>();
        colors[i].b = j_string_colors[i][2].get<img_type>();

        colors_Lab[i] = image_manipulation::col_RGBtoLab(colors[i]);  


        std::cout << "RGB color: " << colors[i][0] << ',' <<  colors[i][1] << ',' <<  colors[i][2] << '\n';
        std::cout << "LAB color: " << colors_Lab[i][0] << ',' <<  colors_Lab[i][1] << ',' <<  colors_Lab[i][2] << '\n';

    }

    background_color.r = j_background_color[0].get<img_type>();
    background_color.g = j_background_color[1].get<img_type>();
    background_color.b = j_background_color[2].get<img_type>();

    background_color_Lab = image_manipulation::col_RGBtoLab(background_color);

    pin_num = config_json["pin_count"].get<u_short>();
    pin_radius = config_json["pin_radius"].get<float>();

    overlap_modifier = config_json["overlap_modifier"].get<float>();
    num_steps = config_json["step_count"].get<u_short>();
    min_pin_separation = config_json["min_pin_separation"].get<u_short>();

    image_path = config_json["image_path"].get<std::string>().c_str();
    input_img = image(image_path);
    std::cout << "Input image range: " << input_img.min() << " -> " << input_img.max() << '\n';
//    input_img.RGBtoLab();

    std::cout << "Pin count: " << pin_num << '\n';
    std::cout << "Step count: " << num_steps << '\n';
    std::cout << "Color count: " << color_num << '\n';
    return true;
}

void path_gen::generate_pins()
{
    float angle;
    //The pixel-space radius is some percent (determined by pin_radius) of the smallest dimension of the image (either width or height)
    int pixel_space_radius = pin_radius * std::min(input_img.width(), input_img.height())/2;
    point<float> center((float)input_img.width()/2, (float)input_img.height()/2);
    pins = new point<float>[pin_num];
    for(int pin_index = 0; pin_index < pin_num; pin_index++)
    {   
        angle = ((float)pin_index / pin_num) * M_PI * 2.0f;
        pins[pin_index].x = center.x + cos(angle) * pixel_space_radius;
        pins[pin_index].y = center.y + sin(angle) * pixel_space_radius;
    }
}

ushort path_gen::best_target_from(ushort from_pin_index, ushort color_index, double& best_score_ref)
{
    //Index of the pin with the best score
    ushort best_pin_index = 0;
    //Score of the pin at best_pin_index
    double best_score = min_img_val;
    //Current score being evaluated in the loop
    double cur_score_difference;
    
    //A shared-memory copy of the difference map being evaluated
    //image graded_image = color_difference_slices.get_shared_slice(color_index);
    
    col& cur_color_Lab = colors_Lab[color_index];
    
    image color_difference_img = image_manipulation::calculate_color_difference(input_img, cur_color_Lab, true);
    image scores = current_difference_map - color_difference_img;
   // input_img.get_LabtoRGB().save("in.png");
    //cur_difference_img.get_normalize(0,255).save("Cur.png",from_pin_index,3);
    //color_difference_img.get_normalize(0,255).save("col.png",from_pin_index,3);
    //scores.get_normalize(0,255).save("difference.png",from_pin_index,3);
    //The given pin to draw from
    point<float>* from_pin = &pins[from_pin_index];
    //The current pin being evaluated
    point<float>* to_pin;

    
    //Loops through all pins (except for from_pin), and find the one with the highest score.
    for(int to_pin_index=0; to_pin_index < pin_num; to_pin_index++)
    {
        if(to_pin_index != from_pin_index)
        {
            to_pin = &pins[to_pin_index];
            //cur_score = image_manipulation::average_color_difference_along_line(string_image_Lab, cur_color_Lab, *from_pin, *to_pin, 0.001f);
            //cur_score_old = image_manipulation::average_grayscale_along_line(cur_difference_img, *from_pin, *to_pin, 0.001f);
            //cur_score_new = image_manipulation::average_grayscale_along_line(color_difference_img, *from_pin, *to_pin, 0.001f);
            cur_score_difference = image_manipulation::average_grayscale_along_line(scores, *from_pin, *to_pin, 0.001f);
            //std::cout << "Line " << from_pin_index << "->" << to_pin_index <<  " Old Score: " << cur_score_old << ", New Score: " << cur_score_new << ", Change Score: " << cur_score_difference << '\n';

            if(cur_score_difference > best_score)
            {
                best_score = cur_score_difference;
                best_pin_index = to_pin_index;
            }
        }
    }
    best_score_ref = best_score;
    return best_pin_index;
}

void path_gen::calculate_initial_grades()
{
    color_difference_slices = image_manipulation::calculate_difference_slices(input_img, colors_Lab, color_num, true);
    //Allocate the triangular arrays to store the grades.
    current_grades = new img_type*[pin_num];
    color_grades = new img_type**[pin_num];
    img_type score;
    point<float> *min_pin,  *max_pin;
    for(int pin_index_x = 0; pin_index_x < pin_num; pin_index_x++)
    {
        max_pin = &pins[pin_index_x];
        current_grades[pin_index_x] = (pin_index_x ) ? new img_type[pin_index_x] : nullptr;
        color_grades[pin_index_x] = (pin_index_x) ? new img_type*[pin_index_x] : nullptr;
        for(int pin_index_y = 0; pin_index_y < pin_index_x; pin_index_y++)
        {
            min_pin = &pins[pin_index_y];
            current_grades[pin_index_x][pin_index_y] = 0;
            color_grades[pin_index_x][pin_index_y] = new img_type[color_num];
            for(int color_index = 0; color_index < color_num; color_index++)
            {
                score = image_manipulation::average_grayscale_along_line(color_difference_slices.get_shared_slice(color_index),*max_pin,*min_pin,0.001f);
                color_grades[pin_index_x][pin_index_y][color_index] = score;
            }
        }
    }

    
}


void path_gen::update_grades(short intersecting_pin_a, short intersecting_pin_b)
{
    image color_difference_image;
    ushort max_pin_index, min_pin_index;
    point<float> *max_pin, *min_pin;

    for(int pin_a_index = (intersecting_pin_a + 1) % 255; pin_a_index != intersecting_pin_b; pin_a_index = (pin_a_index+1) % 255)
    {
        for(int pin_b_index = (intersecting_pin_b + 1) % 255; pin_b_index != intersecting_pin_a; pin_b_index = (pin_b_index+1) % 255)
        {
            max_pin_index = std::max(pin_a_index,pin_b_index);
            min_pin_index = std::max(pin_a_index,pin_b_index);
            max_pin = &pins[max_pin_index];
            min_pin = &pins[min_pin_index];
            current_grades[max_pin_index][min_pin_index] = image_manipulation::average_grayscale_along_line(current_difference_map, *max_pin, *min_pin, 0.001f);
        }
    }
}
