#include <cstdint>
#include <iostream>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <random>
#include <chrono>
#include <vector>
#include <iomanip>

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

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
	BigInt() {}
	BigInt(const uint32_t x);
	BigInt(const string& hexInput);

	string toHex() const;
	string toDecimal() const;
	string toBinary() const;
	static BigInt fromDecimal(const string& dec);
	static BigInt fromBinary(const string& binInput);

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
	BigInt divOneDigit(uint32_t d, uint32_t& remainder) const;
	BigInt operator* (const BigInt& B) const;
	BigInt square() const;
	void divMod(const BigInt& B, BigInt& Q, BigInt& R) const;
	BigInt operator/ (const BigInt& B) const;
	BigInt operator% (const BigInt& B) const;
	BigInt pow(const BigInt& B) const;
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

string BigInt::toDecimal() const
{
	if (*this == BigInt(0))
	{
		return "0";
	}

	string result;
	BigInt x = *this;
	while (x != BigInt(0))
	{
		uint32_t chunk;
		x = x.divOneDigit(1000000000, chunk);
		for (int k = 0; k < 9; k++)
		{
			result += (char)('0' + chunk % 10);
			chunk /= 10;
		}
	}

	while (result.size() > 1 && result.back() == '0')
	{
		result.pop_back();
	}
	reverse(result.begin(), result.end());
	return result;
}

string BigInt::toBinary() const
{
	int len = bitLength();
	if (len == 0)
	{
		return "0";
	}

	string result;
	for (int i = len - 1; i >= 0; i--)
	{
		result += getBit(i) ? '1' : '0';
	}
	return result;
}

BigInt BigInt::fromDecimal(const string& dec)
{
	BigInt result;
	for (char ch : dec)
	{
		if (ch < '0' || ch > '9')
		{
			throw invalid_argument("bad decimal digit");
		}

		BigInt next = result.mulOneDigit(10) + BigInt((uint32_t)(ch - '0'));

		uint32_t rem;
		if (next.divOneDigit(10, rem) != result)
		{
			throw overflow_error("decimal string is longer than BigInt capacity");
		}
		result = next;
	}
	return result;
}

