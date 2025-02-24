/*
* Example Windows console application for using the C++ header interface 
* that implements the crazyflie client.
* Copyright (c) 2024-2025 Young Harvill
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
#
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
* You should have received a copy of the GNU Lesser General Public License
* along with this program. If not, see <https://www.gnu.org/licenses/>.
*
* Implements interfaces found here...
* https://github.com/bitcraze/crazyflie-clients-python
* Using the c++ library found here...
* https://github.com/bitcraze/crazyflie-link-cpp
* The usb libray may be found here...
* https://github.com/libusb/libusb
*
*/

#include"crazyflie.h"
#include "windows.h"
#include <thread>
#include <chrono>
#include <conio.h>

const static uint8_t posForward = VK_NUMPAD8;
const static uint8_t posRight = VK_NUMPAD6;
const static uint8_t posLeft = VK_NUMPAD4;
const static uint8_t posBack = VK_NUMPAD2;
const static uint8_t posUp = VK_UP;
const static uint8_t posDown = VK_DOWN;
const static uint8_t yawRight = VK_RIGHT;
const static uint8_t yawLeft = VK_LEFT;
const static uint8_t takeoff = 0x54;
const static uint8_t land = 0x4C;
const static uint8_t arm = 0x41;
const static uint8_t quit = VK_ESCAPE;

//const static float gPi = 3.14159265359;

struct KeyInput
{
    bool keyDown = false; 
    bool keyUp = false; 
    uint8_t keyId = 0;

    KeyInput()
    {
        keyDown = false;
        keyUp = false;
        keyId = 0;
    }

    KeyInput(uint8_t _keyId)
    {
        keyDown = false;
        keyUp = false;
        keyId = _keyId;
    }

    bool HandleKey()
    {
        bool leadingEdge = false; 
        short result = GetAsyncKeyState(keyId);
        if (result)
        {
            keyDown = true;
            if (keyUp)
            {
                leadingEdge = true;
            }
            keyUp = false; 
        }

        else
        {
            keyDown = false;
            keyUp = true;
        }
        return(leadingEdge);
    }
};


struct ControllerState
{
    int32 sampleRate = 0;
    bool armed = false;
    bool flying = false;
    bool landing = false;
    bool hasMultiranger = false;

    float x, y, z;
    float yaw, pitch, roll;
    float front, back, right, left;
    float batteryLevel; 
    uint32_t timeStamp; 

    float hoverZ;
    uint8 lastKey = 0;
    uint32 landTime = 0;

    KeyInput Arm;
    KeyInput Takeoff;
    KeyInput Land;

    KeyInput PosForward;
    KeyInput PosBack;
    KeyInput PosRight;
    KeyInput PosLeft;
    KeyInput PosUp;
    KeyInput PosDown;
    KeyInput YawRight;
    KeyInput YawLeft;
    std::string message;

    ControllerState()
    {
        Arm.keyId = arm;
        Takeoff.keyId = takeoff;
        Land.keyId = land;
        hoverZ = 0.0f;

        PosForward.keyId = posForward;
        PosBack.keyId = posBack;
        PosRight.keyId = posRight;
        PosLeft.keyId = posLeft;
        PosUp.keyId = posUp;
        PosDown.keyId = posDown;
        YawRight.keyId = yawRight;
        YawLeft.keyId = yawLeft;


        x = 0;
        y = 0;
        z = 0;

        yaw = 0;
        pitch = 0;
        roll = 0;

        front = 0;
        back = 0;
        right = 0;
        left = 0;

        lastKey = 0;
        batteryLevel = 0;
        timeStamp = 0;
    }

    void updateControllerState(CrazyFlie& cf)
    {

        x = cf.state_estimate.posX.fetchFloat(timeStamp);
        y = cf.state_estimate.posY.fetchFloat(timeStamp);
        z = cf.state_estimate.posZ.fetchFloat(timeStamp);

        roll = cf.state_estimate.roll.fetchFloat(timeStamp);
        pitch = cf.state_estimate.pitch.fetchFloat(timeStamp);
        yaw = cf.state_estimate.yaw.fetchFloat(timeStamp);

        if (cf.multirangerDeckPresent)
        {
            hasMultiranger = true;
            front = cf.multi_ranger.getFront(timeStamp);
            back = cf.multi_ranger.getBack(timeStamp);
            right = cf.multi_ranger.getRight(timeStamp);
            left = cf.multi_ranger.getLeft(timeStamp);
        }
        batteryLevel = cf.pm.batteryLevel.fetchFloat(timeStamp);
    }
};

