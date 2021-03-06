#ifndef GIMBAL_H
#define GIMBAL_H

#include "system.h"
#include "pwm.h"
#include "led.h"
#include "vcp.h"
#include "math.h"

#include "revo_f4.h"
#include "spi.h"
#include "M25P16.h"


namespace gimbal
{

class Gimbal
{
public:
    // Functions
    Gimbal();
    void rx_callback(uint8_t byte);
    void norm_commands();
    void tx_callback(float command_rate, float servo_rate, float roll, float pitch, float yaw);
    void calc_servo_rate();
    void rad_to_pwm();
    void pwm_to_rad();
    void get_params();
    void retract_gimbal();
    void extend_gimbal();
    void update_command();


    // Variables

    // serial communication
    static constexpr int OUT_BUFFER_SIZE = 22;
    static constexpr int OUT_START_BYTE = 0xA5;
    static constexpr int OUT_PAYLOAD_LENGTH = 20;
    static constexpr int OUT_MESSAGE_LENGTH = 22;

    static constexpr int IN_BUFFER_SIZE = 14;
    static constexpr int IN_START_BYTE = 0xA5;
    static constexpr int IN_PAYLOAD_LENGTH = 12;
    static constexpr int IN_MESSAGE_LENGTH = 14;

    static constexpr int CRC_LENGTH = 1;
    static constexpr int CRC_INITIAL_VALUE = 0x00;

    // This value is the total range in radians the servo can travel. Not necessarily pi.
    volatile float roll_rad_range = 3.14159;
    volatile float pitch_rad_range = 3.14159;
    volatile float yaw_rad_range = 3.14159;
    volatile float retract_rad_range = 3.14159;

    // Offset values for aligning gimbal servos with desired coordinate frame (IN RADIANS).
    volatile float  roll_rad_offset = 0;
    volatile float pitch_rad_offset = 0;
    volatile float   yaw_rad_offset = 0;
    volatile float retract_rad_offset = 0;

    volatile int roll_direction = 1;
    volatile int pitch_direction = 1;
    volatile int yaw_direction = 1;
    volatile int retract_direction = 1;

    volatile int roll_start_pwm = 1500;
    volatile int pitch_start_pwm = 1500;
    volatile int yaw_start_pwm = 1500;
    volatile int retract_start_pwm = 1500;

    // Limit the servo travel for mechanical restrictions.
    volatile int  roll_pwm_min = 1000;
    volatile int pitch_pwm_min = 1000;
    volatile int   yaw_pwm_min = 1000;
    volatile int retract_pwm_min = 1000;

    volatile int  roll_pwm_max = 2000;
    volatile int pitch_pwm_max = 2000;
    volatile int   yaw_pwm_max = 2000;
    volatile int retract_pwm_max = 2000;

    volatile int retract_up_pwm = 2000;
    volatile int retract_down_pwm = 1000;

    volatile int  roll_pwm_center = 1500;
    volatile int pitch_pwm_center = 1500;
    volatile int   yaw_pwm_center = 1500;

    volatile float roll_rad_command;
    volatile float pitch_rad_command;
    volatile float yaw_rad_command;

    volatile float roll_pwm_command = roll_start_pwm;
    volatile float pitch_pwm_command = pitch_start_pwm;
    volatile float yaw_pwm_command = yaw_start_pwm;

    volatile float norm_roll;
    volatile float norm_pitch;
    volatile float norm_yaw;

    volatile float command_in_rate;
    volatile float servo_command_rate;
    volatile long time_of_last_servo;

    volatile int servo_roll_frequency = 50;
    volatile int servo_pitch_frequency = 330;
    volatile int servo_yaw_frequency = 330;
    volatile int servo_retract_frequency = 50;
    static constexpr int num_servos = 4;

    bool has_retract = false;
    bool is_retracted = true;
    bool first_retract = true;
    bool first_extend = true;
    volatile long retract_time;
    volatile long extend_time;
    volatile long time_of_last_message = 0;


    uint8_t out_buf[OUT_BUFFER_SIZE];

    PWM_OUT servo_out[num_servos];

    VCP vcp;

    void blink_heartbeat();

private:
    float roll, pitch, yaw;
    uint32_t roll_current_pwm, pitch_current_pwm, yaw_current_pwm;
    float pitch_current_rad, yaw_current_rad;

