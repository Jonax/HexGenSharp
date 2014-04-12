#ifndef HG14_VENDOR_STB_IMAGE_WRITE_H
#   define HG14_VENDOR_STB_IMAGE_WRITE_H

int stbi_write_png(char const *filename, int w, int h, int comp, const void *data, int stride_in_bytes);
int stbi_write_bmp(char const *filename, int w, int h, int comp, const void *data);
int stbi_write_tga(char const *filename, int w, int h, int comp, const void *data);

#endif

