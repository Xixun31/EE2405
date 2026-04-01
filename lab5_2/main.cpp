#include "mbed.h"

using namespace std::chrono;

Timer t;

void showt(){
    auto ms = chrono::duration_cast<chrono::milliseconds>(t.elapsed_time()).count();
    printf ("Timer time: %llu ms\n", ms);
}

int main()
{
   t.start();
   EventQueue queue;

   queue.call_every(30ms, showt);

   queue.dispatch_forever();
}