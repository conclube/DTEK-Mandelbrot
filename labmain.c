extern void print(const char*);
extern void print_dec(unsigned int);
extern void print_hex32(unsigned int);
extern void printc(char);

// memcpy if compiler flags -O0 requires it
void *memcpy(void *dest, const void *src, unsigned n) {
    for (unsigned i = 0; i < n; i++) {
        ((char*)dest)[i] = ((char*)src)[i];
    }
}

struct datakey {
  int* ptr;
  char type;
};

struct complex {
  double real;
  double imag;
};

struct mandelbrot {
  char type;
  double xmax;
  double xmin;
  double ymax;
  double ymin;
  int res;
};

struct julia {
  char type;
  double xmax;
  double xmin;
  double ymax;
  double ymin;
  double real;
  double imag;
  int res;
};

struct sierpinski {
  char type;
  int res;
};

/*
  Normally a program uses the heap to allocate memory for our configuration data and cached dictionaries.
  In DTEKV that is not possible, so the second best option is to use 'static'. However, there are issues 
  with modelling a heap using 'static'.
  
  If one is required to step out (^C) the program through the terminal before being able to download 
  data (via dtekv-upload) from the board. Then once one steps back into the program, all static memory 
  sections are cleared (via dtekv-run). A usecase of where this issue becomes annoyingly noticeable is
  if one wants to download files during program runtime.

  If the program is already running and want to first download any given image (which requires us to step out)
  and then step back into the program to generate another image this would not be possible with 'static'.
  Therefore, using relativly highly ordered memory addresses was the solution. 

  The opted solution is not correct though. The issue with just using random memory addresses
  is that the program may decide to use them for other purposes, in which memory segfaults and
  other inconsistencies may occur. However, this was the ONLY solution given the limits of dtekv-run, 
  dtekv-upload and dtekv-download (without modifying these commands).

  Should be mentioned it is also not possible to run dtekv-upload or dtekv-download
  if an instance is running the program in the terminal already. Although that seems to be a
  limitation of JTAGD itself.
*/
char* cfg_ptr =                           (char*)               0x200000;
struct mandelbrot* cfg_mandeldata =       (struct mandelbrot*)  0x210000;
struct julia* cfg_juliadata =             (struct julia*)       0x220000;
struct sierpinski* cfg_sierpinskidata =   (struct sierpinski*)  0x230000;
struct datakey* cfg_datamap =             (struct datakey*)     0x240000;
char* image_buffer =                      (char*)               0x250000;

double sqrt(double x) {
    if (x == 0) {
        return 0;
    }

    double guess = x / 2;
    double tolerance = 1e-5; // Adjust tolerance as needed

    while (guess * guess > x + tolerance || guess * guess < x - tolerance) {
        guess = 0.5 * (guess + x / guess);
    }

    return guess;
}

void handle_interrupt(unsigned cause){}

// returns switch state by index [0,10)
int get_sw(char index) {
  volatile int *ptr = (volatile int*) 0x4000010;
  
  //bit mask by index
  return *ptr & (1 << index);
}

// returns first enabled switch by index [0,10) in natural order
int get_sw_i(void) {
  for (int i = 0; i < 10; i++) {
    if (get_sw(i)) {
      return i;
    }
  }
  return -1;
}

// returns 1 if button is pressed, else 0
int get_btn(void) {
  return *((volatile int*) 0x40000d0);
}

// returns fractal type by index from cache
char fetch_type(int index) {
  char type = cfg_datamap[index].type;
  if (type == 'M' || type == 'J' || type == 'S') {
    return type;
  }

  return '-';
}

// returns mandelbrot by index from cache, may be zero-initialized if absent 
struct mandelbrot fetch_mandelbrot(int index) {
  struct datakey key = cfg_datamap[index];
  struct mandelbrot data = *((struct mandelbrot*)key.ptr);
  return data;
}