BigInt BigInt::fromBinary(const string& binInput)
{
	string bin = binInput;

	if (bin.size() >= 2 && bin[0] == '0' && (bin[1] == 'b' || bin[1] == 'B'))
	{
		bin = bin.substr(2);
	}

	size_t first = bin.find_first_not_of('0');
	bin = (first == string::npos) ? "" : bin.substr(first);

	if (bin.size() > (size_t)BITS)
	{
		throw overflow_error("binary string is longer than BigInt capacity");
	}

	BigInt result;
	int n = (int)bin.size();
	for (int i = 0; i < n; i++)
	{
		char ch = bin[(size_t)(n - 1 - i)];
		if (ch == '1')
		{
			result.setBit(i);
		}
		else if (ch != '0')
		{
			throw invalid_argument("bad binary digit");
		}
	}
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

// Long division by one digit, from the highest word to the lowest
BigInt BigInt::divOneDigit(uint32_t d, uint32_t& remainder) const
{
	if (d == 0)
	{
		throw invalid_argument("division by zero");
	}

	BigInt Q;
	uint64_t rem = 0;
	for (int i = b - 1; i >= 0; i--)
	{
		uint64_t cur = (rem << w) | A[i];
		Q.A[i] = (uint32_t)(cur / d);
		rem = cur % d;
	}
	remainder = (uint32_t)rem;
	return Q;
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

void BigInt::divMod(const BigInt& B, BigInt& Q, BigInt& R) const
{
	if (B == BigInt(0))
	{
		throw invalid_argument("division by zero");
	}

	int k = B.bitLength();
	R = *this;
	Q = BigInt();

	while (R >= B)
	{
		int t = R.bitLength();
		BigInt C = B.shiftLeft(t - k);
		if (R < C)
		{
			t--;
			C = B.shiftLeft(t - k);
		}
		R = R - C;
		Q.setBit(t - k);
	}
}

BigInt BigInt::operator/ (const BigInt& B) const
{
	BigInt Q, R;
	divMod(B, Q, R);
	return Q;
}

BigInt BigInt::operator% (const BigInt& B) const
{
	BigInt Q, R;
	divMod(B, Q, R);
	return R;
}

BigInt BigInt::pow(const BigInt& B) const
{
	BigInt C(1);
	for (int i = B.bitLength() - 1; i >= 0; i--)
	{
		if (B.getBit(i))
		{
			C = C * *this;
		}
		if (i != 0)
		{
			C = C.square();
		}
	}
	return C;
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

bool divisionByZeroThrows()
{
	try
	{
		BigInt(5) / BigInt(0);
		return false;
	}
	catch (const invalid_argument&)
	{
		return true;
	}
}

BigInt randomBigInt(int bits, mt19937& rng)
{
	BigInt x;
	for (int i = 0; i < bits - 1; i++)
	{
		if (rng() & 1)
		{
			x.setBit(i);
		}
	}
	x.setBit(bits - 1);
	return x;
}

void runTests()
{
	BigInt a("123456789ABCDEF0123456789ABCDEF");
	BigInt c("FEDCBA9876543210");
	BigInt x("100000000");
	BigInt y("FFFFFFFF");

	cout << "--- Hex conversion and constants ---" << endl;
	check("hex round trip", BigInt("1234ABCD9876FFFF").toHex() == "1234ABCD9876FFFF");
	check("0x prefix and leading zeros", BigInt("0x00ABC").toHex() == "ABC");
	check("empty string is zero", BigInt("").toHex() == "0");
	check("small constant", BigInt(255).toHex() == "FF");

	cout << "--- Decimal and binary conversion ---" << endl;
	BigInt pow2_100 = BigInt(1).shiftLeft(100);
	check("toDecimal(2^100)", pow2_100.toDecimal() == "1267650600228229401496703205376");
	check("fromDecimal(2^100)", BigInt::fromDecimal("1267650600228229401496703205376") == pow2_100);
	check("toDecimal(0) == \"0\"", BigInt(0).toDecimal() == "0");
	check("toBinary(5) == \"101\"", BigInt(5).toBinary() == "101" && BigInt(0).toBinary() == "0");
	check("fromBinary(\"0b1101\") == 13", BigInt::fromBinary("0b1101") == BigInt(13));

	cout << "--- Comparison ---" << endl;
	check("2^32 > 2^32 - 1", x > y && y < x);
	check("equal / not equal", x == x && x != y);
	check("<= and >= on equal numbers", x <= x && x >= x);

	cout << "--- Addition and subtraction ---" << endl;
	check("carry between words", y + 1 == x);
	check("borrow between words", x - 1 == y);
	check("negative result throws", subtractionThrows());

	cout << "--- Bits and shifts ---" << endl;
	check("getBit of 5 = 101", BigInt(5).getBit(0) && !BigInt(5).getBit(1) && BigInt(5).getBit(2));
	check("bitLength(2^32) == 33", x.bitLength() == 33);
	check("shiftLeft across words", y.shiftLeft(36).toHex() == "FFFFFFFF000000000");
	check("shiftLeft then shiftRight", a.shiftLeft(77).shiftRight(77) == a);
	check("shift out of range gives 0", BigInt(1).shiftLeft(2048) == BigInt(0));

	cout << "--- Multiplication ---" << endl;
	check("FFFFFFFF * FFFFFFFF", (y * y).toHex() == "FFFFFFFE00000001");
	check("a * c (reference value)", (a * c).toHex() == "121FA00AD77D7422358D29092D964322236D88FE5618CF0");

	cout << "--- Division ---" << endl;
	check("100 / 7 == 14, 100 % 7 == 2", BigInt(100) / 7 == BigInt(14) && BigInt(100) % 7 == BigInt(2));
	check("a / c (reference value)", (a / c).toHex() == "124924924924923" && (a % c).toHex() == "7F598F328CC265BF");
	check("a / a == 1, a / 1 == a", a / a == BigInt(1) && a / 1 == a);
	check("smaller / bigger == 0", c / a == BigInt(0) && c % a == c);
	check("division by zero throws", divisionByZeroThrows());

	cout << "--- Power ---" << endl;
	check("2^100 == 1 << 100", BigInt(2).pow(100) == BigInt(1).shiftLeft(100));
	check("3^200 (reference value)", BigInt(3).pow(200).toHex() ==
		"1FD5863C3EB0469EC21A937A76F3432FFD73D97E447606B683ECF6F6E4A7AE225BFAFF1EAAF8B0A1");
	check("a^0 == 1, a^1 == a, a^2 == a * a", a.pow(0) == BigInt(1) && a.pow(1) == a && a.pow(2) == a * a);
	check("0^5 == 0", BigInt(0).pow(5) == BigInt(0));
}

void printExamples()
{
	mt19937 rng(2026);
	BigInt a = randomBigInt(256, rng);
	BigInt b = randomBigInt(128, rng);

	cout << endl << "--- Examples (hex) ---" << endl;
	cout << "a       = " << a.toHex() << endl;
	cout << "b       = " << b.toHex() << endl;
	cout << "a + b   = " << (a + b).toHex() << endl;
	cout << "a - b   = " << (a - b).toHex() << endl;
	cout << "a * b   = " << (a * b).toHex() << endl;
	cout << "a ^ 2   = " << a.square().toHex() << endl;
	cout << "a / b   = " << (a / b).toHex() << endl;
	cout << "a % b   = " << (a % b).toHex() << endl;
	cout << "b ^ 3   = " << b.pow(3).toHex() << endl;
	cout << "b (dec) = " << b.toDecimal() << endl;
	cout << "b (bin) = " << b.toBinary() << endl;
}

void runRandomTests(int rounds)
{
	mt19937 rng(12345);
	int ok[8] = { 0 };

	for (int round = 0; round < rounds; round++)
	{
		BigInt a = randomBigInt(1000, rng);
		BigInt b = randomBigInt(1000, rng);
		BigInt c = randomBigInt(1000, rng);
		BigInt big = randomBigInt(2048, rng);
		uint32_t n = 100 + rng() % 200;

		BigInt left = (a + b) * c;
		if (left == c * (a + b) && left == a * c + b * c)
		{
			ok[0]++;
		}

		BigInt sum;
		for (uint32_t i = 0; i < n; i++)
		{
			sum = sum + a;
		}
		if (a * n == sum)
		{
			ok[1]++;
		}

		if ((a + b) - b == a)
		{
			ok[2]++;
		}

		BigInt ab = a * b;
		if (ab / b == a && ab % b == BigInt(0))
		{
			ok[3]++;
		}

		BigInt q, r;
		big.divMod(c, q, r);
		if (q * c + r == big && r < c)
		{
			ok[4]++;
		}

		// 6. square == a * a
		if (a.square() == a * a)
		{
			ok[5]++;
		}

		BigInt x = randomBigInt(32, rng);
		uint32_t m = rng() % 20;
		uint32_t k = rng() % 20;
		if (x.pow(m) * x.pow(k) == x.pow(m + k))
		{
			ok[6]++;
		}

		if (BigInt(big.toHex()) == big &&
			BigInt::fromDecimal(big.toDecimal()) == big &&
			BigInt::fromBinary(big.toBinary()) == big)
		{
			ok[7]++;
		}
	}

	const char* names[8] =
	{
		"(a + b) * c == c * (a + b) == a * c + b * c",
		"n * a == a + a + ... + a  (n >= 100)",
		"(a + b) - b == a",
		"(a * b) / b == a, (a * b) % b == 0",
		"big == q * c + r, r < c",
		"a.square() == a * a",
		"x^m * x^k == x^(m + k)",
		"hex / decimal / binary round trip",
	};

	cout << endl << "--- Random tests (" << rounds << " rounds) ---" << endl;
	for (int i = 0; i < 8; i++)
	{
		check(string(names[i]) + "  [" + to_string(ok[i]) + "/" + to_string(rounds) + "]", ok[i] == rounds);
	}
}

volatile uint32_t benchSink = 0;   

template <typename Op>
void measure(const string& name, int iterations, Op op)
{
	auto t0 = chrono::steady_clock::now();
	uint64_t c0 = __rdtsc();
	for (int i = 0; i < iterations; i++)
	{
		op((size_t)i);
	}
	uint64_t c1 = __rdtsc();
	auto t1 = chrono::steady_clock::now();

	double us = chrono::duration<double, micro>(t1 - t0).count() / iterations;
	double cycles = (double)(c1 - c0) / iterations;

	cout << left << setw(34) << name
		<< right << setw(14) << fixed << setprecision(3) << us
		<< setw(16) << setprecision(0) << cycles << endl;
}

void runBenchmark()
{
	const size_t N = 1000;
	mt19937 rng(777);

	vector<BigInt> a, b, big, e;
	for (size_t i = 0; i < N; i++)
	{
		a.push_back(randomBigInt(1024, rng));
		b.push_back(randomBigInt(1024, rng));
		big.push_back(randomBigInt(2048, rng));
		e.push_back(randomBigInt(256, rng));
	}

	cout << endl << "--- Benchmark (" << N << " random operands, Release build recommended) ---" << endl;
	cout << left << setw(34) << "operation"
		<< right << setw(14) << "us / op" << setw(16) << "cycles / op" << endl;

	measure("a + b        (1024 + 1024)", 200000, [&](size_t i)
	{
		benchSink = benchSink + (a[i % N] + b[i % N]).getBit(0);
	});
	measure("big - a      (2048 - 1024)", 200000, [&](size_t i)
	{
		benchSink = benchSink + (big[i % N] - a[i % N]).getBit(0);
	});
	measure("cmp          (1024 vs 1024)", 200000, [&](size_t i)
	{
		benchSink = benchSink + (uint32_t)(a[i % N].cmp(b[i % N]) + 1);
	});
	measure("shiftLeft(100)  (1024)", 200000, [&](size_t i)
	{
		benchSink = benchSink + a[i % N].shiftLeft(100).getBit(0);
	});
	measure("a * b        (1024 * 1024)", 20000, [&](size_t i)
	{
		benchSink = benchSink + (a[i % N] * b[i % N]).getBit(0);
	});
	measure("a.square()   (1024, Karatsuba)", 20000, [&](size_t i)
	{
		benchSink = benchSink + a[i % N].square().getBit(0);
	});
	measure("big / a, big % a (2048 / 1024)", 2000, [&](size_t i)
	{
		BigInt q, r;
		big[i % N].divMod(a[i % N], q, r);
		benchSink = benchSink + q.getBit(0) + r.getBit(0);
	});
	measure("a.pow(e)     (1024 ^ 256 bit)", 20, [&](size_t i)
	{
		benchSink = benchSink + a[i % N].pow(e[i % N]).getBit(0);
	});
	measure("toHex        (2048)", 20000, [&](size_t i)
	{
		benchSink = benchSink + (uint32_t)big[i % N].toHex().size();
	});
	measure("toDecimal    (2048)", 2000, [&](size_t i)
	{
		benchSink = benchSink + (uint32_t)big[i % N].toDecimal().size();
	});

	cout << defaultfloat;
	cout << "(pow results are truncated to 2048 bits, like all operations)" << endl;
}

int main()
{
	runTests();
	printExamples();
	runRandomTests(100);

	cout << endl << "Passed: " << testsPassed << ", failed: " << testsFailed << endl;

	runBenchmark();
	return 0;
}
