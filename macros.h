
#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)>(y)?(x):(y))
#define ABS(x) (((x)<0)?(-(x)):(x))

#define ReadPixel(lpscreen,offset,pixel) (pixel)=(*((LPDWORD)((DWORD)(lpscreen) + (DWORD)(offset))))
#define WritePixel(lpscreen,offset,colour) (*((LPDWORD)((DWORD)(lpscreen) + (DWORD)(offset))))=(colour)

#define SET(x,y) ((x)|=(y))
#define TOGGLE(x,y) ((x)^=(y))
#define CHECK(x,y) ((x)&(y))
#define UNSET(x,y) (SET((x),(y)),TOGGLE((x),(y)))

#define    KeyDown(data, n)    (((data[(n)]) & 0x80) ? true : false)
#define    KeyUp(data, n)    (((data[(n)]) & 0x80) ? false : true)

#define soundfloattoint(x) ((x)!=0?((int)(log10(x)*1000.0)):-10000)

// convenient macro for releasing interfaces
#define HELPER_RELEASE(x)   if (x != NULL) \
                            { \
                                x->Release(); \
                                x = NULL; \
                            }


enum { VK_0 = 0x30, VK_1, VK_2, VK_3, VK_4, VK_5, VK_6, VK_7, VK_8, VK_9 };
enum { VK_A = 0x41, VK_B, VK_C, VK_D, VK_E, VK_F, VK_G, VK_H, VK_I, VK_J, VK_K, VK_L, VK_M, VK_N, VK_O, VK_P, VK_Q, VK_R, VK_S, VK_T, VK_U, VK_V, VK_W, VK_X, VK_Y, VK_Z};
