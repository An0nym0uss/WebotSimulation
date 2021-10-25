// File:          CR2_controller.cpp
// Author: Austin Chen

#include <iostream>
#include <array>
#include <string>
#include <iomanip>
#include <fstream>

#include <webots/Receiver.hpp>
#include <webots/Emitter.hpp>
#include <webots/Keyboard.hpp>
#include <webots/Motor.hpp>
#include <webots/Robot.hpp>

auto constexpr MAX_MOTOR_SPEED = 6.28;

class Customer {

private:
    std::array<double, 2> speed {};
    webots::Robot robot;
    double timeStep;
    webots::Keyboard *keyboard;
    webots::Emitter *emitter;
    webots::Receiver *receiver;
    webots::Motor *leftMotor;
    webots::Motor *rightMotor;
    double cash;
    std::string coffee {};
    std::string command {};
    bool timing;
    double startTime;
    bool toOrder;
    bool toStartFromOrder;
    bool toPickup;
    bool toStartfromPickup;
    bool toStartError;

public:
    Customer(webots::Robot &aRobot) : robot{aRobot}, 
    timeStep{robot.getBasicTimeStep()}, keyboard{robot.getKeyboard()}, 
    emitter{robot.getEmitter("emitter")}, 
    receiver{robot.getReceiver("receiver")},
    leftMotor{robot.getMotor("left wheel motor")}, 
    rightMotor{robot.getMotor("right wheel motor")}, 
    timing{false}, startTime{0}, toOrder{true}, toStartFromOrder{false}, 
    toPickup{false}, toStartfromPickup{false}, toStartError{false}
    {
        speed[0] = 0.0;
        speed[1] = 0.0;
        keyboard->enable(timeStep);
        receiver->enable(timeStep);
        readStarting();
    }

    // getters and setter for speed
    void setSpeed(double leftSpeed, double rightSpeed) {
        speed[0] = leftSpeed;
        speed[1] = rightSpeed;
    }

    std::array<double, 2> getSpeed() {
        return speed;
    }

    // Read in the Starting.csv file to find the starting cash of the robot
    void readStarting() {
        std::string filename{"../Starting.csv"};
        std::ifstream data{filename};
        if (!data) {
            std::cerr << "error: open file for input failed!\n";
            return;
        }
        std::string line;
        std::getline(data, line);
        std::vector<std::vector<std::string>> table;
        while (std::getline(data, line)) {
            std::stringstream lineStream(line);
            std::string cell;
            std::vector<std::string> row;
            while (std::getline(lineStream, cell, ',')) {
                row.push_back(cell);
            }
            table.push_back(row);
        }
        if (robot.getName() == "Customer1") {
            cash = std::stod(table[0][1]);
        } else if (robot.getName() == "Customer2") {
            cash = std::stod(table[1][1]);
        } else if (robot.getName() == "Customer3") {
            cash = std::stod(table[2][1]);
        } else if (robot.getName() == "Customer4") {
            cash = std::stod(table[3][1]);
        } 
    }

    // allow uer input to remote control the robot
    void teleoperation() {
        
        leftMotor->setPosition(INFINITY);
        rightMotor->setPosition(INFINITY);
        leftMotor->setVelocity(0.0);
        rightMotor->setVelocity(0.0);

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
            } else if (key == 'E') {
                return;
            } else if (key == 65579) { //ket == '+'
                std::array<double, 2> currentSpeed = getSpeed();
                std::array<double, 2> newSpeed {
                    currentSpeed.at(0) + (0.1 * MAX_MOTOR_SPEED), 
                    currentSpeed.at(1) + (0.1 * MAX_MOTOR_SPEED)};
                setSpeed(newSpeed.at(0), newSpeed.at(1));
            }