// returns mandelbrot by index from cache, may be zero-initialized if absent 
struct julia fetch_julia(int index) {
  struct datakey key = cfg_datamap[index];
  struct julia data = *((struct julia*)key.ptr);
  return data;
}

// returns mandelbrot by index from cache, may be zero-initialized if absent 
struct sierpinski fetch_sierpinski(int index) {
  struct datakey key = cfg_datamap[index];
  struct sierpinski data = *((struct sierpinski*)key.ptr);
  return data;
}

void println(const char* s) {
  print(s);
  printc('\n');
}

void print_long(unsigned long long s) {
  unsigned int msb = (unsigned int) (s >> 32);
  if (msb != 0) {
    print_dec(msb);
  }
  print_dec((unsigned int) 0xffffffff & s);
}

void println_long(unsigned long long s) {
  print_long(s);
  printc('\n');
}

void println_dec(unsigned int s) {
  print_dec(s);
  printc('\n');
}

void println_hex32(unsigned int s) {
  print_hex32(s);
  printc('\n');
}

void printlnc(char s) {
  printc(s);
  printc('\n');
}

void print_double(double x) {
  //support signed values
  if (x < 0) {
    printc('-');
    x = -x;
  }

  // extract int part
  int int_part = (int) x;
  print_dec(int_part);

  if (x - int_part > 0.00001) {
    printc('.');
    double decimal = x - int_part;
    while (decimal > 0.00001) {
      decimal *= 10;
      int digit = (int) decimal;
      printc('0' + digit);
      decimal -= digit;
    }
  }
}

void println_double(double x) {
  print_double(x);
  printc('\n');
}

static unsigned int mcycleh = 0;
static unsigned int mcycle = 0;
static unsigned int minstreth = 0;
static unsigned int minstret = 0;
static unsigned int mhpmcounter3h= 0;
static unsigned int mhpmcounter3 = 0;
static unsigned int mhpmcounter4h= 0;
static unsigned int mhpmcounter4 = 0;
static unsigned int mhpmcounter5h= 0;
static unsigned int mhpmcounter5 = 0;
static unsigned int mhpmcounter6h= 0;
static unsigned int mhpmcounter6 = 0;
static unsigned int mhpmcounter7h= 0;
static unsigned int mhpmcounter7 = 0;
static unsigned int mhpmcounter8h= 0;
static unsigned int mhpmcounter8 = 0;
static unsigned int mhpmcounter9h= 0;
static unsigned int mhpmcounter9 = 0;

void reset_counters() {
  asm volatile ("csrw mcycleh, x0");
  asm volatile ("csrw mcycle, x0");
  asm volatile ("csrw minstreth, x0");
  asm volatile ("csrw minstret, x0");
  asm volatile ("csrw mhpmcounter3h, x0");
  asm volatile ("csrw mhpmcounter3, x0");
  asm volatile ("csrw mhpmcounter4h, x0");
  asm volatile ("csrw mhpmcounter4, x0");
  asm volatile ("csrw mhpmcounter5h, x0");
  asm volatile ("csrw mhpmcounter5, x0");
  asm volatile ("csrw mhpmcounter6h, x0");
  asm volatile ("csrw mhpmcounter6, x0");
  asm volatile ("csrw mhpmcounter7h, x0");
  asm volatile ("csrw mhpmcounter7, x0");
  asm volatile ("csrw mhpmcounter8h, x0");
  asm volatile ("csrw mhpmcounter8, x0");
  asm volatile ("csrw mhpmcounter9h, x0");
  asm volatile ("csrw mhpmcounter9, x0");
}

