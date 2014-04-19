typedef struct Pen Pen;

struct Pen
{
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char alpha;
    double radius;
};

int PenInit
(
    Pen *pen,
    unsigned char r,
    unsigned char g,
    unsigned char b,
    unsigned char a,
    double radius
);

# define PEN_BLACK 0, 0, 0, 255
