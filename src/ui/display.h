namespace nativeapi {

// Representation of a display
struct Display {
  char* id;
  char* name;
  double width;
  double height;
  double visiblePositionX;
  double visiblePositionY;
  double visibleSizeWidth;
  double visibleSizeHeight;
  double scaleFactor;
};

// Representation of a list of displays
struct DisplayList {
  Display* displays;
  int count;
};

}  // namespace nativeapi