void read_counters() {

  asm ("csrr %0, mcycleh" : "=r"(mcycleh));
  asm ("csrr %0, mcycle" : "=r"(mcycle));
  asm ("csrr %0, minstreth" : "=r"(minstreth));
  asm ("csrr %0, minstret" : "=r"(minstret));
  asm ("csrr %0, mhpmcounter3h" : "=r"(mhpmcounter3h));
  asm ("csrr %0, mhpmcounter3" : "=r"(mhpmcounter3));
  asm ("csrr %0, mhpmcounter4h" : "=r"(mhpmcounter4h));
  asm ("csrr %0, mhpmcounter4" : "=r"(mhpmcounter4));
  asm ("csrr %0, mhpmcounter5h" : "=r"(mhpmcounter5h));
  asm ("csrr %0, mhpmcounter5" : "=r"(mhpmcounter5));
  asm ("csrr %0, mhpmcounter6h" : "=r"(mhpmcounter6h));
  asm ("csrr %0, mhpmcounter6" : "=r"(mhpmcounter6));
  asm ("csrr %0, mhpmcounter7h" : "=r"(mhpmcounter7h));
  asm ("csrr %0, mhpmcounter7" : "=r"(mhpmcounter7));
  asm ("csrr %0, mhpmcounter8h" : "=r"(mhpmcounter8h));
  asm ("csrr %0, mhpmcounter8" : "=r"(mhpmcounter8));
  asm ("csrr %0, mhpmcounter9h" : "=r"(mhpmcounter9h));
  asm ("csrr %0, mhpmcounter9" : "=r"(mhpmcounter9));
  print("mcycle     =");
  println_long((((unsigned long long)mcycleh) << 32) | mcycle);
  print("minstret   =");
  println_long((((unsigned long long)minstreth) << 32) | minstret);
  print("mhpmcounter3=");
  println_long((((unsigned long long)mhpmcounter3h) << 32) | mhpmcounter3);
  print("mhpmcounter4=");
  println_long((((unsigned long long)mhpmcounter4h) << 32) | mhpmcounter4);
  print("mhpmcounter5=");
  println_long((((unsigned long long)mhpmcounter5h) << 32) | mhpmcounter5);
  print("mhpmcounter6=");
  println_long((((unsigned long long)mhpmcounter6h) << 32) | mhpmcounter6);
  print("mhpmcounter7=");
  println_long((((unsigned long long)mhpmcounter7h) << 32) | mhpmcounter7);
  print("mhpmcounter8=");
  println_long((((unsigned long long)mhpmcounter8h) << 32) | mhpmcounter8);
  print("mhpmcounter9=");
  println_long((((unsigned long long)mhpmcounter9h) << 32) | mhpmcounter9);
}

// parses next signed int number in ascii, also moves the cursor to the terminating character of the token
signed int parse_int(char** ptr) {
  char* str = *ptr;
  signed int sign = 1;
  signed int val = 0;

  //support negative
  if (*str == '-') {
    sign = -1;
    str++;
  }

  while ('0' <= *str && *str <= '9') {
    val = val * 10 + (*str - '0');
    str++;
  }

  *ptr = str;
  return sign * val;
}

// parses next signed double number in ascii, also moves the cursor to the terminating character of the token
double parse_double(char** ptr) {
  char* str = *ptr;
  double val = 0.0;
  int sign = 1;

  //decimals
  int d_seen = 0;
  int d_index = 0;

  //support negative
  if (*str == '-') {
    sign = -1;
    str++;
  }

  while (*str != ';') {
    char c = *str;

    if (c >= '0' && c <= '9') {
      val = val * 10 + (c - '0');
      if (d_seen) {
        d_index++;
      }
    } else if (c == '.' && !d_seen) {
      d_seen = 1;
    } else {
      *ptr = str;
      return 0;
    }

    str++;
  }

  if (d_index > 0) {
    for (int i = 0; i < d_index; i++) {
      val /= 10;
    }
  }

  *ptr = str;
  return val*sign;
}

// parses next mandelbrot struct in ascii, also moves the cursor to the terminating character of the token
struct mandelbrot parse_mandelbrot(char** ptr) {
  double xmax = parse_double(ptr);
  (*ptr)++;
  double xmin = parse_double(ptr);
  (*ptr)++;
  double ymax = parse_double(ptr);
  (*ptr)++;
  double ymin = parse_double(ptr);
  (*ptr)++;
  int res = parse_int(ptr);
  (*ptr)++;
  struct mandelbrot data = {
    'M',
    xmax,
    xmin,
    ymax,
    ymin,
    res
  };
  return data;
}

