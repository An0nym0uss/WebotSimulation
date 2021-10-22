// File:          DR_controller.cpp
// Date:
// Description:
// Author:
// Modifications:

// You may need to add webots include files such as
// <webots/DistanceSensor.hpp>, <webots/Motor.hpp>, etc.
// and/or to add some other includes
#include <iostream>
#include <string>

#include <webots/Robot.hpp>
#include <webots/Keyboard.hpp>
#include <webots/Node.hpp>
#include <webots/Supervisor.hpp>
#include <webots/Emitter.hpp>

class Director{

private:



public:
    void printMainMenu() {
        std::string menu1{" Director: This is a simulation for MTRN2500 Cafe.\n"
                        " Director: press[I] to reprint the commands.\n"
                        " Director: press[R] to remote control a robot.\n"
                        " Director: press[A] to enter the auto mode.\n"
                        " Director: press[Q] to quit all controller.\n"};
        std::cout << menu1;
    }
    void printRcMenu() {
        std::string menu2{" Director: Press [1] to control the Purple Robot (Customer1).\n"
                         " Director: Press [2] to control the White Robot (Customer2).\n"
                         " Director: Press [3] to control the Gold Robot (Customer3).\n"
                         " Director: Press [4] to control the Green Robot (Customer4)\n"
                         " Director: Press [5] to control the Black Robot (Staff).\n"};
        std::cout << menu2;
    }
    void startRc(webots::Supervisor &robot, int timeStep) {
        auto keyboard = static_cast<webots::Keyboard *>(robot.getKeyboard());
        keyboard->enable(timeStep);
        auto emitter = static_cast<webots::Emitter *>(robot.getEmitter("emitter"));
        std::string message = "";
        while (robot.step(timeStep) != -1) {
            auto key = static_cast<int>(keyboard->getKey());
            if (key == '1') {
                message = "Customer1";
                emitter->setChannel(1);
            } else if (key == '2') {
                message = "Customer2";
                emitter->setChannel(2);
            } else if (key == '3') {
                message = "Customer3";
                emitter->setChannel(3);
            } else if (key == '4') {
                message = "Customer4";
                emitter->setChannel(4);
            } else if (key == '5') {
                message = "Staff";
                emitter->setChannel(5);
            } else {
                //return;
            } if (message.length() > 0) {
                if (emitter->send(message.c_str(), message.size() + 1)) {
                    return;
                }
            }
        }
    }
    void simulate() {
        auto robot = webots::Supervisor();
        auto const timeStep = 128;
        auto keyboard = static_cast<webots::Keyboard *>(robot.getKeyboard());
        keyboard->enable(timeStep);
        printMainMenu();
        while (robot.step(timeStep) != -1) {
            auto key = static_cast<int>(keyboard->getKey());
            switch (key) {
                case 'I': {
                    printMainMenu();
                    break;
                }
                case 'R':{
                    printRcMenu();
                    keyboard->disable();
                    startRc(robot, timeStep);
                    break;
                }
                case 'A':{
                    break;
                }
                case 'Q':{
                    break;
                }
                default:{
                    // std::cout << "Command not found.\n";
                    // printMainMenu();
                    // break;
                }
            }
        }
    }
};

int main(int argc, char **argv) {

    Director director{};
    director.simulate();

    return 0;
}
