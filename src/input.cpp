#include "includes.hpp"
#include "input.hpp"

void input::keyChanged(sf::Keyboard::Key key, bool value)
{
    switch (key)
    {
        case sf::Keyboard::A: inputState = setBit(inputState, 0, value); break;      //A
        case sf::Keyboard::S: inputState = setBit(inputState, 1, value); break;      //B
        case sf::Keyboard::RShift : inputState = setBit(inputState, 2, value); break; //Select
        case sf::Keyboard::Return: inputState = setBit(inputState, 3, value); break; //Start
        case sf::Keyboard::Up: inputState = setBit(inputState, 4, value); break;     //Up
        case sf::Keyboard::Down: inputState = setBit(inputState, 5, value); break;   //Down
        case sf::Keyboard::Left: inputState = setBit(inputState, 6, value); break;   //Left
        case sf::Keyboard::Right: inputState = setBit(inputState, 7, value); break;  //Right
    }
    if (strobe)
    {
        inputShiftRegister = inputState;
    }
}

void input::registerWrite(byte value)
{
    strobe = getBit(value, 0);
    if (strobe)
    {
        inputShiftRegister = inputState;
    }
}

byte input::registerRead()
{
    bool val = getBit(inputShiftRegister, 0);
    if (!strobe)
    {
        inputShiftRegister >>= 1;
    }
    return (byte)val;
}