typedef struct {
  unsigned int    width;
  unsigned int   height;
  unsigned int   bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */ 
  unsigned short*  pixel_data; //[/*320 * 75 * 2 + 1*/];
} logo_t;
