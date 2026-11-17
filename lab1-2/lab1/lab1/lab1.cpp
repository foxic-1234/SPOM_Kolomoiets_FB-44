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

	cout << (BigInt("100000000") - BigInt(1)).toHex() << endl;
	cout << (BigInt(10) - BigInt(3)).toHex() << endl;
	cout << (BigInt("ABCDEF") - BigInt("ABCDEF")).toHex() << endl;
	cout << (BigInt("1" + string(100, '0')) - 1).toHex() << endl;

	BigInt a("123456789ABCDEF0123456789ABCDEF");
	BigInt c("FEDCBA9876543210");
	cout << ((a + c) - c == a) << endl;
	cout << ((a - c) + c == a) << endl;

	try
	{
		BigInt r = BigInt(5) - BigInt(7);
		cout << "ERROR: no exception, got " << r.toHex() << endl;
	}
	catch (const underflow_error& e)
	{
		cout << "OK, exception: " << e.what() << endl;
	}

	BigInt five(5);
	cout << five.getBit(0) << " " << five.getBit(1) << " " << five.getBit(2) << endl;
	BigInt s;
	s.setBit(100);
	cout << (s == BigInt("1" + string(25, '0'))) << endl;
	cout << BigInt(0).bitLength() << " " << BigInt(1).bitLength() << " "
	     << BigInt(255).bitLength() << " " << BigInt("100000000").bitLength() << endl;
	cout << (BigInt(1).shiftLeft(100) == s) << endl;
	cout << BigInt("FFFFFFFF").shiftLeft(4).toHex() << endl;
	cout << BigInt("FFFFFFFF").shiftLeft(32).toHex() << endl;
	cout << BigInt("FFFFFFFF").shiftLeft(36).toHex() << endl;
	cout << BigInt(1).shiftLeft(2047).bitLength() << endl;
	cout << BigInt(1).shiftLeft(2048).toHex() << endl;
	cout << BigInt("FFFFFFFF0").shiftRight(4).toHex() << endl;
	cout << BigInt("123456789ABCDEF").shiftRight(8).toHex() << endl;
	cout << BigInt("123456789ABCDEF").shiftRight(32).toHex() << endl;
	cout << BigInt("123456789ABCDEF").shiftRight(64).toHex() << endl;
	BigInt t("123456789ABCDEF0123456789ABCDEF");
	cout << (t.shiftLeft(77).shiftRight(77) == t) << endl;

	return 0;
}
