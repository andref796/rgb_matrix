//
// Created by hyde on 19/06/22.
//
#include <unistd.h>				//Needed for I2C port
#include <iostream>
#include <fcntl.h>				//Needed for I2C port
#include <sys/ioctl.h>			//Needed for I2C port
extern "C" {
    #include <linux/i2c-dev.h>		//Needed for I2C port
    #include <i2c/smbus.h>
}
#include "MatrixLed.h"
#include <math.h>

bool MatrixLed::Open() {
    if( (_fd = open("/dev/i2c-1",O_RDWR)) < 0 ){
        return false;
    }
    if(ioctl(_fd, I2C_SLAVE, _addr) < 0){
        return false;
    }
    return true;
}

bool MatrixLed::Close() {
    close(_fd);
    return true;
}

bool MatrixLed::Init(){
    this->WriteCmd(CONFIGURE_CMD_PAGE, FUNCTION_PAGE);
    this->WriteCmd(SW_SHUT_DOWN_REG, 0x0);
    this->WriteCmd(PICTURE_DISPLAY_REG, 0x10);
    this->WriteCmd(STAGGERED_DELAY_REG, ((MSKSTD4 & CONST_STD_GROUP4)|(MSKSTD3 & CONST_STD_GROUP3)|(MSKSTD2 & CONST_STD_GROUP2)|(MSKSTD1 & CONST_STD_GROUP1)));
    this->WriteCmd(SLEW_RATE_CTL_REG, 0x1);
    this->WriteCmd(VAF_CTL_REG, (MSKVAF2 | MSKVAF1));
    this->WriteCmd(VAF_CTL_REG2, (MSKFORCEVAFCTL_DISABLE | MSKFORCEVAFTIME_CONST | MSKVAF3));
    this->WriteCmd(CURRENT_CTL_REG, (MSKCURRENT_CTL_EN | CONST_CURRENT_STEP_20mA));
    this->WriteCmd(CONFIGURE_CMD_PAGE, FRAME1_PAGE);
    this->Write_NData(0x00,0x00,0xB3);
    this->WriteCmd(CONFIGURE_CMD_PAGE, FRAME2_PAGE);
    this->Write_NData(0x00,0xB3,0x00);
    this->WriteCmd(CONFIGURE_CMD_PAGE, LED_VAF_PAGE);
    this->WriteArrayData(0x00,tabLED_Type3Vaf,0x40);
    this->WriteCmd(CONFIGURE_CMD_PAGE, FUNCTION_PAGE);
    this->WriteCmd(SW_SHUT_DOWN_REG, 0x1);
    this->WriteCmd(CONFIGURE_CMD_PAGE, FRAME1_PAGE);
    this->Write_NData(0x00,0xFF,0x10);
    this->Write_NData(0x20,0x00,0x80);
    this->WriteCmd(CONFIGURE_CMD_PAGE, FRAME2_PAGE);
    this->Write_NData(0x00, 0xFF,0x10);
    this->Write_NData(0x20,0x00,0x80);
    return true;
}

bool MatrixLed::ShowImage() {
    __u8 revert_image[3][64] = {};
    
    for(int i = 0; i<64;i++){
      revert_image[0][i] = this->rgb_test[i][1];
      revert_image[1][i] = this->rgb_test[i][0];
      revert_image[2][i] = this->rgb_test[i][2];
    }

    __u8 reg = 0x20;
    __u8 empty = 0;
    __u8 pos = 0;

    for (int i = 0; i < 15; i++)
    {
        if (i == 0){
            this->WriteCmd(CONFIGURE_CMD_PAGE, FRAME1_PAGE);
        }
        else if (reg == 0x20)
        {
            this->WriteCmd(CONFIGURE_CMD_PAGE, FRAME2_PAGE);
        }
    

        __u8 color = i % 3;
        __u8 pos_data = pos * 14;
        int data_left = 64 - (pos+1) * 14;

        __u8 data_lenth = 0;
        if (data_left > 0) {
          data_lenth = 14;
        }
        else{
          data_lenth = 8;
          pos_data = 56;
        }
        __u8 data[data_lenth] = {};
        
        for(int i = 0; i < data_lenth;i++){
            data[i] = revert_image[color][pos_data+i];

        }
        
        __u8 insert_data[data_lenth+2] = {0};
        for (int i=0; i<(data_lenth+2); i++)
        {
          if (i < empty){           
            insert_data[i] = data[i];
          }
          else if (i==empty || i== (empty+1)){
            insert_data[i] = 0;
            }
          
          else if (i > (empty+1)){
            insert_data[i] = data[i-2];
          }
        }
        if (data_lenth == 8){
          insert_data[8] = 0;
          insert_data[9] = 0;
          
        }

        i2c_smbus_write_i2c_block_data(_fd, reg, data_lenth + 2, insert_data);
        if (color == 2){
            empty += 3;
            pos += 1;
        }
        reg += 0x10;
        if (reg == 0xA0){
            reg = 0x20;
        }
    
    }

    return true;
}

