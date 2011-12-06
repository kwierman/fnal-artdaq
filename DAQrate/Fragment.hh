#ifndef Fragment_HHH
#define Fragment_HHH

typedef long ElementType;

struct FragHeader
{
  ElementType dummy[2];
  ElementType id_;
  ElementType from_;
  ElementType time_ms_; // length of time to process
};

#endif