// parses next julia struct in ascii, also moves the cursor to the terminating character of the token
struct julia parse_julia(char** ptr) {
  double xmax = parse_double(ptr);
  (*ptr)++;
  double xmin = parse_double(ptr);
  (*ptr)++;
  double ymax = parse_double(ptr);
  (*ptr)++;
  double ymin = parse_double(ptr);
  (*ptr)++;
  double real = parse_double(ptr);
  (*ptr)++;
  double imag = parse_double(ptr);
  (*ptr)++;
  int res = parse_int(ptr);
  (*ptr)++;
  struct julia data = {
    'J',
    xmax,
    xmin,
    ymax,
    ymin,
    real,
    imag,
    res
  };
  return data;
}


// parses next mandelbrot struct in ascii, also moves the cursor to the terminating character of the token
struct sierpinski parse_sierpinski(char** ptr) {
  int res = parse_int(ptr);
  (*ptr)++;
  struct sierpinski data = {
    'S',
    res
  };
  return data;
}

// reads configuration in ascii from the given pointer
void load_cfg(char* str) {
  int mandel_i = 0;
  int julia_i = 0;
  int sierpinski_i = 0;
  int datamap_i = 0;
  while (1) {
    char c0 = *str;
    str++;
    char c1 = *str;
    if (c1 == ';') {
      str++;
      if (c0 == 'M') {
        struct mandelbrot data = parse_mandelbrot(&str);
        cfg_mandeldata[mandel_i] = data;
        struct datakey key = {
          (struct mandelbrot*) &cfg_mandeldata[mandel_i],
          'M'
        };
        cfg_datamap[datamap_i] = key;

        datamap_i++;
        mandel_i++;
      } else if (c0 == 'J') {
        struct julia data = parse_julia(&str);
        cfg_juliadata[julia_i] = data;
        struct datakey key = {
          (struct julia*) &cfg_juliadata[julia_i],
          'J'
        };
        cfg_datamap[datamap_i] = key;

        datamap_i++;
        julia_i++;
      } else if (c0 == 'S') {
        struct sierpinski data = parse_sierpinski(&str);
        cfg_sierpinskidata[sierpinski_i] = data;
        struct datakey key = {
          (struct sierpinski*) &cfg_sierpinskidata[sierpinski_i],
          'S'
        };
        cfg_datamap[datamap_i] = key;

        datamap_i++;
        sierpinski_i++;
      }

    } else if (c0 == '\0' || c0 == '#') {
      break;
    }
  }
}

// writes small header (64x64) in PPM format (P6), will always be plain
void write_small_header(char** dst) {
  char* ptr = *dst;
  *ptr = 'P'; ptr++; *ptr = '6'; ptr++;
  *ptr = '\n'; ptr++;
  *ptr = '6'; ptr++; *ptr = '4'; ptr++;
  *ptr = '\n'; ptr++;
  *ptr = '6'; ptr++; *ptr = '4'; ptr++;
  *ptr = '\n'; ptr++;
  *ptr = '2'; ptr++; *ptr = '5'; ptr++; *ptr = '5'; ptr++;
  *ptr = '\n'; ptr++;
  *dst = ptr;
} 

// writes medium header (128x128) in PPM format (P6), will always be plain
void write_medium_header(char** dst) {
  char* ptr = *dst;
  *ptr = 'P'; ptr++; *ptr = '6'; ptr++;
  *ptr = '\n'; ptr++; 
  *ptr = '1'; ptr++; *ptr = '2'; ptr++; *ptr = '8'; ptr++;
  *ptr = '\n'; ptr++;
  *ptr = '1'; ptr++; *ptr = '2'; ptr++; *ptr = '8'; ptr++;
  *ptr = '\n'; ptr++;
  *ptr = '2'; ptr++; *ptr = '5'; ptr++; *ptr = '5'; ptr++;
  *ptr = '\n'; ptr++;
  *dst = ptr;
}