    // Functions
    void handle_in_msg(float roll, float pitch, float yaw);
    void unpack_in_payload(uint8_t buf[], float *roll, float *pitch, float *yaw);
    bool parse_in_byte(uint8_t c);
    uint8_t in_crc8_ccitt_update (uint8_t inCrc, uint8_t inData);
    uint8_t out_crc8_ccitt_update (uint8_t outCrc, uint8_t outData);
    void blink_led();
    void calc_command_rate();
    void set_params(float roll, float pitch, float yaw);
    void smooth_command(float pwm_command, int servo_freq, int servo_num);
    void calc_smooth_rate(int servo_freq);

    volatile int num_smooth_steps = 0;
    volatile float diff_command = 0;
    volatile float pwm_step = 0;

    // Variables
    enum ParseState {
        PARSE_STATE_IDLE,
        PARSE_STATE_GOT_START_BYTE,
        PARSE_STATE_GOT_PAYLOAD
    };

    struct EepromData{
        int servo_roll_frequency;
        int roll_pwm_max;
        int roll_pwm_min;
        int roll_direction;
        float roll_rad_range;
        float roll_rad_offset;
        float roll_start_pwm;

        int servo_pitch_frequency;
        int pitch_pwm_max;
        int pitch_pwm_min;
        int pitch_direction;
        float pitch_rad_range;
        float pitch_rad_offset;
        float pitch_start_pwm;

        int servo_yaw_frequency;
        int yaw_pwm_max;
        int yaw_pwm_min;
        int yaw_direction;
        float yaw_rad_range;
        float yaw_rad_offset;
        float yaw_start_pwm;

        int servo_retract_frequency;
        int retract_pwm_max;
        int retract_pwm_min;
        int retract_direction;
        float retract_rad_range;
        float retract_rad_offset;
        float retract_start_pwm;

        uint8_t begin_data_DB = 0xDB;
        uint8_t end_data_FF = 0xFF;
        uint8_t crc;
    };

    enum ParamValue {
        WRITE_PARAMS=5000,
        READ_PARAMS=7000,
        PARAM_IDLE = 6000,

        SERVO_PITCH_FREQUENCY=5001,
        SERVO_PITCH_UPPER_PWM=5002,
        SERVO_PITCH_LOWER_PWM=5003,
        SERVO_PITCH_DIRECTION=5004,
        SERVO_PITCH_RAD_RANGE=5005,
        SERVO_PITCH_RAD_OFFSET=5006,
        SERVO_PITCH_START_PWM=5007,

        SERVO_YAW_FREQUENCY=5011,
        SERVO_YAW_UPPER_PWM=5012,
        SERVO_YAW_LOWER_PWM=5013,
        SERVO_YAW_DIRECTION=5014,
        SERVO_YAW_RAD_RANGE=5015,
        SERVO_YAW_RAD_OFFSET=5016,
        SERVO_YAW_START_PWM=5017,

        SERVO_RETRACT_FREQUENCY=5021,
        SERVO_RETRACT_UPPER_PWM=5022,
        SERVO_RETRACT_LOWER_PWM=5023,
        SERVO_RETRACT_DIRECTION=5024,
        SERVO_RETRACT_RAD_RANGE=5025,
        SERVO_RETRACT_RAD_OFFSET=5026,
        SERVO_RETRACT_START_PWM=5027
    };

    volatile long time_of_last_command;
    volatile long time_of_last_blink;
    volatile long time_of_last_heartbeat;


    volatile uint32_t crc_error_count;
    volatile uint32_t start_byte_error;
    volatile uint32_t payload_index_error;

    // Flash
    SPI spi;
    M25P16 flash;

    // serial
    volatile uint8_t in_buf[IN_BUFFER_SIZE];

    ParseState parse_state;
    ParamValue param_value;
    EepromData eeprom_data;
    uint8_t eeprom_buffer[sizeof(EepromData)];
    uint8_t in_payload_buf[IN_PAYLOAD_LENGTH];
    volatile int in_payload_index;
    volatile uint8_t in_crc_value;
    volatile uint8_t out_crc_value;

    LED info;
    LED heartbeat;

protected:

};

#endif // GIMBAL_H
} // end gimbal namespace
