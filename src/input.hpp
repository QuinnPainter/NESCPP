#pragma once

class input
{
    public:
        void keyChanged(sf::Keyboard::Key key, bool value);
        void registerWrite(byte value);
        byte registerRead();
    private:
        byte inputState = 0;
        byte inputShiftRegister = 0;
        bool strobe = 1;
};