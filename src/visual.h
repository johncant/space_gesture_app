#ifndef __WAVES_VISUAL_H__
#define __WAVES_VISUAL_H__

class VisualImpl;

class Visual {
  VisualImpl* pimpl;

  public:
  ~Visual();
  Visual();
  void initialize();
  void configure(double w, double h);
  void draw();
  void time_advance(double t);

  void time(double t);
};

#endif