            std::array<double, 2> motorSpeed = getSpeed();
            leftMotor->setVelocity(motorSpeed.at(0));
            rightMotor->setVelocity(motorSpeed.at(1));
        }
    }
    
    // following series of functions contains command that allows the traversal
    // of the robot from one specific location to another in auto mode, the 
    // angle and distance that would allow the robot arrived at the destination
    // are determoned through counting the time elapsed during the simulation 
    // time
    void toOrderCommand() {
        if (!timing) {
            std::cout << "Customer 2: I am heading to order counter\n";
            startTime = robot.getTime();
            timing = true;
        }
        double currentTime = robot.getTime();
        if ((currentTime - startTime) < 0.76) {
            command = "Right";
        } else if ((currentTime - startTime) < 25.5) {
            command = "Straight";
        } else if ((currentTime - startTime) < 25.6) {
            command = "Stop";
        } else {
            emitter->setChannel(5);
            std::cout << "Customer 2: Hi Staff, I want to order " << coffee << 
                " \n";
            std::string message = "2,Customer2," + coffee + ",atOrderTile";
            emitter->send(message.c_str(), message.size() + 1);
            toOrder = false;
            timing = false;
        }
    }
    void toStartFromOrderCommand() {
        if (!timing) {
            std::cout << "Customer 2: I am returning to starting point \n";
            startTime = robot.getTime();
            timing = true;
        }
        double currentTime = robot.getTime();
        if ((currentTime - startTime) < 1.98) {
            command = "Left";
        } else if ((currentTime - startTime) < 27) {
            command = "Straight";
        } else if ((currentTime - startTime) < 27.1) {
            command = "Stop";
        } else {
            toStartFromOrder = false;
            timing = false;
        }
    }

    void toPickupCommand() {
        if (!timing) {
            std::cout << "Customer 2: I am heading to pickup counter\n";
            startTime = robot.getTime();
            timing = true;
        }
        double currentTime = robot.getTime();
        if ((currentTime - startTime) < 2.2) {
            command = "Right";
        } else if ((currentTime - startTime) < 24) {
            command = "Straight";
        } else if ((currentTime - startTime) < 24.18) {
            command = "Right";
        } else if ((currentTime - startTime) < 24.3) {
            command = "Stop";
        }  else {
            std::cout << "Customer 2: I got my " << coffee << " \n";
            toPickup = false;
            timing = false;
            toStartfromPickup = true;
        }
    }
    void toStartfromPickupCommand() {
        if (!timing) {
            std::cout << "Customer 2: I am returning to starting point\n";
            startTime = robot.getTime();
            timing = true;
        }
        double currentTime = robot.getTime();
        if ((currentTime - startTime) < 1.78) {
            command = "Right";
        } else if ((currentTime - startTime) < 23) {
            command = "Straight";
        } else if ((currentTime - startTime) < 23.6) {
            command = "Stop";
        }  else {
            emitter->setChannel(0);
            std::string message = "Next";
            emitter->send(message.c_str(), message.size() + 1);
            toStartfromPickup = false;
            timing = false;
        }
    }
    void toStartErrorCommand() {
        if (!timing) {
            std::cout << "Customer 2: I am returning to starting point \n";
            startTime = robot.getTime();
            timing = true;
        }
        double currentTime = robot.getTime();
        if ((currentTime - startTime) < 1.98) {
            command = "Left";
        } else if ((currentTime - startTime) < 27) {
            command = "Straight";
        } else if ((currentTime - startTime) < 27.1) {
            command = "Stop";
        }   else {
            emitter->setChannel(0);
            std::string message = "Next";
            emitter->send(message.c_str(), message.size() + 1);
            toStartError = false;
            timing = false;
        }
    }

    // following a set of pre-determined instruction and orders are sent to the
    // robot through the communication between it and the staff robot
    void autoMode() {
        while (robot.step(timeStep) != -1) {
            if (receiver->getQueueLength() > 0) {
                auto message = static_cast<std::string>(
                    (static_cast<const char *>(receiver->getData())));
                receiver->nextPacket();
                if (message.find("CheckBalance") != std::string::npos) {
                    std::vector<std::string> instructions;
                    std::stringstream messageStream{message};
                    std::string instruction;
                    while (getline(messageStream, instruction, ',')) {
                        instructions.push_back(instruction);
                    }
                    if (cash >= std::stoi(instructions[0])) {
                        std::cout << "Customer 1: Hi Staff, I will buy it\n";
                        cash = cash - std::stoi(instructions[0]);
                        emitter->setChannel(5);
                        std::string message = "Confirm";
                        emitter->send(message.c_str(), message.size() + 1);
                        toStartFromOrder = true;
                    } else if (cash < std::stoi(instructions[0])){
                        std::cout << 
                            "Customer 1: Hi Staff, oops, I can't afford it\n";
                        emitter->setChannel(5);
                        std::string message = "Cancel";
                        emitter->send(message.c_str(), message.size() + 1);
                        toStartError = true;
                    }
                } else if (message.find("PickUp") != std::string::npos) {
                    toPickup = true;
                } else if (message.find("Error") != std::string::npos) {
                    toStartError = true;
                } else if (message.find("FinishingStatement") != std::string::npos) {
                    std::cout << "Customer 2: My current balance is " << cash 
                        << " dollors\n";
                }
            }
            if (toOrder) {
                toOrderCommand();
            }
            if (toStartFromOrder) {
                toStartFromOrderCommand();
            }
            if (toPickup) {
                toPickupCommand();
            }
            if (toStartfromPickup) {
                toStartfromPickupCommand();
            }
            if (toStartError) {
                toStartErrorCommand();
            }

            leftMotor->setPosition(INFINITY);
            rightMotor->setPosition(INFINITY);
            leftMotor->setVelocity(0.0);
            rightMotor->setVelocity(0.0);

            if (command == "Straight") {
                setSpeed(0.5 * MAX_MOTOR_SPEED, 0.5 * MAX_MOTOR_SPEED);
            } else if (command == "Right") {
                setSpeed(0.5 * MAX_MOTOR_SPEED, -0.5 * MAX_MOTOR_SPEED);
            } else if (command == "Left") {
                setSpeed(-0.5 * MAX_MOTOR_SPEED, 0.5 * MAX_MOTOR_SPEED);
            } else if (command == "Stop") {
                setSpeed(0.0, 0.0);
            }

            std::array<double, 2> motorSpeed = getSpeed();
            leftMotor->setVelocity(motorSpeed.at(0));
            rightMotor->setVelocity(motorSpeed.at(1));
        }
    }

    // deciding if enter Remote control mode or auto mode
    void cpu() {
        while (robot.step(timeStep) != -1) {
            if (receiver->getQueueLength() > 0) {
                auto message = static_cast<std::string>(
                    (static_cast<const char *>(receiver->getData())));
                receiver->nextPacket();
                if (message == "Remote Control") {
                    std::cout << "Remote controlling "<<static_cast<std::string>
                        (robot.getName()) << "...\n";
                    teleoperation();
                } else if (message.find("StartAuto") != std::string::npos) {
                    std::stringstream orderLine{message};
                    std::string orderCoffee{};
                    std::getline(orderLine, orderCoffee, '-');
                    coffee = orderCoffee;
                    autoMode();
                }
            }
        }
    }
};


int main(int argc, char **argv) {

    auto robot = webots::Robot();
    Customer customer(robot);
    customer.cpu();
    return 0;
}
