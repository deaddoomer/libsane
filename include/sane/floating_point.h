
#ifndef __sane_floating_point_h__
#define __sane_floating_point_h__

#include <type_traits>
#include <cstdint>
#include <cmath>
#include <cstring>
#include <utility>
#include <string>

#include "endian.h"

namespace SANE {

namespace floating_point {

#if defined(__i386__) || defined(__x86_64__) || defined(__m68k__)
	// long double is float80 (ABI specific)
	constexpr bool native_extended = sizeof(long double) >= 10;
#else
	// precission reduced down to double, but works everywhere
	constexpr bool native_extended = false;
#endif

	template<size_t size>
	void reverse_bytes(void *vp) {
		char *cp = (char *)vp;
		for (size_t i = 0; i < size / 2; ++i)
			std::swap(cp[i], cp[size - i - 1]);
	}

	template<size_t size>
	void reverse_bytes_if(void *vp, std::true_type) {
		reverse_bytes<size>(vp);
	}
	template<size_t size>
	void reverse_bytes_if(void *vp, std::false_type) {
	}

#if 0
	enum classification {
		zero,
		infinite,
		quiet_nan,
		signaling_nan,
		normal,
		subnormal
	};
#endif

	namespace half_traits {
		constexpr size_t bias = 15;
		constexpr size_t exponent_bits = 5;
		constexpr size_t significand_bits = 10;
		constexpr int max_exp = 15;
		constexpr int min_exp = -14;

		constexpr uint16_t significand_mask = ((1 << significand_bits) - 1);
		constexpr uint16_t sign_bit = UINT16_C(1) << 15;
		constexpr uint16_t nan_exp = UINT16_C(31) << significand_bits;

		constexpr uint16_t quiet_nan = UINT16_C(0x02) << (significand_bits - 2);
		constexpr uint16_t signaling_nan = UINT16_C(0x01) << (significand_bits - 2);
	}

	namespace single_traits {
		constexpr size_t bias = 127;
		constexpr size_t exponent_bits = 8;
		constexpr size_t significand_bits = 23;
		constexpr int max_exp = 127;
		constexpr int min_exp = -126;

		constexpr uint32_t significand_mask = ((1 << significand_bits) - 1);
		constexpr uint32_t sign_bit = UINT32_C(1) << 31;
		constexpr uint32_t nan_exp = UINT32_C(255) << significand_bits;

		constexpr uint32_t quiet_nan = UINT32_C(0x02) << (significand_bits - 2);
		constexpr uint32_t signaling_nan = UINT32_C(0x01) << (significand_bits - 2);
	}

	namespace double_traits {
		constexpr size_t bias = 1023;
		constexpr size_t exponent_bits = 11;
		constexpr size_t significand_bits = 52;
		constexpr int max_exp = 1023;
		constexpr int min_exp = -1022;

		constexpr uint64_t significand_mask = ((UINT64_C(1) << significand_bits) - 1);
		constexpr uint64_t sign_bit = UINT64_C(1) << 63;
		constexpr uint64_t nan_exp = UINT64_C(2047) << significand_bits;

		constexpr uint64_t quiet_nan = UINT64_C(0x02) << (significand_bits - 2);
		constexpr uint64_t signaling_nan = UINT64_C(0x01) << (significand_bits - 2);
	}

	namespace extended_traits {

		constexpr size_t bias = 16383;
		constexpr size_t exponent_bits = 15;
		constexpr size_t significand_bits = 63; // does not include explicit 1.
		constexpr int max_exp = 16383;
		constexpr int min_exp = -16382;

		constexpr uint64_t significand_mask = ((UINT64_C(1) << significand_bits) - 1);

		constexpr uint64_t quiet_nan = UINT64_C(0x02) << (significand_bits - 2);
		constexpr uint64_t signaling_nan = UINT64_C(0x01) << (significand_bits - 2);

		constexpr uint64_t one_bit = UINT64_C(0x8000000000000000);


		// stored separately.
		constexpr uint16_t sign_bit = 0x8000;
		constexpr uint16_t nan_exp = 0x7fff;

	}

	template<size_t _size, endian _byte_order>
	struct format {
		static constexpr size_t size = _size;
		static constexpr endian byte_order = _byte_order;
	};


	class info {
	private:
		void read_single(const void *);
		void read_double(const void *);
		void read_extended(const void *);


