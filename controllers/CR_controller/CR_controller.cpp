// File:          CR_controller.cpp
// Date:
// Description:
// Author:
// Modifications:

// You may need to add webots include files such as
// <webots/DistanceSensor.hpp>, <webots/Motor.hpp>, etc.
// and/or to add some other includes
#include <iostream>

#include <webots/Receiver.hpp>
#include <webots/Keyboard.hpp>
#include <webots/Motor.hpp>
#include <webots/Robot.hpp>

auto constexpr WHEEL_RADIUS = 0.0205; 
auto constexpr MAX_MOTOR_SPEED = 6.28;

class Customer {

private:

public:
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
            auto leftSpeed = 0.0;
            auto rightSpeed = 0.0;

            if (key == 'W') {
                leftSpeed = 0.5 * MAX_MOTOR_SPEED;
                rightSpeed = 0.5 * MAX_MOTOR_SPEED;
            }
            else if (key == 'D') {
                leftSpeed = 0.5 * MAX_MOTOR_SPEED;
                rightSpeed = -0.5 * MAX_MOTOR_SPEED;
            }
            else if (key == 'A') {
                leftSpeed = -0.5 * MAX_MOTOR_SPEED;
                rightSpeed = 0.5 * MAX_MOTOR_SPEED;
            }

            leftMotor->setVelocity(leftSpeed);
            rightMotor->setVelocity(rightSpeed);
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
