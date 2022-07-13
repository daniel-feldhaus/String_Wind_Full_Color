#ifndef POINT_H
#define POINT_H

namespace string_wind
{
    template<class T>
    class point
    {
    public:
        T x, y;
        //Create a point with the given coordinates.
        point<T>(T _x, T _y)
        {
            x = _x;
            y = _y;
            return;
        }
        //Create a copy of another point.
        point<T>(const string_wind::point<T>& to_copy)
        {
            x = to_copy.x;
            y = to_copy.y;
            return;
        }
        //Create a zero point
        point<T>()
        {
            x = 0;
            y = 0;
            return;
        }

        double sqr_manitude()
        {
            return std::pow(x,2) + std::pow(y,2);
        }

        //Calculate the magnitude (or length) from the point (0,0) to the point (x,y).
        //WARNING: This is expensive. When possible, use sqr_magnitude.
        double magnitude()
        {
            return std::sqrt(sqr_manitude());
        }

        point<T> normalized()
        {
            return *this / magnitude();
        }

        //Return linear sum of two points.
        point<T> operator+(const point<T>& other)
        {
            return point<T>(x + other.x, y + other.y);
        }
        //Add the given point to this.
        point<T> operator+=(const point<T>& other)
        {
            x += other.x;
            y += other.y;
            return *this;
        }
        //Return linear subtraction of other from this.
        point<T> operator-(const point<T>& other)
        {
            return point<T>(x - other.x, y - other.y);
        }
        point<T> operator-(const point<T>& other) const
        {
            return point<T>(x - other.x, y - other.y);
        }
        //Multiply x and y by a scalar.
        point<T> operator*(const T s)
        {
            return point<T>(x * s, y * s);
        }
        //Divide x and y by a scalar.
        point<T> operator/(const T s)
        {
            return point<T>(x / s, y / s);
        }



    };
}
#endif