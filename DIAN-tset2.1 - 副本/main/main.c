#include <stdio.h>
#include "driver/i2c.h"
 
 
float accel_scale=16384;
float gyro_scale=131;
float calibration_array[6]={0};
 
void write_register(uint8_t *data){
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 0x68<<1|0,1);
    i2c_master_write(cmd, data, sizeof(data), 1);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(0, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

}
 
uint8_t read_register(uint8_t reg){
    uint8_t data;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 0x68<<1|0,1);
    i2c_master_write_byte(cmd, reg,1);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 0x68<<1|1,1);
    i2c_master_read_byte(cmd, &data,1);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(0, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return data;
}
 
void I2C_init(void){
    i2c_config_t conf={
        .mode=I2C_MODE_MASTER,
        .sda_io_num=17,
        .scl_io_num=18,
        .sda_pullup_en=1,
        .scl_pullup_en=1,
        .master.clk_speed=100000
    };
    i2c_param_config(0,&conf);
    i2c_driver_install(0,I2C_MODE_MASTER,0,0,0);
}
 
void MPU6050_init(void){    
    

    uint8_t CONF1[]={0x6B, 0x80};
    uint8_t CONF2[]={0x6B, 0x00};
    uint8_t CONF3[]={0x1B, 0x00};
    uint8_t CONF4[]={0x1C, 0x00};
    uint8_t CONF5[]={0x38, 0x00};
    uint8_t CONF6[]={0x6A, 0x00};
    uint8_t CONF7[]={0x23, 0x00};
    uint8_t CONF8[]={0x19, 0x63};
    uint8_t CONF9[]={0x1A, 0x13};
    uint8_t CONF10[]={0x6B, 0x01};
    uint8_t CONF11[]={0x6C, 0x00};


    
    write_register(CONF1);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    write_register(CONF2);
    write_register(CONF3);
    write_register(CONF4);
    write_register(CONF5);
    write_register(CONF6);
    write_register(CONF7);
    write_register(CONF8);
    write_register(CONF9);
    write_register(CONF10);
    write_register(CONF11);
 
}
 
struct AccelGyroData_t
{
    float accelX;
    float accelY;
    float accelZ;
    float gyroX;
    float gyroY;
    float gyroZ;
};
 
struct AccelGyroData_t get_raw_mpu6050_data(){
    uint8_t accelGyroRegArray[]={59,60,61,62,63,64,67,68,69,70,71,72};
    float AccelGyroDataArray[6];
    AccelGyroDataArray[0]=(read_register(accelGyroRegArray[0])<<8)+read_register(accelGyroRegArray[1]);
    AccelGyroDataArray[1]=(read_register(accelGyroRegArray[2])<<8)+read_register(accelGyroRegArray[3]);
    AccelGyroDataArray[2]=(read_register(accelGyroRegArray[4])<<8)+read_register(accelGyroRegArray[5]);
    AccelGyroDataArray[3]=(read_register(accelGyroRegArray[6])<<8)+read_register(accelGyroRegArray[7]);
    AccelGyroDataArray[4]=(read_register(accelGyroRegArray[8])<<8)+read_register(accelGyroRegArray[9]);
    AccelGyroDataArray[5]=(read_register(accelGyroRegArray[10])<<8)+read_register(accelGyroRegArray[11]);
    for(int i=0;i<12;i+=2){
        if(i<6){
            if(AccelGyroDataArray[i/2]>32767) AccelGyroDataArray[i/2]=AccelGyroDataArray[i/2]/accel_scale-4;
            else AccelGyroDataArray[i/2]=AccelGyroDataArray[i/2]/accel_scale;
        }
        else{
            if(AccelGyroDataArray[i/2]>32767) AccelGyroDataArray[i/2]=AccelGyroDataArray[i/2]/gyro_scale-500;
            else AccelGyroDataArray[i/2]=AccelGyroDataArray[i/2]/gyro_scale;
        };
    };
    struct AccelGyroData_t AccelGyroData={AccelGyroDataArray[0],AccelGyroDataArray[1],AccelGyroDataArray[2],AccelGyroDataArray[3],AccelGyroDataArray[4],AccelGyroDataArray[5]};
    return AccelGyroData;
}
 
void get_mpu6050_calibration(float calibration_array[]){
    struct AccelGyroData_t accelGyroData;
    for(int i=0;i<20;i++){
        accelGyroData=get_raw_mpu6050_data();
        calibration_array[0]+=accelGyroData.accelX;
        calibration_array[1]+=accelGyroData.accelY;
        calibration_array[2]+=accelGyroData.accelZ;
        calibration_array[3]+=accelGyroData.gyroX;
        calibration_array[4]+=accelGyroData.gyroY;
        calibration_array[5]+=accelGyroData.gyroZ;
        vTaskDelay(5/portTICK_PERIOD_MS);
    };
    for(int i=0;i<6;i++){
        calibration_array[i]/=20;
        if(i==2) calibration_array[i]=1-calibration_array[i];
        else calibration_array[i]=0-calibration_array[i];
    };
    printf("calibration succees\n");
}
 
struct AccelGyroData_t get_mpu6050_data(float calibration_array[]){
    struct AccelGyroData_t accelGyroData=get_raw_mpu6050_data();
    accelGyroData.accelX+=calibration_array[0];
    accelGyroData.accelY+=calibration_array[1];
    accelGyroData.accelZ+=calibration_array[2];
    accelGyroData.gyroX+=calibration_array[3];
    accelGyroData.gyroY+=calibration_array[4];
    accelGyroData.gyroZ+=calibration_array[5];
    return accelGyroData;
}

void app_main(void)
{
    vTaskDelay(1000/portTICK_PERIOD_MS);
    I2C_init();
    vTaskDelay(1000/portTICK_PERIOD_MS);
    MPU6050_init();
    vTaskDelay(1000/portTICK_PERIOD_MS);
    get_mpu6050_calibration(calibration_array);
    struct AccelGyroData_t data;
    while(1){
        data= get_mpu6050_data(calibration_array);
        printf("%f %f %f %f %f %f\n",data.accelX,data.accelY,data.accelZ,data.gyroX,data.gyroY,data.gyroZ);
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}
    
 