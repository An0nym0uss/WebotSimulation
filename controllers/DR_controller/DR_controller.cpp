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
    int timeStep;
    webots::Supervisor robot;
    webots::Keyboard *keyboard;
    webots::Emitter *emitter;
    bool inRc;

public:

    // Director constructor
    Director(webots::Supervisor &supervisor) : timeStep{128}, robot{supervisor},
        keyboard{robot.getKeyboard()}, emitter{robot.getEmitter("emitter")}, 
        inRc{false} {
        keyboard->enable(timeStep);
    }

    void toggleInRc(bool toggle) {
        inRc = toggle;
    }

    // print the main menu
    void printMainMenu() {
        std::string menu1{" Director: This is a simulation for MTRN2500 Cafe.\n"
                        " Director: press[I] to reprint the commands.\n"
                        " Director: press[R] to remote control a robot.\n"
                        " Director: press[A] to enter the auto mode.\n"
                        " Director: press[Q] to quit all controller.\n"};
        std::cout << menu1;
    }

    // print the menu for romote control selection, i.e. when 'R' was pressed
    void printRcMenu() {
        std::string menu2{" Director: Please select the robot to control remotely:\n"
                          " Director: Press [1] to control the Purple Robot (Customer1).\n"
                          " Director: Press [2] to control the White Robot (Customer2).\n"
                          " Director: Press [3] to control the Gold Robot (Customer3).\n"
                          " Director: Press [4] to control the Green Robot (Customer4)\n"
                          " Director: Press [5] to control the Black Robot (Staff).\n"};
        std::cout << menu2;
    }
    void startRc() {
        std::string message = "";
        while (robot.step(timeStep) != -1) {
            int key = keyboard->getKey();
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
            } else if (key != -1){
                std::cout << "Command not found.\n";
                toggleInRc(false);
                printMainMenu();
                return;
            } 
            if (message.length() > 0) {
                if (emitter->send(message.c_str(), message.size() + 1)) {
                    return;
                }
            }
        }
    }
    void simulate() {
        printMainMenu();
        while (robot.step(timeStep) != -1) {
            int key = keyboard->getKey();
            if (key == 'I') {
                printMainMenu();
            } else if (key == 'R') {
                printRcMenu();
                toggleInRc(true);
                startRc();
            } else if (key == 'A') {

            } else if (key == 'Q') {
                return;
            } else if (key != -1 && !inRc){
                std::cout << "Command not found.\n";
                printMainMenu();
            }
        }
    }
};

int main(int argc, char **argv) {

    auto robot = webots::Supervisor();
    Director director(robot);
    director.simulate();

    return 0;
}
