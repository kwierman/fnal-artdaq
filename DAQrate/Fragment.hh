#ifndef Fragment_HHH
#define Fragment_HHH

typedef long ElementType;

struct FragHeader
{
  ElementType frag_words_;
  ElementType from_;
  ElementType id_;
  ElementType time_ms_; // length of time to process
};

#endif
