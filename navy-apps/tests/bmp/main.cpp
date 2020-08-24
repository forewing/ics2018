#include <assert.h>
#include <stdlib.h>
#include <ndl.h>

#include <stdio.h>

void pass(){
  static int i = 0;
  i++;
  printf("PASS %d\n", i);
}

int main() {
  NDL_Bitmap *bmp = (NDL_Bitmap*)malloc(sizeof(NDL_Bitmap));
  pass();
  NDL_LoadBitmap(bmp, "/share/pictures/projectn.bmp");
  pass();
  assert(bmp->pixels);
  pass();
  NDL_OpenDisplay(bmp->w, bmp->h);
  pass(); 
  NDL_DrawRect(bmp->pixels, 0, 0, bmp->w, bmp->h);
  pass();
  NDL_Render();
  pass();
  NDL_CloseDisplay();
  pass();
  while (1);
  return 0;
}
