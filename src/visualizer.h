#ifndef __VISUALIZER_H__
#define __VISUALIZER_H__

// Just so you can use fake static polymorphism here

class VisualizerImpl;

class VisualWrapperBase {
  friend class Visualizer;
  protected:
  virtual void draw()=0;
  virtual void configure(double w, double h)=0;
  virtual void initialize()=0;
  virtual void time(double t)=0;
  virtual ~VisualWrapperBase() {};
};

// Just so you can use fake static polymorphism here
template <class visual_t>
class VisualWrapper : public VisualWrapperBase {
  visual_t& visual;
  friend class Visualizer;
  VisualWrapper(visual_t& v) : visual(v) { }
  ~VisualWrapper() {}
  // Delegate everything
  void draw() { visual.draw(); }
  void configure(double w, double h) { visual.configure(w, h); }
  void initialize() { visual.initialize(); }
  void time(double t) { visual.time(t); }
};


// Fake singleton - only make one of these please!
class Visualizer {
  friend class VisualizerImpl;
  VisualizerImpl* mkimpl();

  void initialize();
  void configure(double w, double h); // Need dimensions
  void draw();

  public:

  template <class visual_t>
  Visualizer(visual_t& v) : visual(new VisualWrapper<visual_t>(v)), pimpl(mkimpl()) {}
  ~Visualizer();
  void mainloop(); // Displays window
  template<class visual_t>
  void change_visual(visual_t& v) {
    delete visual;
    visual = new VisualWrapper<visual_t>(v);
  }

  void destroy();

  private:
  VisualWrapperBase* visual;
  VisualizerImpl* pimpl;
};

#endif
