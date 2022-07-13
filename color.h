#ifndef COLOR_H
#define COLOR_H

namespace string_wind
{
    template<class T>
    class color
    {
    public:
        //References are provided for both rgb and lab. Currently, there's no difference and they point to the same value.
        //  This is simply to make it more clear which space the color is currently in. 
        T channels[3];

        T& r = channels[0];
        T& l = channels[0];

        T& g = channels[1];
        T& a = channels[1];

        T& b = channels[2];
        
        color(const T c0,const T c1,const T c2)
        {
            channels[0] = c0;
            channels[1] = c1;
            channels[2] = c2;
            return;
        }
        color(const color<T>& to_copy)
        {
            channels[0] = to_copy.channels[0];
            channels[1] = to_copy.channels[1];
            channels[2] = to_copy.channels[2];
            return;
        }
        color()
        {
            channels[0] = 0;
            channels[1] = 0;
            channels[2] = 0;
            return;
        }
        color& operator =(const color& other)
        {
            r = other.r;
            g = other.g;
            b = other.b;
            return *this;
        }
        T& operator [](int i)
        {
            return channels[i];
        }
        T& operator [](int i) const
        {
            return channels[i];
        }
    };
}

#endif