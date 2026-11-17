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
	BigInt(const uint32_t x);
	BigInt(const string& hexInput);

	string toHex() const;

	bool getBit(int k) const;
	void setBit(int k);
	int bitLength() const;
	BigInt shiftLeft(int k) const;
	BigInt shiftRight(int k) const;

	int cmp (const BigInt& B) const;
	bool operator== (const BigInt& B) const;
	bool operator!= (const BigInt& B) const;
	bool operator< (const BigInt& B) const;
	bool operator<= (const BigInt& B) const;
	bool operator> (const BigInt& B) const;
	bool operator>= (const BigInt& B) const;

	BigInt operator+ (const BigInt& B) const;
	BigInt operator- (const BigInt& B) const;
	BigInt mulOneDigit(uint32_t d) const;
	BigInt operator* (const BigInt& B) const;
	BigInt square() const;

};

BigInt::BigInt(const uint32_t x)
{
	A[0] = x;
}

BigInt::BigInt(const string& hexInput)
{
	string hex = hexInput;

	if (hex.size() >= 2 && hex[0] == '0' && (hex[1] == 'x' || hex[1] == 'X'))
	{
		hex = hex.substr(2);
	}

	size_t first = hex.find_first_not_of('0');
	hex = (first == string::npos) ? "" : hex.substr(first);

	if (hex.size() > (size_t)b * (w / 4))
	{
		throw overflow_error("hex string is longer than BigInt capacity");
	}

	int n = (int)hex.size();
	for (int i = 0; i < b; i++)
	{
		for (int j = 0; j < (w / 4); j++)
		{
			int pos = n - 1 - (i * (w / 4) + j);

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

string BigInt::toHex() const
{
	const char* digits = "0123456789ABCDEF";
	string result;

	for (int i = b - 1; i >= 0; i--)
	{
		for (int j = (w / 4) - 1; j >= 0; j--)
		{
			uint32_t d = (A[i] >> (4 * j)) & 0xF;
			result += digits[d];
		}
	}
	size_t first = result.find_first_not_of('0');
	result = (first == string::npos) ? "0" : result.substr(first);
	return result;
}

bool BigInt::getBit(int k) const
{
	if (k < 0 || k >= BITS)
	{
		throw out_of_range("bit index out of range");
	}
	return (A[k / w] >> (k % w)) & 1;
}

void BigInt::setBit(int k)
{
	if (k < 0 || k >= BITS)
	{
		throw out_of_range("bit index out of range");
	}
	A[k / w] |= (uint32_t)1 << (k % w);
}

int BigInt::bitLength() const
{
	for (int i = b - 1; i >= 0; i--)
	{
		if (A[i] != 0)
		{
			uint32_t x = A[i];
			int len = 0;
			while (x != 0)
			{
				x >>= 1;
				len++;
			}
			return i * w + len;
		}
	}
	return 0;
}

BigInt BigInt::shiftLeft(int k) const
{
	if (k < 0)
	{
		throw invalid_argument("negative shift");
	}

	BigInt C;
	if (k >= BITS)
	{
		return C;
	}

	int wordShift = k / w;
	int bitShift = k % w;

	for (int i = 0; i < b; i++)
	{
		int src = i - wordShift;
		if (src >= 0)
		{
			C.A[i] = A[src] << bitShift;
			if (bitShift != 0 && src - 1 >= 0)
			{
				C.A[i] |= A[src - 1] >> (w - bitShift);
			}
		}
	}
	return C;
}

BigInt BigInt::shiftRight(int k) const
{
	if (k < 0)
	{
		throw invalid_argument("negative shift");
	}

	BigInt C;
	if (k >= BITS)
	{
		return C;
	}

	int wordShift = k / w;
	int bitShift = k % w;

	for (int i = 0; i < b; i++)
	{
		int src = i + wordShift;
		if (src < b)
		{
			C.A[i] = A[src] >> bitShift;
			if (bitShift != 0 && src + 1 < b)
			{
				C.A[i] |= A[src + 1] << (w - bitShift);
			}
		}
	}
	return C;
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
		uint64_t temp = uint64_t(A[i]) + uint64_t(B.A[i]) + carry;
		C.A[i] = temp & ((uint32_t)-1);
		carry = temp >> w;
	}
	return C;
}

BigInt BigInt::operator- (const BigInt& B) const
{
	BigInt C;
	int64_t borrow = 0;
	for (int i = 0; i < b; i++)
	{
		int64_t temp = (int64_t)A[i] - B.A[i] - borrow;
		if (temp < 0)
		{
			temp += 0x100000000LL;
			borrow = 1;
		}
		else
		{
			borrow = 0;
		}

		C.A[i] = temp & ((uint32_t)-1);
	}
	if (borrow != 0)
	{
		throw underflow_error("result of subtraction is negative");
	}
	return C;
}

BigInt BigInt::mulOneDigit(uint32_t d) const
{
	BigInt C;
	uint64_t carry = 0;
	for (int i = 0; i < b; i++)
	{
		uint64_t temp = (uint64_t)A[i] * d + carry;
		C.A[i] = (uint32_t)temp;
		carry = temp >> w;
	}
	return C;
}

BigInt BigInt::operator* (const BigInt& B) const
{
	BigInt C;
	for (int i = 0; i < b; i++)
	{
		if (B.A[i] == 0)
		{
			continue;
		}
		BigInt temp = mulOneDigit(B.A[i]);
		temp = temp.shiftLeft(i * w);
		C = C + temp;
	}
	return C;
}

BigInt BigInt::square() const
{
	int n = (bitLength() + w - 1) / w;

	if (n <= 8 || n > b / 2)
	{
		return *this * *this;
	}

	int m = n / 2;
	BigInt x1 = shiftRight(m * w);
	BigInt x0 = *this - x1.shiftLeft(m * w);

	BigInt z0 = x0.square();
	BigInt z2 = x1.square();
	BigInt z1 = (x0 + x1).square() - z0 - z2;

	return z2.shiftLeft(2 * m * w) + z1.shiftLeft(m * w) + z0;
}

int testsPassed = 0;
int testsFailed = 0;

void check(const string& name, bool ok)
{
	cout << (ok ? "[ OK ] " : "[FAIL] ") << name << endl;
	if (ok)
	{
		testsPassed++;
	}
	else
	{
		testsFailed++;
	}
}

bool subtractionThrows()
{
	try
	{
		BigInt(5) - BigInt(7);
		return false;
	}
	catch (const underflow_error&)
	{
		return true;
	}
}

void runTests()
{
	BigInt a("123456789ABCDEF0123456789ABCDEF");
	BigInt c("FEDCBA9876543210");
	BigInt d("ABCDEF0123456789");
	BigInt x("100000000");
	BigInt y("FFFFFFFF");

	cout << "--- Hex conversion and constants ---" << endl;
	check("hex round trip", BigInt("1234ABCD9876FFFF").toHex() == "1234ABCD9876FFFF");
	check("0x prefix and leading zeros", BigInt("0x00ABC").toHex() == "ABC");
	check("empty string is zero", BigInt("").toHex() == "0");
	check("small constant", BigInt(255).toHex() == "FF");

	cout << "--- Comparison ---" << endl;
	check("2^32 > 2^32 - 1", x > y && y < x);
	check("equal / not equal", x == x && x != y);
	check("<= and >= on equal numbers", x <= x && x >= x);

	cout << "--- Addition and subtraction ---" << endl;
	check("carry between words", y + 1 == x);
	check("borrow between words", x - 1 == y);
	check("(a + c) - c == a", (a + c) - c == a);
	check("negative result throws", subtractionThrows());

	cout << "--- Bits and shifts ---" << endl;
	check("getBit of 5 = 101", BigInt(5).getBit(0) && !BigInt(5).getBit(1) && BigInt(5).getBit(2));
	check("bitLength(2^32) == 33", x.bitLength() == 33);
	check("shiftLeft across words", y.shiftLeft(36).toHex() == "FFFFFFFF000000000");
	check("shiftLeft then shiftRight", a.shiftLeft(77).shiftRight(77) == a);
	check("shift out of range gives 0", BigInt(1).shiftLeft(2048) == BigInt(0));

	cout << "--- Multiplication ---" << endl;
	check("FFFFFFFF * FFFFFFFF", (y * y).toHex() == "FFFFFFFE00000001");
	check("a * c (checked with Python)", (a * c).toHex() == "121FA00AD77D7422358D29092D964322236D88FE5618CF0");
	check("a * c == c * a", a * c == c * a);
	check("(a + c) * d == a * d + c * d", (a + c) * d == a * d + c * d);

	BigInt sum100;
	for (int i = 0; i < 100; i++)
	{
		sum100 = sum100 + a;
	}
	check("a * 100 == a + a + ... + a", a * 100 == sum100);

	cout << "--- Square (Karatsuba) ---" << endl;
	BigInt big(string(256, 'F'));
	check("square == a * a (small)", a.square() == a * a);
	check("square == a * a (1024 bits)", big.square() == big * big);

	cout << endl << "Passed: " << testsPassed << ", failed: " << testsFailed << endl;
}

int main()
{
	runTests();
	return 0;
}
