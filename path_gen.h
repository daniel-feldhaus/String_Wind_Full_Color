#ifndef PATH_GEN_H
#define PATH_GEN_H

#include <exception>
#include <fstream>
#include <string>

#include "image_manipulation.h"
#include "json.hpp"
#include "point.h"
#include "color.h"
#include "defs.h"


namespace string_wind
{
    //Represents a single step in the string path. Contains a target pin, and the string color.
    struct path_step
    {
        ushort color_number = -1;
        ushort pin_number = -1;
    };

    //Returned by path generator. Iterates through each step in the string path. 
    class path_iterator
    {
    public:
        ushort num_steps, num_pins, num_colors;
        col* colors;
        point<float>* pins;

        int increment = 0;
        //Creates a path iterator with the given number of steps.
        path_iterator(ushort _num_steps, col* _colors, ushort _num_colors, point<float>* _pins, ushort _num_pins);

        //Destroys the path
        ~path_iterator();

        //Return a pointer to the start of the series
        path_step & begin() { return steps[0];};
        //Return a pointer to the end of the series
        path_step & end() { return steps[num_steps-1];};
        //Return a reference to the current step
        path_step & operator*(){ return steps[increment];};
        //Increment and return the current step
        path_step & operator++(int)
        {
            increment++;
            return steps[increment];
        }
    private:
        path_step* steps;        
    };

    class path_gen
    {
    public:
        //Create a path_gen instance, which can be used to generate a path from an input image. 
        path_gen(const char* config_name);

        //De-allocate teh path generator.
        ~path_gen();

        //Generate a path of colored strings around pegs, that will approximate the given image file.
        path_iterator generate_path();
    private:
        //Number of pins to draw between.
        u_short pin_num;
        //Number of string colors
        u_short color_num;
        //Number of steps in the path
        u_short num_steps;
        //Minimim distance between pins when choosing a path
        u_short min_pin_separation;

        //Radius of the pin circle, in image radii
        float pin_radius;

        //An array of the coordinates corresponding to each pin index.
        point<float>* pins;

        //Effects how much a path is darkened once it's been drawn
        float overlap_modifier;
        
        //String colors, given by the config file. Length = color_num
        col* colors;
        //String colors in LAB space.
        col* colors_Lab;

        //Color of the background behind the strings.
        col background_color;
        //Color of the background in LAB
        col background_color_Lab;
        
        //The input image, in LAB format with some minor adjustments. 
        image input_img;

        //The current string image appearance, in RGB. Used for debug visualization.
        image string_image;

        //The current string image appearance, in Lab space
        image string_image_Lab;
        //  (at the start, this is the difference between the background color and the image)
        image current_difference_map;

        //A collection of 2D difference maps between the input image and the string colors.
        //Dimensions:
        //  x,y: x,y coordinate of image.
        //  z: Color index
        //  w: 0
        image color_difference_slices;
        
        //A 2D array of the caverage color difference between each pin combo (difference between string_image_Lab and input_img)
        //Each combination is represented once, so the array is triangular, like so:
        //   0 1 2 3 4 x
        // 0 - * * * *
        // 1 - - * * * 
        // 2 - - - * *
        // 3 - - - - *
        // 4 - - - - -
        // y 
        //Dimensions:
        //  x: Pin A (Larger than pin B)
        //  y: Pin B (Smaller than pin A)
        img_type** current_grades;
        img_type*** color_grades; 

        //Load the settings for the path generation from a config json file.
        bool load_config(const char* file_name);
        
        //Populate the pins array with coordinates for each pin.
        void generate_pins();

        //Find the highest-graded line from the given pin to any other pin, graded with the given color.
        ushort best_target_from(ushort from_pin_index, ushort color_index, double& best_score_ref);

        void calculate_initial_grades();

        //Calculate a grade representing the potential increase in similarity if each string were drawn.
        void update_grades(short intersecting_pin_a = -1, short intersecting_pin_b = -1);
    };
}



#endif