// writes large header (256x256) in PPM format (P6), will always be plain
void write_large_header(char** dst) {
  char* ptr = *dst;
  *ptr = 'P'; ptr++; *ptr = '6'; ptr++;
  *ptr = '\n'; ptr++;
  *ptr = '2'; ptr++; *ptr = '5'; ptr++; *ptr = '6'; ptr++;
  *ptr = '\n'; ptr++;
  *ptr = '2'; ptr++; *ptr = '5'; ptr++; *ptr = '6'; ptr++;
  *ptr = '\n'; ptr++;
  *ptr = '2'; ptr++; *ptr = '5'; ptr++; *ptr = '5'; ptr++;
  *ptr = '\n'; ptr++;
  *dst = ptr;
}

/*
  Writes mandelbrot data.

  Mandelbrot sets are defined as z_n = z_n-1 + c,
  where z_0 = 0+0i and c is the given (x,y) pixel,
  where we use y to represnet the imaginary axis
  and we use x to represent the real axis.
  
  We then iterate up to z_255, if the value
  converges (that is if it cycles around), then we
  paint it black, else we paint it some other color
  depending on how quickly it diverges (that is if it
  approaches infinity).
*/
int write_mandelbrot_data(struct mandelbrot data, char* dst, int* size) {
  reset_counters();
  int sz = (int) dst;
  if (data.res == 64) {
    write_small_header(&dst);
  } else if (data.res == 128) {
    write_medium_header(&dst);
  } else if (data.res == 256) {
    write_large_header(&dst);
  } else {
    print("[SEVERE] Bad resolution '");
    print_dec(data.res);
    println("', only resolutions 64, 128 and 256 are allowed!");
    return 0;
  }
  print("[INFO] Writing Mandelbrot with resolution '");
  print_dec(data.res);
  printlnc('\'');

  // how many times we check if a value converges or diverges
  const int max_it_count = 256;

  double xmax = data.xmax;
  double xmin = data.xmin;
  double ymax = data.ymax;
  double ymin = data.ymin;
  int res = data.res;

  // dx and dy
  double step_x = (xmax-xmin)/res;
  double step_y = (ymax-ymin)/res;

  // cached stack variables
  int it_count;
  int i, j;
  double x, y;  
  double u, v, u2, v2;
  
  for (j = 0; j < res; j++) {
    //calculate y-coord value
    y = ymax - (j * step_y);

    //print new progress
    printc('\r');
    print_double(((double)j*100)/res);
    printlnc('%');


    for(i = 0; i < res; i++) {
    //calculate x-coord value
      x = xmin + i * step_x;

      u = 0.0;
      v = 0.0;
      u2 = 0;
      v2 = 0;

      //inspiration from https://en.wikipedia.org/wiki/Plotting_algorithms_for_the_Mandelbrot_set#Optimized_escape_time_algorithms
      for (it_count = 1; max_it_count > it_count && (u2 + v2 < 4.0); it_count++) {
        v = 2 * u * v + y;
        u = u2 - v2 + x;
        u2 = u * u;
        v2 = v * v;
      }

      //paint black if value does not diverge
      if (max_it_count <= it_count) {
        *dst = 0; dst++;
        *dst = 0; dst++;
        *dst = 0; dst++;
      }
      //paint some other color if value diverges
      //we can use 'it_count' to represent how quickly said value diverges
      else {
        *dst = (it_count >> 2) % 256; dst++;
        *dst = it_count % 256; dst++;
        *dst = (it_count + 10) % 256; dst++;
      }
    }
  }

  *size = (int) dst - sz;
  read_counters();
  return 1;
}