void handleQuit(ControllerState& cs, bool& running)
{
    short result = GetAsyncKeyState(quit);
    if (result)
    {
        running = false;
        cs.message = "quitting...";
    }
}

void handleKeyInput(CrazyFlie &cf, ControllerState &cs)
{
    if (!cs.flying)
    {
        if (cs.Arm.HandleKey())
        {
            if (cs.armed)
            {
                cs.armed = false;
                cf.platform->send_arming_request(false);
            }
            else
            {
                cs.armed = true;
                cf.platform->send_arming_request(true);
            }
        }
        if (cs.armed)
        {
            if (cs.Takeoff.HandleKey())
            {
                cs.message = "taking off";
                cf.high_level_commander.takeoff(0.5f, 2.0f);
                cs.flying = true;
                cs.hoverZ = 0.5f;
            }
        }
    }
    else
    {
        if (cs.landing)
        {
            uint32_t currentTicks = GetTickCount();
            uint32_t diff = currentTicks - cs.landTime;
            if (diff > 3000)
            {
                cf.high_level_commander.stop();
                cs.landing = false;
                cs.flying = false;
            }
        }
        else
        {
            bool needsSetPoint = true; 

            if (cs.Land.HandleKey())
            {
                cs.message = "landing...";
                cf.high_level_commander.land(0.0f, 2.0f);
                cs.landTime = GetTickCount();

                cs.landing = true;
                needsSetPoint = false; 
            }
            if (cs.PosForward.HandleKey())
            {
                cs.message = "move forward";
                cf.high_level_commander.go_to(0.4f, 0, 0, 0, 3.0f, true);
                
            }
            if (cs.PosBack.HandleKey())
            {
                cs.message = "move back";
                cf.high_level_commander.go_to(-0.4f, 0, 0, 0, 3.0f, true);
               
            }
            if (cs.PosLeft.HandleKey())
            {
                cs.message = "move left";
                cf.high_level_commander.go_to(0, -0.4f, 0, 0, 3.0f, true);

            }
            if (cs.PosRight.HandleKey())
            {
                cs.message = "move right";
                cf.high_level_commander.go_to(0.0f, 0.4f, 0, 0, 3.0f, true);

            }
            if (cs.PosUp.HandleKey())
            {
                cs.message = "move up";
                cf.high_level_commander.go_to(0.0f, 0, 0.4f, 0, 3.0f, true);

            }
            if (cs.PosDown.HandleKey())
            {
                cs.message = "move down";
                cf.high_level_commander.go_to(0.0f, 0, -0.4f, 0, 3.0f, true);

            }
            if (cs.YawLeft.HandleKey())
            {
                cs.message = "yaw left";
                cf.high_level_commander.go_to(0.0f, 0, 0, (float)gPi * 0.25f, 3.0f, true);

            }
            if (cs.YawRight.HandleKey())
            {
                cs.message = "yaw right";
                cf.high_level_commander.go_to(0.0f, 0, 0, (float)gPi * -0.25f, 3.0f, true);
            }
        }
    }
}


void clearScreen() {
    std::cout << "\033[2J\033[1;1H";
}

void setCursorPosition(int x, int y) {
    std::cout << "\033[" << y << ";" << x << "H";
}

