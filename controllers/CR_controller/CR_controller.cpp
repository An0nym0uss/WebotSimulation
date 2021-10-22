// File:          CR_controller.cpp
// Date:
// Description:
// Author:
// Modifications:

// You may need to add webots include files such as
// <webots/DistanceSensor.hpp>, <webots/Motor.hpp>, etc.
// and/or to add some other includes
#include <iostream>
#include <array>

#include <webots/Receiver.hpp>
#include <webots/Keyboard.hpp>
#include <webots/Motor.hpp>
#include <webots/Robot.hpp>

auto constexpr WHEEL_RADIUS = 0.0205; 
auto constexpr MAX_MOTOR_SPEED = 6.28;

class Customer {

private:
    std::array<double, 2> speed {};

public:

    Customer() {
        speed[0] = 0.0;
        speed[1] = 0.0;
    }

    void setSpeed(double leftSpeed, double rightSpeed) {
        speed[0] = leftSpeed;
        speed[1] = rightSpeed;
    }
    std::array<double, 2> getSpeed() {

        return speed;
    }

    void teleoperation(webots::Robot &robot) {

        auto leftMotor = static_cast<webots::Motor *>(robot.getMotor("left wheel motor"));
        auto rightMotor = static_cast<webots::Motor *>(robot.getMotor("right wheel motor"));
        leftMotor->setPosition(INFINITY);
        rightMotor->setPosition(INFINITY);
        leftMotor->setVelocity(0.0);
        rightMotor->setVelocity(0.0);
        auto const timeStep = static_cast<int>(robot.getBasicTimeStep());
        auto keyboard = static_cast<webots::Keyboard *>(robot.getKeyboard());
        keyboard->enable(timeStep);

        while (robot.step(timeStep) != -1) {
            auto key = static_cast<int>(keyboard->getKey());
            if (key == 'W') {
                setSpeed(0.5 * MAX_MOTOR_SPEED, 0.5 * MAX_MOTOR_SPEED);
            } else if (key == 'D') {
                setSpeed(0.5 * MAX_MOTOR_SPEED, -0.5 * MAX_MOTOR_SPEED);
            } else if (key == 'A') {
                setSpeed(-0.5 * MAX_MOTOR_SPEED, 0.5 * MAX_MOTOR_SPEED);
            } else if (key == ' ') {
                setSpeed(0.0, 0.0);
            } else if (key == 65579) {
                std::array<double, 2> currentSpeed = getSpeed();
                std::array<double, 2> newSpeed {currentSpeed.at(0) + (0.1 * MAX_MOTOR_SPEED), currentSpeed.at(1) + (0.1 * MAX_MOTOR_SPEED)};
                setSpeed(newSpeed.at(0), newSpeed.at(1));
            }

            std::array<double, 2> motorSpeed = getSpeed();
            leftMotor->setVelocity(motorSpeed.at(0));
            rightMotor->setVelocity(motorSpeed.at(1));
        }
    }
    void remoteControls() {
        auto robot = webots::Robot();
        auto const timeStep = 60;
        auto receiver = static_cast<webots::Receiver *>(robot.getReceiver("receiver"));
        receiver->enable(timeStep);

        while (robot.step(timeStep) != -1) {
            if (receiver->getQueueLength() > 0) {
                auto message = static_cast<std::string>((static_cast<const char *>(receiver->getData())));
                std::cout << "Remote controlling " << message << "...\n";
                receiver->nextPacket();
                teleoperation(robot);
            }
        }
    }
};
int main(int argc, char **argv)
{
    Customer customer;
    customer.remoteControls();
    return 0;
}
