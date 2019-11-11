#pragma once

std::string byteToString(byte num);
std::string ushortToString(ushort num);
ushort combineBytes(byte b1, byte b2);
byte highByte(ushort s);
byte lowByte(ushort s);
//byte setBit(byte b, byte index, bool value);
//bool getBit(byte b, byte index);
byte reverseByte(byte b);

//returns b, and sets the bit with index to value
//bits go from right to left
template <typename T>
inline T setBit(T b, int index, bool value)
{
    T ret = b & ~(1 << index);
    ret |= value << index;
    return ret;
}

//returns bit with index from a byte
//bits go from right to left
template <typename T>
inline bool getBit(T b, int index)
{
    return (b & (1 << index));
}