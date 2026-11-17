#include <cstdint>
#include <iostream>
#include <string>
#include <stdexcept>

using namespace std;
int hexDigit(char c)
{
	if (c >= '0' && c <= '9')
	{
		return c - '0';
	}
	if (c >= 'A' && c <= 'F')
	{
		return c - 'A' + 10;
	}
	if (c >= 'a' && c <= 'f')
	{
		return c - 'a' + 10;
	}
	return -1;
}

class BigInt
{
private:
	static constexpr int BITS = 2048;
	static constexpr int w = 32;
	static constexpr int b = BITS / w;
	uint32_t A[b] = { 0 };

public:
	BigInt() {};
	BigInt(const string& hexInput);
	BigInt(const uint32_t x);
	BigInt operator+ (const BigInt& B) const;
	bool operator== (const BigInt& B) const;
	bool operator!= (const BigInt& B) const;
	bool operator< (const BigInt& B) const;
	bool operator<= (const BigInt& B) const;
	bool operator> (const BigInt& B) const;
	bool operator>= (const BigInt& B) const;
	string toHex() const;
	int cmp (const BigInt& B) const;
};

BigInt::BigInt(const string& hexInput)
{
	string hex = hexInput;

	if (hex.size() >= 2 && hex[0] == '0' && (hex[1] == 'x' || hex[1] == 'X'))
	{
		hex = hex.substr(2);
	}

	size_t first = hex.find_first_not_of('0');
	hex = (first == string::npos) ? "" : hex.substr(first);

	if (hex.size() > (size_t)b * 8)
	{
		throw overflow_error("hex string is longer than BigInt capacity");
	}

	int n = (int)hex.size();
	for (int i = 0; i < b; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			int pos = n - 1 - (i * 8 + j);

			if (pos < 0)
			{
				return;
			}

			int d = hexDigit(hex[pos]);

			if (d == -1)
			{
				throw invalid_argument("bad hex digit");
			}

			A[i] |= (uint32_t)d << (4 * j);
		}

	}
}

BigInt::BigInt(const uint32_t x)
{
	A[0] = x;
}

int BigInt::cmp (const BigInt& B) const
{
	for (int i = b - 1; i >= 0; i--)
	{
		if (A[i] > B.A[i])
		{
			return 1;
		}
		if (A[i] < B.A[i])
		{
			return -1;
		}
	}
	return 0;
}

bool BigInt::operator== (const BigInt& B) const
{
	return cmp(B) == 0;
}

bool BigInt::operator!= (const BigInt& B) const
{
	return cmp(B) != 0;
}

bool BigInt::operator< (const BigInt& B) const
{
	return cmp(B) == -1;
}

bool BigInt::operator<= (const BigInt& B) const
{
	return cmp(B) <= 0;
}

bool BigInt::operator> (const BigInt& B) const
{
	return cmp(B) == 1;
}

bool BigInt::operator>= (const BigInt& B) const
{
	return cmp(B) >= 0;
}

BigInt BigInt::operator+ (const BigInt& B) const
{
	BigInt C;
	uint64_t carry = 0;
	for (int i = 0; i < b; i++)
	{
		uint64_t temp = uint64_t(this->A[i]) + uint64_t(B.A[i]) + carry;
		C.A[i] = temp & ((uint32_t)-1);
		carry = temp >> w;
	}
	return C;
}

string BigInt::toHex() const
{
	const char* digits = "0123456789ABCDEF";
	string result;

	for (int i = b - 1; i >= 0; i--)
	{
		for (int j = 7; j >= 0; j--)
		{
			uint32_t d = (A[i] >> (4 * j)) & 0xF;
			result += digits[d];
		}
	}
	size_t first = result.find_first_not_of('0');
	result = (first == string::npos) ? "0" : result.substr(first);
	return result;
}

int main()
{
	cout << boolalpha;
	cout << BigInt("FFFFFFFF").toHex() << endl;  
	cout << (BigInt("FFFFFFFF") + BigInt("1")).toHex() << endl;
	cout << BigInt("0x00ABC").toHex() << endl;
	cout << BigInt("").toHex() << endl;
	cout << BigInt("1234ABCD9876FFFF").toHex() << endl;

	cout << BigInt(0).toHex() << endl;
	cout << BigInt(1).toHex() << endl;
	cout << BigInt(255).toHex() << endl;
	cout << BigInt(4294967295u).toHex() << endl;
	cout << (BigInt(4294967295u) + BigInt(1)).toHex() << endl;

	BigInt x("100000000");
	BigInt y("FFFFFFFF");

	cout << x.cmp(y) << endl;
	cout << y.cmp(x) << endl;
	cout << x.cmp(x) << endl;

	cout << (x > y)  << endl;
	cout << (x < y)  << endl;
	cout << (x == y) << endl;
	cout << (x != y) << endl;
	cout << (x >= x) << endl;
	cout << (x <= x) << endl;
	cout << (BigInt(0) == BigInt("")) << endl;
	cout << (BigInt(5) == BigInt("0x0005")) << endl;
	cout << (BigInt("FFFFFFFF") + 1 == BigInt("100000000")) << endl;


	return 0;
}
