//===================================
// include guard
#ifndef COORDINATE_H
#define COORDINATE_H

class Coordinate {
  private:
    int row;
    int col;
  public:
    int get_row();
    int get_col();
    void set_row(int new_row);
    void set_col(int new_col);
    void set_row_col(int new_row, int new_col);
};
#endif /* COORDINATE_H */
