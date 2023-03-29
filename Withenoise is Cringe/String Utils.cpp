#include "string_utils.h"

namespace str {
	bool isLetter(char ch) {
		if (ch >= 'A' && ch <= 'Z' || ch >= 'a' && ch <= 'z')
			return true;
		return false;
	}

	bool isUppercase(char ch) {
		if (ch >= 'A' && ch <= 'Z')
			return true;
		return false;
	}

	bool isPrintable(char ch) {
		if (ch < 32)
			return false;
		return true;
	}

	void toLowercase(std::string & str) {
		for (size_t i = 0; i < str.size(); ++i) {
			if (isUppercase(str[i]))
				str[i] = str[i] + 32;
		}
	}

	bool startswith(std::string& str, std::string prefix) {
		if (prefix.size() > str.size())
			return false;

		for (size_t i = 0; i < prefix.size(); i++) {
			if (prefix[i] != str[i])
				return false;
		}
		return true;
	}

	bool endswith(std::string& str, std::string ending) {
		if (ending.size() > str.size())
			return false;

		size_t j = (int)str.size() - (int)ending.size();
		for (size_t i = 0; i < ending.size(); i++, j++) {
			if (ending[i] != str[j])
				return false;
		}
		return true;
	}

	int compare(std::string a, std::string b) {
		for (size_t i = 0; i < a.size() && i < b.size(); i++)
			if (a[i] < b[i])
				return 1;
			else if (b[i] < a[i])
				return -1;

		if (a.size() > b.size())
			return -1;
		else if (a.size() == b.size())
			return 0;
		else
			return 1;

	}

	void reverse(std::string& str) {
		int len = str.size();
		for (int i = 0; i < len / 2; i++)
			std::swap(str[i], str[len - i - 1]);
	}

	std::string intToString(int x) {
		std::string res = "";
		bool negative = x < 0;
		do {
			res += (x % 10 + '0');
			x /= 10;
		} while (x != 0);

		if (negative)
			res += '-';

		reverse(res);
		return res;
	}
}