bool MatrixLed::DrawPoint(__u8 coor[2], __u8 R, __u8 G, __u8 B){
    this->rgb_test[8*coor[1]+coor[0]][0] = B;
    this->rgb_test[8*coor[1]+coor[0]][1] = G;
    this->rgb_test[8*coor[1]+coor[0]][2] = R;
    return true;
}

bool MatrixLed::DrawLine(const __u8 coor[4], __u8 R,__u8 G,__u8 B){
    if (coor[0] == coor[2]){
      for (int i = coor[1]*8+coor[0]; i<(coor[3]+1)*8; i+=8){
          this->rgb_test[i][0] = B;
          this->rgb_test[i][1] = G;
          this->rgb_test[i][2] = R;   
	  }
  }	  
  else if (coor[1] == coor[3]){
      for (int i = coor[1]*8+coor[0]; i<coor[1]*8+coor[2]+1; i++){
          this->rgb_test[i][0] = B;
          this->rgb_test[i][1] = G;
          this->rgb_test[i][2] = R;   
	  }	  
  }
  return true;
};

bool MatrixLed::ShowHex(const __u8 * hex, __u8 R, __u8 G, __u8 B) {
    char temp;
    int i, j;
    //unsigned char chrtemp[24] = {0};
    unsigned char x,y,temp2;
    unsigned char chrtemp2[24] = {0};

    y = 0;
    temp2 = 0;
    for(int dex = 0;dex<8;dex++){
        for(x=0;x<8;x++){

            if((hex[x]<<dex) & 0x80)
            {
                temp2 = 1*pow(2,x) + temp2;
                chrtemp2[y] = temp2;
            }
            else{
                temp2 = 0*pow(2,x) + temp2;
                chrtemp2[y] = temp2;
            }
        }
        temp2 = 0;
        y++;
    }

    for(i = 0;i < 8;i++)
    {  
        temp = chrtemp2[i];
        std::cout << temp << std::endl;
        for(j = 7;j > -1;j--)
        {
            if(temp & 0x80)
            {
                rgb_test[8*j+i][0] = B;
                rgb_test[8*j+i][1] = G;
                rgb_test[8*j+i][2] = R;
            }
            else
            {
                rgb_test[8*j+i][0] = 0;
                rgb_test[8*j+i][1] = 0;
                rgb_test[8*j+i][2] = 0;
            }
            temp = temp << 1;
        }
    }
    this->ShowImage();
    return true;
}


MatrixLed::~MatrixLed() {
    close(_fd);
}


bool MatrixLed::WriteCmd( const __u8 cmd, const __u8 reg){
    i2c_smbus_write_byte_data(this->_fd, cmd, reg);
    return true;
};

bool MatrixLed::Write_NData(const __u8 startRegister, const __u8 data, const __u8 length){
    for (__u8 i = startRegister; i < length ; i++) {
        this->WriteCmd(i, data);
    }
    return true;
};

bool MatrixLed::WriteArrayData(const __u8 startRegister, const __u8 data[64], const __u8 length){
    for (__u8 i = startRegister; i < length ; i++) {
        this->WriteCmd(i, data[i]);
    }
    return true;    
};