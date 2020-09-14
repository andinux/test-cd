#include <stdio.h>
int main(int argc, const char* argv[]) {
   const char* str = "";
   if (argc > 1) {
      str = argv[1];
   }
   // printf() displays the string inside quotation
   printf("Hello, World! %s\n", str);
   return argc-1;
}
