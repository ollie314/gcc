/* { dg-options "-O2" } */

int arr[4][4];

void
foo (int x, int y)
{
  arr[0][1] = x;
  arr[1][0] = y;
  arr[2][0] = x;
  arr[1][1] = y;
  arr[0][2] = x;
  arr[0][3] = y;
  arr[1][2] = x;
  arr[2][1] = y;
  arr[3][0] = x;
  arr[3][1] = y;
  arr[2][2] = x;
  arr[1][3] = y;
  arr[2][3] = x;
  arr[3][2] = y;
}

/* { dg-final { scan-assembler-times "stp\tw\[0-9\]+, w\[0-9\]" 7 } } */
