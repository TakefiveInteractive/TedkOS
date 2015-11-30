#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sha1.h"  /* from RFC3174 */

int32_t ece391_getargs(void *, int32_t);

/* Test for nonzero error code, exit with message if so */
void test(int err, const char* msg) {
  if (err) {
    fprintf(stderr, "%s: error %d\n", msg, err);
    exit(1);
  }
}

int main() {
  int i, len;
  FILE* f;
  unsigned char buf[20] = {0};
  SHA1Context sha;

  unsigned char filename[90];
  memset(filename, 0, 90);
  ece391_getargs(filename, 90);

  printf("file is: %s\n", filename);

  unsigned char *inputbuf = malloc(10 * 1024 * 1024);
  f = fopen(filename, "rb");
  len = fread(inputbuf, 1, 10 * 1024 * 1024, f);

  printf("read file\n");

  test(SHA1Reset(&sha), "SHA1Reset");
  test(SHA1Input(&sha, inputbuf, len), "SHA1Input");
  test(SHA1Result(&sha, buf), "SHA1Result");

  printf("Key hash =");
  for (i=0; i<20; ++i)
    printf(" %02X", buf[i]);
  printf("\n");
  return 0;
}