		void write_single(void *) const;
		void write_double(void *) const;
		void write_extended(void *) const;
	public:

		bool sign = false;
		bool one = false;
		int exp = 0;
		uint64_t sig = 0; // includes explicit 1 bit, adjusted to 63 bits of fraction.

		bool nan = false;
		bool inf = false;

		//classification type = zero;

		template<class T, typename = std::enable_if<std::is_floating_point<T>::value> >
		void read(T x)
		{ read(format<sizeof(x), endian::native>{}, &x); }

		void read(long double x) {
			if (native_extended) {
				read(format<sizeof(x), endian::native>{}, &x);
			} else {
				read((double)x);
			}
		}

		template<size_t size, endian byte_order>
		void read(format<size, byte_order>, const void *vp) {

			uint8_t buffer[size];
			static_assert(byte_order != endian::native, "byte order");

			std::memcpy(buffer, vp, size);
			reverse_bytes<size>(buffer);
			read(format<size, endian::native>{}, buffer);
		}

		void read(format<4, endian::native>, const void *vp) {
			read_single(vp);
		}


		void read(format<8, endian::native>, const void *vp) {
			read_double(vp);
		}

		void read(format<10, endian::native>, const void *vp) {
			read_extended(vp);
		}

		void read(format<12, endian::native>, const void *vp) {
			read_extended(vp);
		}

		void read(format<16, endian::native>, const void *vp) {
			read_extended(vp);
		}



		template<class T, typename = std::enable_if<std::is_floating_point<T>::value> >
		void write(T &x) const
		{ write(format<sizeof(x), endian::native>{}, &x); }

		void write(long double &x) const {
			if (native_extended) {
				write(format<sizeof(x), endian::native>{}, &x);
			} else {
				double tmp = x;
				write(tmp);
				x = tmp;
			}
		}

		template<size_t size, endian byte_order>
		void write(format<size, byte_order>, void *vp) const {

			uint8_t buffer[size];
			static_assert(byte_order != endian::native, "byte order");


			write(format<size, endian::native>{}, buffer);

			reverse_bytes<size>(buffer);
			std::memcpy(vp, buffer, size);
		}


		void write(format<4, endian::native>, void *vp) const {
			write_single(vp);
		}


		void write(format<8, endian::native>, void *vp) const {
			write_double(vp);
		}

		void write(format<10, endian::native>, void *vp) const {
			write_extended(vp);
		}

		void write(format<12, endian::native>, void *vp) const {
			write_extended(vp);
			std::memset((uint8_t *)vp + 10, 0, 12-10);
		}

		void write(format<16, endian::native>, void *vp) const {
			write_extended(vp);
			std::memset((uint8_t *)vp + 10, 0, 16-10);
		}


		explicit operator long double() const {
			if (native_extended) {
				long double tmp;
				write(tmp);
				return tmp;
			} else {
				double tmp;
				write(tmp);
				return tmp;
			}
		}
		explicit operator double() const {
			double tmp;
			write(tmp);
			return tmp;
		}
		explicit operator float() const {
			float tmp;
			write(tmp);
			return tmp;
		}


		template<class T, typename = std::enable_if<std::is_floating_point<T>::value> >
		explicit info(T x) { read(x); }
		info() = default;





	};


	template<endian byte_order>
	float read_single(format<4, byte_order>, const void *vp) {
		constexpr size_t size = 4;
		typedef float return_type;

		static_assert(sizeof(return_type) == size, "float size");

		typename std::aligned_storage<size, alignof(return_type)>::type buffer[1];

		std::memcpy(buffer, vp, size);
		reverse_bytes_if<size>(buffer, std::integral_constant<bool, byte_order != endian::native>{});
		return *(return_type *)buffer;
	}

	template<endian byte_order>
	double read_double(format<8, byte_order>, const void *vp) {
		constexpr size_t size = 8;
		typedef double return_type;

		static_assert(sizeof(return_type) == size, "double size");

		typename std::aligned_storage<size, alignof(return_type)>::type buffer[1];

		std::memcpy(buffer, vp, size);
		reverse_bytes_if<size>(buffer, std::integral_constant<bool, byte_order != endian::native>{});
		return *(return_type *)buffer;
	}


