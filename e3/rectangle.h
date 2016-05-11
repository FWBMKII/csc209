#define NAMESIZE 20

struct point {
    int x;
    int y;
};

struct rectangle {
    struct point upperleft;
    struct point lowerright;
    char label[NAMESIZE + 1];
};

struct point *create_point(int x, int y);

struct rectangle *create_rectangle(struct point ul, struct point lr, 
                                   char *label);

int area1(struct rectangle r);
int area2(struct rectangle *r);
void change_label(struct rectangle *r, char *newlabel);
void broken_change_label(struct rectangle r, char * newlabel);
void print_rectangle(struct rectangle *r);
