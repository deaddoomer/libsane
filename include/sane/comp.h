#ifndef __sane_comp_h__
#define __sane_comp_h__

#include <cmath>
#include <string>
#include <cstdlib>
#include <cstdint>

	// comp is an int64_t but 0x8000_0000_0000_0000 is NaN
	//typedef int64_t comp;

namespace SANE {

	struct comp {

	public:
		constexpr static const uint64_t NaN = 0x8000000000000000;

		comp(const comp &rhs) = default;

		explicit comp(int rhs) : _data(rhs) {}
		explicit comp(int64_t rhs) : _data(rhs) {}
		explicit comp(uint64_t rhs) : _data(rhs) {}


		explicit comp(float rhs) { read_from(rhs); }
		explicit comp(double rhs) { read_from(rhs); }
		explicit comp(long double rhs) { read_from(rhs); }



		comp &operator=(const comp &rhs) = default;

		comp &operator=(uint64_t rhs) {
			_data = rhs;
			return *this;
		}


		comp &operator=(float rhs) {
			read_from(rhs);
			return *this;
		}

		comp &operator=(double rhs) {
			read_from(rhs);
			return *this;
		}
		comp &operator=(long double rhs) {
			read_from(rhs);
			return *this;
		}


		explicit operator uint64_t() const {
			return _data;
		}

		explicit operator int64_t() const {
			return _data;
		}

		explicit operator long double() const {
			if (_data == NaN)
				return NAN;
			return _data;
		}

		explicit operator double() const {
			if (_data == NaN)
				return NAN;
			return _data;
		}

		explicit operator float() const {
			if (_data == NaN)
				return NAN;
			return _data;
		}

		bool operator==(const comp &rhs) {
			if (isnan(*this) || isnan(rhs)) return false;
			return _data == rhs._data;
		}

		bool operator!=(const comp &rhs) {
			if (isnan(*this) || isnan(rhs)) return true;
			return _data != rhs._data;
		}



	private:
		int64_t _data = 0;

		template <class T>
		void read_from(T t) {

			switch (std::fpclassify(t)) {
				case FP_NAN:
				case FP_INFINITE:
					_data = NaN;
					break;
				default:
					if (t > INT64_MAX || t < -INT64_MAX) {
						_data = NaN;
					} else {
						_data = t;
					}
					break;
			}
		}

		friend std::string to_string(const comp &);
		friend int fpclassify(const comp &);
		friend int signbit(const comp &);
		friend int isnan(const comp &);
		friend int isinf(const comp &);
		friend int isfinite(const comp &);
		friend int isnormal(const comp &);
		friend comp abs(const comp &);
	};



	std::string to_string(const comp &c) {

		if (isnan(c)) return std::string("nan");
		return std::to_string((int64_t)c);
	}

	inline int fpclassify(const comp &c) {
		if (isnan(c)) return FP_NAN;
		if ((uint64_t)c == (uint64_t)0) return FP_ZERO;
		return FP_NORMAL;
	}

	inline int signbit(const comp &c) {
		if (isnan(c)) return 0;
		return ((int64_t)c < (int64_t)0) ? 1 : 0;
	}

	inline int isnan(const comp &c) {
		return c._data == comp::NaN;
	}

	inline int isinf(const comp &c) {
		return false;
	}

	inline int isfinite(comp c) {
		if (isnan(c)) return false;
		return true;
	}

	inline int isnormal(const comp &c) {
		if (isnan(c)) return false;
		if ((uint64_t)c == 0) return false;
		return true;
	}

	inline comp abs(const comp &c) {
		if (isnan(c)) return c;
		return comp(std::abs((int64_t)c));
	}
}

#endif