	template<endian byte_order>
	long double read_extended(format<8, byte_order> f, const void *vp) {
		return read_double(f, vp);
	}


	/*
	  sigh...
	  gcc ppc and gcc aarm64 claim long double is 16-bits but it's really just a double
	  with extra padding. (and ppc might actually be 2 doubles but it's weird.)
	  usable extendeds are only availble on x86, x64, and maybe Apple's PPC.
	 */
	template<size_t size, endian byte_order>
	long double read_extended(format<size, byte_order> f, const void *vp) {

		constexpr size_t ldsize = sizeof(long double);
		typedef long double return_type;

		static_assert(size == 10 || size == 12 || size == 16, "extended size");

		if (ldsize == 8) {
			// long double is really a double. manually down-grade it.
			info fpi;
			fpi.read(f, vp);

			return (return_type)fpi;
		} else if (!native_extended) {
			info fpi;
			fpi.read(format<10, byte_order>{}, vp);
			return (return_type)fpi;
		} else {

			constexpr size_t ssize = ldsize > size ? ldsize : size;

			typename std::aligned_storage<ssize, alignof(return_type)>::type buffer[1];

			std::memset(buffer, 0, ssize);
			std::memcpy(buffer, vp, size);
			reverse_bytes_if<size>(buffer, std::integral_constant<bool, byte_order != endian::native>{});
			return *(return_type *)buffer;
		}
	}


	template<endian byte_order>
	void write_single(float x, format<4, byte_order>, void *vp) {
		constexpr size_t size = 4;
		typedef float return_type;

		static_assert(sizeof(return_type) == size, "float size");

		typename std::aligned_storage<size, alignof(return_type)>::type buffer[1];

		std::memcpy(buffer, &x, size);
		reverse_bytes_if<size>(buffer, std::integral_constant<bool, byte_order != endian::native>{});
		std::memcpy(vp, buffer, size);
	}

	template<endian byte_order>
	void write_double(double x, format<8, byte_order>, void *vp) {
		constexpr size_t size = 8;
		typedef double return_type;

		static_assert(sizeof(return_type) == size, "double size");

		typename std::aligned_storage<size, alignof(return_type)>::type buffer[1];

		std::memcpy(buffer, &x, size);
		reverse_bytes_if<size>(buffer, std::integral_constant<bool, byte_order != endian::native>{});
		std::memcpy(vp, buffer, size);
	}


	template<size_t size, endian byte_order>
	void write_extended(long double x, format<size, byte_order> f, void *vp) {

		constexpr size_t ldsize = sizeof(long double);
		typedef long double return_type;

		static_assert(size == 10 || size == 12 || size == 16, "extended size");

		constexpr size_t ssize = ldsize > size ? ldsize : size;
		typename std::aligned_storage<ssize, alignof(return_type)>::type buffer[1];

		if (ldsize == 8 || !native_extended) {
			// do it manually.
			info fpi;
			fpi.read(x);
			fpi.write(f, vp);
		} else {

			std::memset(buffer, 0, ssize);
			std::memcpy(buffer, &x, ldsize);
			reverse_bytes_if<size>(buffer, std::integral_constant<bool, byte_order != endian::native>{});
			std::memcpy(vp, buffer, size);
		}
	}



} // floating point.


	inline int fpclassify(const floating_point::info &fpi) {
		if (fpi.nan) return FP_NAN;
		if (fpi.inf) return FP_INFINITE;
		if (fpi.sig == 0) return FP_ZERO;
		return fpi.sig >> 63 ? FP_NORMAL : FP_SUBNORMAL;
	}

	inline int signbit(const floating_point::info &fpi) {
		return fpi.sign;
	}

	inline int isnan(const floating_point::info &fpi) {
		return fpi.nan;
	}

	inline int isinf(const floating_point::info &fpi) {
		return fpi.inf;
	}

	inline int isfinite(const floating_point::info &fpi) {
		if (fpi.nan || fpi.inf) return false;
		return true;
	}

	inline int isnormal(const floating_point::info &fpi) {
		if (fpi.nan || fpi.inf) return false;
		return fpi.sig >> 63;
	}

	inline floating_point::info abs(const floating_point::info &fpi) {
		floating_point::info tmp(fpi);
		tmp.sign = 0;
		return tmp;
	}

	std::string to_string(const floating_point::info &fpi);

}

#endif
