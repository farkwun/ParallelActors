//===================================
// include guard
#ifndef COORDINATE_H
#define COORDINATE_H

class Coordinate {
  private:
    int x;
    int y;
  public:
    int get_x();
    int get_y();
    void set_x(int new_x);
    void set_y(int new_y);
};
#endif /* COORDINATE_H */
