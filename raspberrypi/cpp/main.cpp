#include <iostream>
#include <errno.h>
#include "rgb_matrix/MatrixLed.h"


using namespace std;

int main()
{
    MatrixLed testI2c = MatrixLed(0x74);

    testI2c.Open();
    testI2c.Init();
    /*
    for(int i = 0; i < 8; i++){
        for(int j = 0; j < 8; j++){
            __u8 dot[2] = {(__u8)i,(__u8)j};
            testI2c.DrawPoint(dot, 243, 81, 0);                
        }
    }
    */
   __u8 line[4] = {0,1,7,1};
   testI2c.DrawLine(line,50,100,150);


    
    testI2c.ShowImage(); 
//    __u8 heart[] = {0x00, 0x66, 0xff, 0xff, 0x7e, 0x3c, 0x18, 0x00};
//    testI2c.ShowHex(heart, 100, 100, 0);
}
