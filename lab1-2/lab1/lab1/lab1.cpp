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
	static constexpr int b = 64;
	uint32_t A[b] = { 0 };
	int w = 32;

public:
	BigInt() {};
	BigInt(string hex);
	BigInt operator+ (const BigInt& B);
};

BigInt::BigInt(string hex)
{
	int n = hex.size();
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

BigInt BigInt::operator+ (const BigInt& B)
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

int main()
{
	return 0;
}
