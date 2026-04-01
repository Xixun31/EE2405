#include "mbed.h"

using namespace std::chrono;

Timer t1, t2, t3;

int main()
{
   t1.start();
   exp(100);
   t1.stop();

   t2.start();
   exp(200);
   t2.stop();

   t3.start();
   exp(300);
   t3.stop();

   auto s = chrono::duration_cast<chrono::seconds>(t1.elapsed_time()).count();
   auto ms = chrono::duration_cast<chrono::milliseconds>(t1.elapsed_time()).count();
   auto us = t1.elapsed_time().count();

   printf ("Timer1 time: %llu s\n", s);
   printf ("Timer1 time: %llu ms\n", ms);
   printf ("Timer1 time: %llu us\n", us);

   s = chrono::duration_cast<chrono::seconds>(t2.elapsed_time()).count();
   ms = chrono::duration_cast<chrono::milliseconds>(t2.elapsed_time()).count();
   us = t2.elapsed_time().count();

   printf ("Timer2 time: %llu s\n", s);
   printf ("Timer2 time: %llu ms\n", ms);
   printf ("Timer2 time: %llu us\n", us);

   s = chrono::duration_cast<chrono::seconds>(t3.elapsed_time()).count();
   ms = chrono::duration_cast<chrono::milliseconds>(t3.elapsed_time()).count();
   us = t3.elapsed_time().count();

   printf ("Timer3 time: %llu s\n", s);
   printf ("Timer3 time: %llu ms\n", ms);
   printf ("Timer3 time: %llu us\n", us);
}

