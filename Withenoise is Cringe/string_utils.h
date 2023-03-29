#pragma once
#include <string>

namespace str {
	bool isLetter(char ch);

	bool isUppercase(char ch);

	bool isPrintable(char ch);

	void toLowercase(std::string& str);

	bool startswith(std::string& str, std::string prefix);

	bool endswith(std::string& str, std::string ending);

	// Returns 1 if string "a" is lexicographically
	// smaller than string "b", -1 if bigger, and 0 is equal
	int compare(std::string a, std::string b);

	void reverse(std::string& str);

	std::string intToString(int x);
}