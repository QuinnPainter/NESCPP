#include "includes.hpp"
#include "helpers.hpp"

/*
template <typename T>
std::string NumberToString ( T Number )
{
    std::ostringstream ss;
    ss << std::hex << Number;
    return ss.str();
}
*/

//converts byte to hex string ex. 255 becomes "FF"
std::string byteToString(byte num)
{
    std::ostringstream ss;
    ss.width(2);
    ss.fill('0');
    ss << std::hex << std::uppercase << (int)num;
    return ss.str();
}

//converts short to hex string ex. 65535 becomes "FFFF"
std::string ushortToString(ushort num)
{
    std::ostringstream ss;
    ss.width(4);
    ss.fill('0');
    ss << std::hex << std::uppercase << (int)num;
    return ss.str();
}

//makes 2 bytes into a big-endian short ex. 55 and 9A become 559A
ushort combineBytes(byte b1, byte b2)
{
    return (((ushort)b1 << 8) | b2);
}

//returns the first byte of a short eg. 559A becomes 55
byte highByte(ushort s)
{
    return s >> 8;
}

//returns the second byte of a short eg. 559A becomes 9A
byte lowByte(ushort s)
{
    return s & 0xFF;
}

/*
//returns b, and sets the bit with index to value
//bits go from right to left, 0 to 7
byte setBit(byte b, byte index, bool value)
{
    if (index > 7)
    {
        logging::logerr("setBit index cannot be greater than 7!");
    }
    byte ret = b & ~(1 << index);
    ret |= value << index;
    return ret;
}

//returns bit with index from a byte
//bits go from right to left, 0 to 7
bool getBit(byte b, byte index)
{
    if (index > 7)
    {
        logging::logerr("getBit index cannot be greater than 7!");
    }
    //return ((b & (1 << index)) == (1 << index));
    return (b & (1 << index));
}
*/

//Bitwise reverses a byte eg. 01 becomes 80
byte reverseByte(byte b) {
    return ((b & 0x1)  << 7) | ((b & 0x2)  << 5) |
           ((b & 0x4)  << 3) | ((b & 0x8)  << 1) |
           ((b & 0x10) >> 1) | ((b & 0x20) >> 3) |
           ((b & 0x40) >> 5) | ((b & 0x80) >> 7);
}