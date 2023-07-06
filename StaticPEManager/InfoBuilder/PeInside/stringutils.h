#include <iostream>
#include <filesystem>
#include <string>
#include <sstream>
#include <codecvt>
#include "md5.h"
#include "openssl/sha.h"

std::string HexEncode(const void* bytes, size_t size);
std::string GetMd5(unsigned char* buffer, unsigned int buf_size);
std::string GetSha256(std::wstring buffer_string);
std::string toLower(std::string str);
std::string toUpper(std::string str);
bool startsWith(const std::string& str, const std::string& withWhat);
bool endsWith(const std::string& str, const std::string& withWhat);
bool areEqualCaseInsensitive(const std::string& str1, const std::string& str2);
bool isShorterPrefixOfCaseInsensitive(const std::string& str1,
	const std::string& str2);
bool isComposedOnlyOfChars(const std::string& str, const std::string& chars);
bool containsAnyOfChars(const std::string& str, const std::string& chars);
bool contains(const std::string& str, const std::string& sub);

template<typename T>
bool areEqualFPWithEpsilon(const T& x, const T& y, const T& epsilon);

template<typename T>
bool areEqual(const T& x, const T& y);

// Specialization for floats.
template<>
bool areEqual<float>(const float& x, const float& y);

// Specialization for doubles.
template<>
bool areEqual<double>(const double& x, const double& y);

// Specialization for long doubles.
template<>
bool areEqual<long double>(const long double& x, const long double& y);

//template<typename I>
//std::string intToHexString(I w, bool addBase, unsigned fillToN);
//
//template<typename N>
//bool strToNum(const std::string& str, N& number,
//	std::ios_base& (*format)(std::ios_base&));

bool hasSubstringOnPosition(const std::string& str,
	const std::string& withWhat, std::string::size_type position);

bool hasSubstringInArea(const std::string& str, const std::string& withWhat,
	std::string::size_type start, std::string::size_type stop);

bool isValidSignaturePattern(const std::string& pattern);

std::string extractVersion(const std::string& input);

void separateStrings(std::string& str, std::vector<std::string>& words);

bool findAutoIt(const std::string& content);

std::string toWide(const std::string& str, std::string::size_type length);

double getEntropy(std::string_view bytes);

std::string readNullTerminatedAscii(const std::uint8_t* bytes, std::size_t bytesLen, std::size_t offset,
	std::size_t maxBytes, bool failOnExceed);

unsigned int byteSwap32(unsigned int val);
