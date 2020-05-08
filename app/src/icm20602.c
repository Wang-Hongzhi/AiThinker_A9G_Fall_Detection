#include "api_os.h"
#include "api_debug.h"
#include "api_event.h"
#include "api_hal_i2c.h"
#include "icm20602.h"

uint8_t ICM20602_Init(void)
{
    uint8_t res;
    I2C_Config_t config;

    config.freq = I2C_FREQ_400K;
    I2C_Init(I2C2, config);

    res = ICM_Read_Byte(WHO_AM_I); //读取ICM20602的ID
    if (res != ICM20602_ID)        //器件ID正确
    {
        Trace(1, "ID=%#X\r\n", res);
        Trace(1, "ICM20602 is fail!\n");
        return 0;
    }
    else
    {
        Trace(1, "ID=%#X\r\n", res);
        Trace(1, "ICM20602 is OK!\n");
    }

    res = 0;
    ICM_Write_Byte(ICM_PWR_MGMT1_REG, 0X80); //复位
    OS_Sleep(100);                            //延时100ms
    ICM_Write_Byte(ICM_PWR_MGMT1_REG, 0X00); //唤醒
    OS_Sleep(100);                            //延时100ms

    ICM_Set_Gyro_Fsr(3);                     //陀螺仪传感器,±2000dps
    ICM_Set_Accel_Fsr(1);                    //加速度传感器,±4g
    ICM_Set_Rate(1000);                      //设置采样率1000Hz
    ICM_Write_Byte(ICM_CFG_REG, 0x02);       //设置数字低通滤波器   98hz
    ICM_Write_Byte(ICM_INT_EN_REG, 0X00);    //关闭所有中断
    ICM_Write_Byte(ICM_USER_CTRL_REG, 0X00); //I2C主模式关闭
    ICM_Write_Byte(ICM_PWR_MGMT1_REG, 0X01); //设置CLKSEL,PLL X轴为参考
    ICM_Write_Byte(ICM_PWR_MGMT2_REG, 0X00); //加速度与陀螺仪都工作

    return 1;
}

//设置ICM20602陀螺仪传感器满量程范围
//fsr:0,±250dps;1,±500dps;2,±1000dps;3,±2000dps
//返回值:0,设置成功
//    其他,设置失败
void ICM_Set_Gyro_Fsr(uint8_t fsr)
{
    ICM_Write_Byte(ICM_GYRO_CFG_REG, fsr << 3); //设置陀螺仪满量程范围
}
//设置ICM20602加速度传感器满量程范围
//fsr:0,±2g;1,±4g;2,±8g;3,±16g
//返回值:0,设置成功
//    其他,设置失败
void ICM_Set_Accel_Fsr(uint8_t fsr)
{
    ICM_Write_Byte(ICM_ACCEL_CFG_REG, fsr << 3); //设置加速度传感器满量程范围
}

//设置ICM20602的数字低通滤波器
//lpf:数字低通滤波频率(Hz)
//返回值:0,设置成功
//    其他,设置失败
void ICM_Set_LPF(uint16_t lpf)
{
    uint8_t data = 0;
    if (lpf >= 188)
        data = 1;
    else if (lpf >= 98)
        data = 2;
    else if (lpf >= 42)
        data = 3;
    else if (lpf >= 20)
        data = 4;
    else if (lpf >= 10)
        data = 5;
    else
        data = 6;
    ICM_Write_Byte(ICM_CFG_REG, data); //设置数字低通滤波器
}

//设置ICM20602的采样率(假定Fs=1KHz)
//rate:4~1000(Hz)
//返回值:0,设置成功
//    其他,设置失败
void ICM_Set_Rate(uint16_t rate)
{
    uint8_t data;
    if (rate > 1000)
        rate = 1000;
    if (rate < 4)
        rate = 4;
    data = 1000 / rate - 1;
    ICM_Write_Byte(ICM_SAMPLE_RATE_REG, data); //设置数字低通滤波器
    ICM_Set_LPF(rate / 2);                     //自动设置LPF为采样率的一半
}

//得到温度值
//返回值:温度值(扩大了100倍)
short ICM_Get_Temperature(void)
{
    uint8_t buf[3];
    short raw;
    float temp;
    ICM_Read_Len(ICM_TEMP_OUTH_REG, 2, buf);
    raw = ((uint16_t)buf[0] << 8) | buf[1];
    temp = 21 + ((double)raw) / 333.87;
    return (short)temp * 100;
}
//得到陀螺仪值(原始值)
//gx,gy,gz:陀螺仪x,y,z轴的原始读数(带符号)
//返回值:0,成功
//    其他,错误代码
void ICM_Get_Gyroscope(short *gx, short *gy, short *gz)
{
    uint8_t buf[7];
    ICM_Read_Len(ICM_GYRO_XOUTH_REG, 6, buf);

    *gx = ((uint16_t)buf[1] << 8) | buf[2];
    *gy = ((uint16_t)buf[3] << 8) | buf[4];
    *gz = ((uint16_t)buf[5] << 8) | buf[6];
}
//得到加速度值(原始值)
//gx,gy,gz:陀螺仪x,y,z轴的原始读数(带符号)
//返回值:0,成功
//    其他,错误代码
void ICM_Get_Accelerometer(short *ax, short *ay, short *az)
{
    uint8_t buf[7];
    ICM_Read_Len(ICM_ACCEL_XOUTH_REG, 6, buf);

    *ax = ((uint16_t)buf[1] << 8) | buf[2];
    *ay = ((uint16_t)buf[3] << 8) | buf[4];
    *az = ((uint16_t)buf[5] << 8) | buf[6];
}

//得到加计值、温度值、角速度值(原始值)
//gx,gy,gz:陀螺仪x,y,z轴的原始读数(带符号)
//返回值:0,成功
//    其他,错误代码
void ICM_Get_Raw_data(short *ax, short *ay, short *az, short *gx, short *gy, short *gz)
{
    uint8_t buf[15];
    ICM_Read_Len(ICM_ACCEL_XOUTH_REG, 14, buf);

    *ax = ((uint16_t)buf[1] << 8) | buf[2];
    *ay = ((uint16_t)buf[3] << 8) | buf[4];
    *az = ((uint16_t)buf[5] << 8) | buf[6];
    *gx = ((uint16_t)buf[9] << 8) | buf[10];
    *gy = ((uint16_t)buf[11] << 8) | buf[12];
    *gz = ((uint16_t)buf[13] << 8) | buf[14];
}

void ICM_Read_Len(uint8_t reg, uint8_t len, uint8_t *buf)
{
    reg = reg | 0x80;
    /* 写入要读的寄存器地址 */
    I2C_ReadMem(I2C2, ICM20602_ADDR, reg, 1, &buf[1], len, OS_TIME_OUT_WAIT_FOREVER);
}

void ICM_Write_Byte(uint8_t reg, uint8_t value)
{
    I2C_WriteMem(I2C2, ICM20602_ADDR, reg, 1, &value, 1, OS_TIME_OUT_WAIT_FOREVER);
}

uint8_t ICM_Read_Byte(uint8_t reg)
{
    uint8_t temp;
    reg = reg | 0x80; //先发送寄存器

    I2C_ReadMem(I2C2, ICM20602_ADDR, reg, 1, &temp, 1, OS_TIME_OUT_WAIT_FOREVER);

    return temp;
}

