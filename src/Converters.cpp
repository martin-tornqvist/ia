#include "Converters.h"

#include <string>
#include <sstream>
#include <iostream>

std::string intToString(int in)
{
	std::ostringstream buffer;
	buffer << in;
	return buffer.str();
}

int stringToInt(std::string in)
{
	int _nr;
	std::istringstream buffer(in);
	buffer >> _nr;
	return _nr;
}

//int charArrayToInt(char* array)
//{
//    int val = 0;
//    int integer = 0;
//    int step    = 1;
//
////    std::cout << "array[0] and [1] in converter: " << array[0] << array[1] << std::endl;
////
////    for (int i = 255; i >= 0; i--) {
////
////        val = array[i];
////        //val = 1;
////        std::cout << "val         : " << val << std::endl;
////
////        if ( (val >= 0 && val <= 9) || val == '-') {
////            if (val != '-') {
////
////                for (int ii = 1; ii < step; ii++)
////                    val *= 10;
////
////                integer += val;
////                std::cout << "integer now : " << integer << std::endl;
////                std::cout << "i           : " << i << std::endl;
////            }
////            else {
////                integer *= -1;
////                std::cout << "integer now : " << integer << std::endl;
////                std::cout << "i           : " << i << std::endl;
////            }
////
////            step++;
////        }
////    }
//    integer = atoi(array);
//
//    std::cout << "integer from converter: " << integer << std::endl;
//
//    return integer;
//}

/*
char* stringToCharStar(std::string in)
    {
    const char* out;
    out = in.data();
    return out;
    }
*/

/*
Uint16 charToUint16(char in)
    {
    return (Uint16)in;
    }
*/