void drawScreen(CrazyFlie& cf, ControllerState &cs)
{
    setCursorPosition(0, 0);

    int32 xpos = 2;
    int32 ypos = 0;
    int32 spacing = 18;

    setCursorPosition(xpos, ypos);
    std::cout << "x: "; 
    std::cout << cs.x;
    std::cout << "       ";
    xpos += spacing;

    setCursorPosition(xpos, ypos);
    std::cout << "y: ";
    std::cout << cs.y;
    std::cout << "       ";
    xpos += spacing;

    setCursorPosition(xpos, ypos);
    std::cout << "z: ";
    std::cout << cs.z;
    std::cout << "       ";
    xpos += spacing;

    setCursorPosition(xpos, ypos);
    std::cout << "roll: ";
    std::cout << cs.roll;
    std::cout << "       ";
    xpos += spacing;

    setCursorPosition(xpos, ypos);
    std::cout << "pitch: ";
    std::cout << cs.pitch;
    std::cout << "       ";
    xpos += spacing;

    setCursorPosition(xpos, ypos);
    std::cout << "yaw: ";
    std::cout << cs.yaw;
    std::cout << "       ";
    xpos += spacing;

   
    if (cs.hasMultiranger)
    {
        ypos += 2;
        xpos = 2;

        setCursorPosition(xpos, ypos);
        std::cout << "range ";
        std::cout << "       ";
        xpos += spacing;

        setCursorPosition(xpos, ypos);
        std::cout << "front: ";
        std::cout << cs.front;
        std::cout << "       ";
        xpos += spacing;

        setCursorPosition(xpos, ypos);
        std::cout << "back: ";
        std::cout << cs.back;
        std::cout << "       ";
        xpos += spacing;

        setCursorPosition(xpos, ypos);
        std::cout << "left: ";
        std::cout << cs.left;
        std::cout << "       ";
        xpos += spacing;

        setCursorPosition(xpos, ypos);
        std::cout << "right: ";
        std::cout << cs.right;
        std::cout << "       ";
        xpos += spacing;
    }

    ypos += 2;
    xpos = 2;

    setCursorPosition(xpos, ypos);
    std::cout << "armed: ";
    if (cs.armed)
    {
        std::cout << "true";
    }
    else
    {
        std::cout << "false";
    }
    std::cout << "       ";
    xpos += spacing;

    setCursorPosition(xpos, ypos);
    std::cout << "flying: ";
    if (cs.flying)
    {
        std::cout << "true";
    }
    else
    {
        std::cout << "false";
    }
    std::cout << "       ";
    xpos += spacing;

    setCursorPosition(xpos, ypos);
    std::cout << "packets per second: ";
    std::cout << cf.portConnect->packetsPerSecond;
    std::cout << "       ";

    xpos += spacing * 2;
    setCursorPosition(xpos, ypos);
    std::cout << "battery: ";
    std::cout << cs.batteryLevel;
    std::cout << "       ";

    ypos += 2;
    xpos = 2;

    setCursorPosition(xpos, ypos);
    std::cout << cs.message;
    std::cout << "       ";

    ypos += 2;
    xpos = 2;
    setCursorPosition(xpos, ypos);
    std::cout << "key inputs: esc=quit, a=arm, t=takeoff, l=land, numpad8=forward, numpad6=right, numpad2=back, numpad4=left  ";
    ypos += 1;
    setCursorPosition(xpos, ypos);
    std::cout << "upArrow=up, downArrow=down, rightArrow=yawRight, leftArrow=yawleft    ";
    std::cout << "\n\r";
}



int main(int argc, char* argv[])
{
    CrazyFlie cf; 
    ControllerState cs;

    bool connected = false; 
    bool needsScreenClear = true;

    std::string path;
    if (argc > 0)
    {
        path = argv[0];
        size_t off = path.find_last_of('\\');
        if (off != std::string::npos)
        {
            path.resize(off);
        }
        else
        {
            path.clear();
        }
    }
    cf.defaultDirectory = path;

   if (cf.connect())
    {
        bool running = true; 
        while (running)
        {
            if (cf.setupComplete)
            {
                connected = true;
                
                if (cf.flowDeckPresent)
                {
                    if (needsScreenClear)
                    {
                        clearScreen(); // Clear screen and reset cursor position
                        needsScreenClear = false;
                    }
                    cs.updateControllerState(cf);
                    drawScreen(cf, cs);

                    handleKeyInput(cf, cs);
                }
                handleQuit(cs, running);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(32));
        }
        cf.disconnect();
    }
    {
       messageOut << "Unable to connect\n\r";
    }
}

