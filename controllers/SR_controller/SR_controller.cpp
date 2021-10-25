// File:          SR_controller.cpp
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

auto constexpr WHEEL_RADIUS = 0.0205; 
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
    std::string coffee {};
    std::string command {};
    bool timing;
    double startTime;
    double prepareTime;
    bool toOrder;
    bool toPickup;
    bool toStartfromPickup;
    bool toStartError;
    std::string customer{};
    int customerChannel;
    std::vector<std::vector<std::string>> menu;
    std::string cash;
    std::string coffeePrice{};

public:

    Customer(webots::Robot &aRobot) : robot{aRobot}, 
    timeStep{robot.getBasicTimeStep()},keyboard{robot.getKeyboard()}, 
    emitter{robot.getEmitter("emitter")}, 
    receiver{robot.getReceiver("receiver")},
    leftMotor{robot.getMotor("left wheel motor")}, 
    rightMotor{robot.getMotor("right wheel motor")}, timing{false},
    startTime{0}, prepareTime{100}, toOrder{false}, toPickup{false}, 
    toStartfromPickup{false}, toStartError{false}, customerChannel{-1}
    {
        speed[0] = 0.0;
        speed[1] = 0.0;
        keyboard->enable(timeStep);
        receiver->enable(timeStep);
        readMenu();
        readStarting();
        std::ofstream file;
        file.open("../Account.csv");
        file << "Time(s),Item,Customer,Account Balance($)\n";
        std::string line{"0, , ," + cash + "\n"};
        file << line;
        file.close();
    }

    void setSpeed(double leftSpeed, double rightSpeed) {
        speed[0] = leftSpeed;
        speed[1] = rightSpeed;
    }
    std::array<double, 2> getSpeed() {
        return speed;
    }

    void readMenu() {
        std::string filename{"../Menu.csv"};
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
        menu = table;
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
        cash = std::stod(table[4][1]);
    }

    void teleoperation() {
        
        leftMotor->setPosition(INFINITY);
        rightMotor->setPosition(INFINITY);
        leftMotor->setVelocity(0.0);
        rightMotor->setVelocity(0.0);

        while (robot.step(timeStep) != -1) {
            auto key = static_cast<int>(keyboard->getKey());
            if (key == keyboard->UP) {
                setSpeed(0.5 * MAX_MOTOR_SPEED, 0.5 * MAX_MOTOR_SPEED);
            } else if (key == keyboard->RIGHT) {
                setSpeed(0.5 * MAX_MOTOR_SPEED, -0.5 * MAX_MOTOR_SPEED);
            } else if (key == keyboard->LEFT) {
                setSpeed(-0.5 * MAX_MOTOR_SPEED, 0.5 * MAX_MOTOR_SPEED);
            } else if (key == ' ') {
                setSpeed(0.0, 0.0);
            } else if (key == 'E') {
                return;
            } else if (key == 65579) { //ket == '+'
                std::array<double, 2> currentSpeed = getSpeed();
                std::array<double, 2> newSpeed {currentSpeed.at(0) + 
                    (0.1 * MAX_MOTOR_SPEED), 
                                                currentSpeed.at(1) + 
                    (0.1 * MAX_MOTOR_SPEED)};
                setSpeed(newSpeed.at(0), newSpeed.at(1));
            }

            std::array<double, 2> motorSpeed = getSpeed();
            leftMotor->setVelocity(motorSpeed.at(0));
            rightMotor->setVelocity(motorSpeed.at(1));
        }
    }

    void toOrderCommand() {
        if (!timing) {
            std::cout << "Staff: I am heading to order counter\n";
            startTime = robot.getTime();
            timing = true;
        }
        double currentTime = robot.getTime();
        if ((currentTime - startTime) < 0.155) {
            command = "Left";
        } else if ((currentTime - startTime) < 17) {
            command = "Straight";
        } else if ((currentTime - startTime) < 17.52) {
            command = "Left";
        } else if ((currentTime - startTime) < 17.62) {
            command = "Stop";
        } else {
            for (auto item : menu) {
                if (item[0] == coffee) {
                    std::cout << "Staff: Hi " << customer << ", the price for " 
                    << coffee << " is " << item[2] << " dollors\n";
                    coffeePrice = item[2];
                    prepareTime = std::stod(item[1]);
                    emitter->setChannel(customerChannel);
                    std::string message = item[2] + ",CheckBalance";
                    emitter->send(message.c_str(), message.size() + 1);
                    toOrder = false;
                    timing = false;
                    return;
                }
            }
            std::cout << "Staff: Hi Customer 1, oh no, we don't have " 
            << coffee << " in our menu\n";
            emitter->setChannel(customerChannel);
            std::string message = "Error";
            emitter->send(message.c_str(), message.size() + 1);
            toStartError = true;
            toOrder = false;
            timing = false;
        }
    }

    void toPickUpCommand() {
        if (!timing) {
            std::cout << "Staff: I am heading to pickup counter\n";
            startTime = robot.getTime();
            timing = true;
        }
        double currentTime = robot.getTime();
        if ((currentTime - startTime) < 0.68) {
            command = "Left";
        } else if ((currentTime - startTime) < 10) {
            command = "Straight";
        } else if ((currentTime - startTime) < 10.1) {
            command = "Stop";
        } else if ((currentTime - startTime) < 10.1 + prepareTime) {
           
        } else {
            std::cout << "Staff: Hi " << customer << " your " << coffee 
            << " is ready, please proceed to pickup counter\n";
            emitter->setChannel(customerChannel);
            std::string message = "PickUp";
            emitter->send(message.c_str(), message.size() + 1);
            toPickup = false;
            toStartfromPickup = true;
            timing = false;
        }
    }

    void toStartfromPickupCommand () {
        if (!timing) {
            std::cout << "Staff: I am heading to pickup counter\n";
            startTime = robot.getTime();
            timing = true;
        }
        double currentTime = robot.getTime();
        if ((currentTime - startTime) < 0.33) {
            command = "Left";
        } else if ((currentTime - startTime) < 9.33) {
            command = "Straight";
        } else if ((currentTime - startTime) < 10.38) {
            command = "Left";
        } else if ((currentTime - startTime) < 10.5) {
            command = "Stop";
        } else {
            toStartfromPickup = false;
            timing = false;
       }
    }

    void toStartErrorCommand () {
        if (!timing) {
            std::cout << "Staff: I am heading to order counter\n";
            startTime = robot.getTime();
            timing = true;
        }
        double currentTime = robot.getTime();
        if ((currentTime - startTime) < 0.86) {
            command = "Left";
        } else if ((currentTime - startTime) < 17.6) {
            command = "Straight";
        } else if ((currentTime - startTime) < 18.82) {
            command = "Left";
        } else if ((currentTime - startTime) < 18.9) {
            command = "Stop";
        } else {
            toStartError = false;
            timing = false;
        }
    }

    void autoMode() {
        while (robot.step(timeStep) != -1) {

            if (receiver->getQueueLength() > 0) {
                auto message = static_cast<std::string>(
                    (static_cast<const char *>(receiver->getData())));
                receiver->nextPacket();
                if (message.find("OrderTile") != std::string::npos) {
                    toOrder = true;
                    std::vector<std::string> instructions;
                    std::stringstream messageStream{message};
                    std::string instruction;
                    while (getline(messageStream, instruction, ',')) {
                        instructions.push_back(instruction);
                    }
                    customerChannel = std::stoi(instructions[0]);
                    customer = instructions[1];
                    coffee = instructions[2];
                } else if (message.find("Confirm") != std::string::npos) {
                    std::ofstream file;
                    file.open("../Account.csv", std::ios::out | std::ios::app);
                    file << "Time(s),Item,Customer,Account Balance($)\n";
                    std::string line{std::to_string(robot.getTime()) + "," 
                    + coffee + "," + std::to_string(customerChannel) + "," 
                    + coffeePrice + "\n"};
                    file << line;
                    file.close();
                    toPickup = true;
                } else if(message.find("Cancel") != std::string::npos) {
                    toStartError = true;
                } else if(message.find("FinishingStatement") != 
                    std::string::npos) {
                }
            }
            if (toOrder) {
                toOrderCommand();
            }

            if (toPickup) {
                toPickUpCommand();
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

    void cpu() {
        while (robot.step(timeStep) != -1) {
            if (receiver->getQueueLength() > 0) {
                auto message = static_cast<std::string>(
                    (static_cast<const char *>(receiver->getData())));
                receiver->nextPacket();
                if (message == "Remote Control") {
                    std::cout << "Remote controlling "
                        << static_cast<std::string>(robot.getName()) << "...\n";
                    teleoperation();
                } else if (message.find("StartAuto") != std::string::npos) {
                    autoMode();
                    return;
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

