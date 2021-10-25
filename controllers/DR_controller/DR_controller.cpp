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
#include <fstream>
#include <vector>
#include <iomanip>
#include <queue>

#include <webots/Robot.hpp>
#include <webots/Keyboard.hpp>
#include <webots/Node.hpp>
#include <webots/Supervisor.hpp>
#include <webots/Emitter.hpp>
#include <webots/Receiver.hpp>

class Director{

private:
    int timeStep;
    webots::Supervisor robot;
    webots::Keyboard *keyboard;
    webots::Emitter *emitter;
    webots::Receiver *receiver;
    webots::Field *translationField;
    bool inRc;
    double mTime;
    std::queue<int> robots{};
    std::queue<std::string> orders{};
    

public:

    // Director constructor
    Director(webots::Supervisor &supervisor) : timeStep{128}, robot{supervisor},
        keyboard{robot.getKeyboard()}, emitter{robot.getEmitter("emitter")}, 
        receiver{robot.getReceiver("receiver")}, inRc{false}, mTime{0.0} {
        keyboard->enable(timeStep);
        receiver->enable(timeStep);
        readOrder();
    }

    void toggleInRc(bool toggle) {
        inRc = toggle;
    }

    // getter and setter for start time
    void setStartTime(double time) {
        mTime = time;
    }

    double getStartTime() {
        return mTime;
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
        std::string menu2{
            " Director: Please select the robot to control remotely:\n"
            " Director: Press [1] to control the Purple Robot (Customer1).\n"
            " Director: Press [2] to control the White Robot (Customer2).\n"
            " Director: Press [3] to control the Gold Robot (Customer3).\n"
            " Director: Press [4] to control the Green Robot (Customer4)\n"
            " Director: Press [5] to control the Black Robot (Staff).\n"};
        std::cout << menu2;
    }

    void modeRc() {
        std::string message = "";
        while (robot.step(timeStep) != -1) {
            int key = keyboard->getKey();
            if (key == '1') {
                message = "Remote Control";
                emitter->setChannel(1);
            } else if (key == '2') {
                message = "Remote Control";
                emitter->setChannel(2);
            } else if (key == '3') {
                message = "Remote Control";
                emitter->setChannel(3);
            } else if (key == '4') {
                message = "Remote Control";
                emitter->setChannel(4);
            } else if (key == '5') {
                message = "Remote Control";
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

    // read in the data from Order.csv
    void readOrder() {

        std::string filename{"../Order.csv"};
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
        setStartTime(robot.getTime());

        for (int i = 0; i < static_cast<int>(table.size()); i++) {
            robots.push(std::stoi(table[i][0]));
            orders.push(table[i][1]);
        }
    }

    // start auto mode
    void modeAuto() {
        std::string message = orders.front() + "-StartAuto";
        emitter->setChannel(robots.front());
        emitter->send(message.c_str(), message.size() + 1);
        emitter->setChannel(5);
        emitter->send(message.c_str(), message.size() + 1);
        orders.pop();
        robots.pop();
        emitter->setChannel(-1);
    }

    // start simulation
    void simulate() {
        printMainMenu();
        auto actor = static_cast<webots::Node *>(robot.getFromDef("CUSTOMER1"));
        translationField = actor->getField("translation");
        while (robot.step(timeStep) != -1) {

            if (receiver->getQueueLength() > 0) {
                auto message = static_cast<std::string>(
                    (static_cast<const char *>(receiver->getData())));
                receiver->nextPacket();
                if (message == "Next") {
                    modeAuto();
                } else if (message == "Done") {
                    std::cout << "Director: The whole auto mode takes up " << 
                    robot.getTime() - mTime << "s\n ";
                    std::string message = "FinishingStatement";
                    for (int i = 1; i < 6; i++) {
                        emitter->send(message.c_str(), message.size() + 1);
                        emitter->setChannel(i);
                    }
                }
            }

            int key = keyboard->getKey();
            if (key == 'I') {
                printMainMenu();
            } else if (key == 'R') {
                printRcMenu();
                toggleInRc(true);
                modeRc();
            } else if (key == 'A' && !inRc) {
                modeAuto();
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