/*
  Writes julia data.

  Julia sets are defined as z_n = z_n-1 + c,
  where z_0 = (x,y) of the given (x,y) pixel,
  where we use y to represnet the imaginary axis
  and we use x to represent the real axis. Then c 
  is just some starting number c = a + bi.
  
  We then iterate up to z_255 and here we paint
  the pixel differently depending on how quickly
  it increases and eventually diverges or is cyclic.
*/
int write_julia_data(struct julia data, char* dst, int* size) {
  int sz = (int) dst;
  if (data.res == 64) {
    write_small_header(&dst);
  } else if (data.res == 128) {
    write_medium_header(&dst);
  } else if (data.res == 256) {
    write_large_header(&dst);
  } else {
    print("[SEVERE] Bad resolution '");
    print_dec(data.res);
    println("', only resolutions 64, 128 and 256 are allowed!");
    return 0;
  }
  print("[INFO] Writing Julia with resolution '");
  print_dec(data.res);
  printlnc('\'');

  // how many times we check if a value converges or diverges
  const int max_it_count = 256;

  double xmax = data.xmax;
  double xmin = data.xmin;
  double ymax = data.ymax;
  double ymin = data.ymin;
  int res = data.res;

  // dx and dy
  double step_x = (xmax-xmin)/res;
  double step_y = (ymax-ymin)/res;

  double cx = data.real;
  double cy = data.imag;

  // cached stack variables
  int it_count;
  int i, j;
  double x, y;  
  double u, v, u2, v2;
  
  for (j = 0; j < res; j++) {
    //calculate y-coord value
    y = ymax - (j * step_y);

    //print new progress
    printc('\r');
    print_double(((double)j*100)/res);
    printlnc('%');


    for(i = 0; i < res; i++) {
    //calculate x-coord value
      x = xmin + i * step_x;

      u = x;
      v = y;
      u2 = u*u;
      v2 = v*v;

      for (it_count = 1; max_it_count > it_count && (u2 + v2 < 4.0); it_count++) {
        v = 2*u*v + cy;
        u = u2 - v2 + cx;
        v2 = v * v;
        u2 = u * u;
      }

      *dst = 255 - (it_count % 256); dst++;
      *dst = 255 - (it_count*2 % 256); dst++;
      *dst = 255 - (it_count*4 % 256); dst++;
    }
  }

  *size = (int) dst - sz;
  return 1;
}


int write_sierpinski_data(struct sierpinski data, char* dst, int* size) {
  println("[ERROR] Sierpinski is not yet implemented");
  return 0;
}

int process_image(int index, char* dst, int* size) {
  char type = fetch_type(index);
  if (type == '-') {
    print("[WARNING] Cannot process due to bad type at switch '");
    print_dec(index);
    printc('\'');
    println(", this could be due to an incorrect type or missing.");
    return 0;
  } else if (type == 'M') {
    return write_mandelbrot_data(fetch_mandelbrot(index), dst, size);
  } else if (type == 'J') {
    return write_julia_data(fetch_julia(index), dst, size);
  } else if (type == 'S') {
    return write_sierpinski_data(fetch_sierpinski(index), dst, size);
  }else {
    //should never happen since we cover all cases already
    println("[ERROR] How did we get here?");
    return 0;
  }
}

int main() {
  print("[INFO] Config buffer address: ");
  println_hex32((int) cfg_ptr);

  println("[INFO] Loading config...");

  load_cfg(cfg_ptr);

  println("[INFO] Config loaded!");

  println("[INFO] Select a switch and press the BUTTON to generate an image!");

  while (1) {
    if (get_btn()) {
      int i = get_sw_i();
      if (i < 0 || i > 9) { 
        continue;
      }

      print("[INFO] Generating image from switch '");
      print_dec(i);
      printlnc('\'');

      int size = 0;

      print("[INFO] Initiating writing data to '");
      print_hex32((int)image_buffer);
      println("'!");
      
      if (process_image(i,image_buffer,&size)) {
        print("[INFO] Finished writing data to '");
        print_hex32((int)image_buffer);
        print("' with size of '");
        print_hex32(size);
        println("'-bytes!");
      }
    }
  }
}
