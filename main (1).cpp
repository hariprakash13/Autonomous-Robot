#include "mbed.h"
#include <chrono>
using namespace std::chrono;

// ---------- SERIAL ----------
static BufferedSerial usb(USBTX, USBRX, 9600);
FileHandle *mbed::mbed_override_console(int) { return &usb; }

// ---------- MOTOR ----------
PwmOut steer(PB_0);
PwmOut motor_fwd(PB_10);
PwmOut motor_rev(PB_4);

// ---------- ULTRASONIC ----------
DigitalOut trig_L(PA_10);
DigitalIn  echo_L(PB_3);
DigitalOut trig_R(PA_8);
DigitalIn  echo_R(PA_9);

// ---------- STATES ----------
enum State {
    FORWARD,
    STOPPED,
    REVERSING,
    TURNING,
    DEAD_END
};

State state = FORWARD;
int turn_dir = 0; // -1 = left, +1 = right

// ---------- ULTRASONIC FUNCTION ----------
float read_ultra(DigitalOut &trig, DigitalIn &echo) {
    trig = 0; wait_us(2);
    trig = 1; wait_us(10);
    trig = 0;

    Timer t;
    t.start();

    while (echo == 0) {
        if (t.elapsed_time() > 30ms) return 500;
    }

    t.reset();

    while (echo == 1) {
        if (t.elapsed_time() > 30ms) return 500;
    }

    float us = duration_cast<microseconds>(t.elapsed_time()).count();
    return (us * 0.343f) / 2.0f; // mm
}

// ---------- MOVEMENT ----------
void stop() {
    motor_fwd.pulsewidth_ms(0);
    motor_rev.pulsewidth_ms(0);
}

void forward(int speed) {
    motor_rev.pulsewidth_ms(0);
    motor_fwd.pulsewidth_ms(speed);
}

void reverse(int speed) {
    motor_fwd.pulsewidth_ms(0);
    motor_rev.pulsewidth_ms(speed);
}

void steer_center() { steer.pulsewidth_us(1500); }
void steer_left()   { steer.pulsewidth_us(1900); }
void steer_right()  { steer.pulsewidth_us(1100); }

// ---------- MAIN ----------
int main() {

    // PWM setup
    steer.period_ms(20);
    motor_fwd.period_ms(20);
    motor_rev.period_ms(20);

    steer_center();
    stop();

    ThisThread::sleep_for(2s);

    float L = 500, R = 500;

    while (1) {

        // -------- SENSOR READ --------
        L = read_ultra(trig_L, echo_L);
        R = read_ultra(trig_R, echo_R);

        printf("State:%d  L:%d  R:%d\r\n", state, (int)L, (int)R);

        switch (state) {

        //  NORMAL FORWARD
        case FORWARD:

            // Dead end condition (both sides very close)
            if (L < 150 && R < 150) {
                state = STOPPED;
                break;
            }

            // Corridor following
            if (L < 200 && R > 200) {
                steer_right();
                forward(8);
            }
            else if (R < 200 && L > 200) {
                steer_left();
                forward(8);
            }
            else {
                steer_center();
                forward(10);
            }

        break;

        //  STOP
        case STOPPED:
            stop();
            ThisThread::sleep_for(300ms);
            state = REVERSING;
        break;

        //  REVERSE
        case REVERSING:
            steer_center();
            reverse(8);
            ThisThread::sleep_for(600ms);
            stop();

            // Decide turn direction (go to open space)
            if (L > R) turn_dir = -1;  // left
            else       turn_dir = 1;   // right

            state = TURNING;
        break;

        //  TURN
        case TURNING:

            if (turn_dir == -1) steer_left();
            else steer_right();

            forward(8);
            ThisThread::sleep_for(800ms); // tune this!

            steer_center();
            state = FORWARD;

        break;

        //  DEAD END (optional full reverse)
        case DEAD_END:

            reverse(8);
            steer_center();
            ThisThread::sleep_for(2000ms);

            stop();
            while (1);

        break;
        }

        ThisThread::sleep_for(50ms);
    }
}