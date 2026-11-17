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
	BigInt operator+ (const BigInt& B) const;
	string toHex() const;
};

BigInt::BigInt(const string& hexInput)
{
	string hex = hexInput;

	// optional "0x" / "0X" prefix
	if (hex.size() >= 2 && hex[0] == '0' && (hex[1] == 'x' || hex[1] == 'X'))
	{
		hex = hex.substr(2);
	}

	// leading zeros do not affect the value
	size_t first = hex.find_first_not_of('0');
	hex = (first == string::npos) ? "" : hex.substr(first);

	// each word holds 8 hex digits
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
	string result;

	for (int i = b - 1; i >= 0; i--)
	{
		
	}

	return result;
}

int main()
{
	return 0;
}
