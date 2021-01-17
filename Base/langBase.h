#define _IMGDISP_SOURCE_FILE_LANGBASE_H
#pragma once

#include <string>
#include <functional>
#include <stdarg.h>

#include "logger.h"


/************************
 Using
*************************/
using Logging::Logger;

using std::function;
using std::string;
using std::to_string;


/************************
 Define
*************************/
// Language
#ifndef DLLEXPORT
#ifdef DLL_IMPORT_MODE
#define DLLEXPORT			__declspec(dllimport)
#else
#define DLLEXPORT			__declspec(dllexport)
#endif
#endif

// Do Something
#define DO_SOMETHING(stts)		do{	\
	stts	\
	} while(0);

// Mem
#define SAFE_DELETE(P)		{if(nullptr != (P)){ delete (P); (P) = nullptr;}}
#define SAFE_DELETE_LIST(P)	{if(nullptr != (P)){ delete[] (P); (P) = nullptr;}}

// return
#define RETVAL_ON_FAILED(hr, val)		{ if(FAILED(hr)) { return (val); }}
#define RETVAL_ON_TRUE(b, val)			{ if((b)) { return (val); }}
#define RETVAL_ON_TRUE_AND(b, val, stts)	{ if((b)) { DO_SOMETHING(stts); return (val); }}
#define RET_ON_TRUE(b)					{ if(b) { return true; }}
#define RET_ON_TRUE_AND(b, stts)		{ if(b) { DO_SOMETHING(stts); return true; }}
#define RET_ON_FALSE(b)					{ if(!(b)) { return false; }}
#define RET_ON_FALSE_AND(b, stts)		{ if(!(b)) { DO_SOMETHING(stts); return false; }}
#define RET_ON_ZERO(v)					{ if(0 == (v)) { return (0); }}
#define RET_ON_ZERO_AND(v, stts)		{ if(0 == (v)) { DO_SOMETHING(stts); return (0); }}
#define RETVAL_ON_NULLPTR(p, val)		{ if(nullptr == (p)) { return (val); }}
#define RETVAL_ON_ZERO(v, val)			{ if(0 == (v)) { return (val); }}
#define RETVAL_ON_FALSE(b, val)			{ if(!(b)) { return (val); }}
#define RETVAL_ON_FALSE_AND(b, val, stts)	{ if(!(b)) { DO_SOMETHING(stts); return (val); }}
#define RET_ON_NEG(val)					{ if(decltype(val) $val$ = (val) < 0) { return $val$; }}
#define RET_ON_NEG_AND(val, stts)		{ if(decltype(val) $val$ = (val) < 0) { DO_SOMETHING(stts); return $val$; }}
#define RETVAL_ON_NEG(v, val)			{ if((v) < 0) { return (val); }}
#define RETVAL_ON_NEG_AND(v, val, stts)	{ if((v) < 0) { DO_SOMETHING(stts); return (val); }}
#define RET_ON_NP(val)					{ if(decltype(val) $val$ = (val) <= 0) { return $val$; }}
#define RETVAL_ON_POS(v, val)			{ if((v) > 0) { return (val); }}
#define RETVAL_ON_NP(v, val)			{ if((v) <= 0) { return (val); }}
#define RETVAL_ON_NP_AND(v, val, stts)	{ if((v) <= 0) { DO_SOMETHING(stts); return (val); }}
#define RET_ON_NP_AND(val, stts)		{ if(decltype(val) $val$ = (val) <= 0) { DO_SOMETHING(stts); return $val$; }}
#define RETVAL_ON_NULLPTR_AND(p, val, stts)	{ if(nullptr == (p)) { DO_SOMETHING(stts); return (val); }}
#define BREAK_ON_FALSE_AND(exp, stts)	{ if(!(exp)) { DO_SOMETHING(stts); break; }}
#define BREAK_ON_POS(val)				{ if((val) > 0) { break; }}
#define BREAK_ON_NEG_AND(val, stts)		{ if((val) < 0) { DO_SOMETHING(stts); break; }}
#define BREAK_ON_NULLPTR_AND(p, stts)	{ if(nullptr == (p)) { DO_SOMETHING(stts); break; }}
#define CONTINUE_ON_FALSE(b)			{ if(!(b)) { continue; }}
#define CONTINUE_ON_FALSE_AND(b, stts)	{ if(!(b)) { DO_SOMETHING(stts); continue; }}
#define CONTINUE_ON_NEG(val)			{ if((val) < 0) { continue; }}
#define CONTINUE_ON_NEG_AND(val, stts)	{ if((val) < 0) { DO_SOMETHING(stts); continue;}}
#define CONTINUE_ON_NP(val)				{ if((val) <= 0) { continue; }}
#define CONTINUE_ON_NP_AND(val, stts)	{ if((val) <= 0) { DO_SOMETHING(stts); continue; }}
#define CONTINUE_ON_NULLPTR(p)			{ if(nullptr == (p)) { DO_SOMETHING(stts); continue; }}
#define CONTINUE_IF(exp)				{ if(exp) { continue; }}
#define CONTINUE_IF_AND(exp, stts)		{ if(exp) { DO_SOMETHING(stts); continue; }}

// Spec
#define SIZET_MAX				(size_t)(-1)
#define MIN_OF_SIGNEDTYPE(T)	((T)((T)1 << (sizeof(T)*8 - 1)))
#define MIN_OF_UNSIGNEDTYPE(T)	(0)
#define MAX_OF_SIGNEDTYPE(T)	((T)((0xFFFFFFFFFFFFFFFFui64) >> (64 - sizeof(T)*8 + 1)))
#define MAX_OF_UNSIGNEDTYPE(T)	((T)(-1))

// Limit
#define OVERFLOW_MUL(a, b, x)		((x)/(a) < (b))
#define OVERFLOW_DIV(a, b, x)		((a)/(b) > (x))

// Class
#define OVERRIDE		override
#define PURE			= 0

//#define VIRTUALINTERNALLIFECYCLE
#ifndef VIRTUALINTERNALLIFECYCLE
#define LFCTYPE
#define LFCOVERRIDE
#else
#define LFCTYPE			virtual
#define LFCOVERRIDE		override
#endif



// Namespace bse
namespace bse
{

#pragma region Template

	//template<typename T>
	//bool IsSignedInt()
	//{
	//	return std::is_same<__int8, T>::value ||
	//			std::is_same<__int16, T>::value ||
	//			std::is_same<__int32, T>::value ||
	//			std::is_same<__int64, T>::value;
	//}
	//
	//template<bool... B>
	//struct LogicOr
	//{
	//	static const bool value = false;
	//};
	//template<bool BB, bool... B>
	//struct LogicOr<BB, B...>
	//{
	//	static const bool value = BB || LogicOr<B...>::value;
	//};
	//
	//template<typename T>
	//struct is_signedint
	//{
	//	static const bool value =
	//		LogicOr<
	//			std::is_same<__int8, T>::value,
	//			std::is_same<__int16, T>::value,
	//			std::is_same<__int32, T>::value,
	//			std::is_same<__int64, T>::value>::value;
	//};

	// Is signed int
	template<typename T>
	struct _Is_SignedInt : std::false_type {};
	template<>
	struct _Is_SignedInt<__int8> : std::true_type {};
	template<>
	struct _Is_SignedInt<__int16> : std::true_type {};
	template<>
	struct _Is_SignedInt<__int32> : std::true_type {};
	template<>
	struct _Is_SignedInt<__int64> : std::true_type {};


	template<typename T>
	struct is_signedint : _Is_SignedInt<typename std::remove_cv<T>::type> {};

	// Is unsigned int
	template<typename T>
	struct _Is_UnsignedInt : std::false_type {};
	template<>
	struct _Is_UnsignedInt<unsigned __int8> : std::true_type {};
	template<>
	struct _Is_UnsignedInt<unsigned __int16> : std::true_type {};
	template<>
	struct _Is_UnsignedInt<unsigned __int32> : std::true_type {};
	template<>
	struct _Is_UnsignedInt<unsigned __int64> : std::true_type {};
	template<typename T>
	struct is_unsignedint : _Is_UnsignedInt<typename std::remove_cv<T>::type> {};

	template<typename T, typename O = void>
	struct enable_if_signedint : std::enable_if<is_signedint<T>::value, O> {};

	template<typename T, typename O = void>
	struct enable_if_unsignedint : std::enable_if<is_unsignedint<T>::value, O> {};

	template<typename T, typename O = void>
	struct enable_if_int
		: std::enable_if<is_signedint<T>::value || is_unsignedint<T>::value, O> {};
	//: std::enable_if<std::numeric_limits<T>::is_interger, O>

	template<typename T, typename O = void>
	struct enable_if_int3264
		: std::enable_if<std::is_same<__int32, T>::value || std::is_same<__int64, T>::value, O>
	{
	};

	template<typename T, typename O = void>
	struct enable_if_pod8
		: std::enable_if<(sizeof(T) <= 8 && std::is_pod<T>::value), O>
	{
	};

#pragma endregion Template

#pragma region Limit
	/************************
	 Limit
	*************************/

#pragma region Limit-Add

#pragma endregion Limit-Add

#pragma region Limit-Mul

#pragma endregion Limit-Mul


	template<typename T>
	const typename
		enable_if_signedint<T, T>::type
		MaxValue()
	{
		//return std::numeric_limits<T>::min();
		return MAX_OF_SIGNEDTYPE(T);
	}
	template<typename T>
	const typename
		enable_if_unsignedint<T, T>::type
		MaxValue()
	{
		return MAX_OF_UNSIGNEDTYPE(T);
	}

	template<typename T>
	const typename
		enable_if_signedint<T, T>::type
		MinValue()
	{
		return MIN_OF_SIGNEDTYPE(T);
	}
	template<typename T>
	const typename
		enable_if_unsignedint<T, T>::type
		MinValue()
	{
		return MIN_OF_UNSIGNEDTYPE(T);
	}


	// mathematically add greater than
	template<typename T>
	static typename
		enable_if_signedint<T, bool>::type
		AddGT(T a, T b, T x)
	{
		const bool bAP = a >= 0;
		const bool bBP = b >= 0;
		if (bAP ^ bBP)
		{
			return a + b > x;
		}
		else
		{
			const bool bXP = x >= 0;
			if (bXP ^ bAP)
			{
				return bAP;
			}
			else
			{
				return (x - a < b);
			}
		}
	}
	// mathematically add greater than
	template<typename T>
	static typename
		enable_if_unsignedint<T, bool>::type
		AddGT(T a, T b, T x)
	{
		return (a > x || b > x - a);
	}
	// mathematically add greater than/equal
	template<typename T>
	static typename
		enable_if_signedint<T, bool>::type
		AddGE(T a, T b, T x)
	{
		const bool bAP = a >= 0;
		const bool bBP = b >= 0;
		if (bAP ^ bBP)
		{
			return a + b >= x;
		}
		else
		{
			const bool bXP = x >= 0;
			if (bXP ^ bAP)
			{
				return bAP;
			}
			else
			{
				return (x - a <= b);
			}
		}
	}
	// mathematically add greater than/equal
	template<typename T>
	static typename
		enable_if_unsignedint<T, bool>::type
		AddGE(T a, T b, T x)
	{
		return (a >= x || b >= x - a);
	}
	// mathematically add less than
	template<typename T>
	static typename
		enable_if_signedint<T, bool>::type
		AddLT(T a, T b, T x)
	{
		const bool bAP = a >= 0;
		const bool bBP = b >= 0;
		if (bAP ^ bBP)
		{
			return a + b < x;
		}
		else
		{
			const bool bXP = x >= 0;
			if (bXP ^ bAP)
			{
				return !bAP;
			}
			else
			{
				return (x - a > b);
			}
		}
	}
	// mathematically add less than
	template<typename T>
	static typename
		enable_if_unsignedint<T, bool>::type
		AddLT(T a, T b, T x)
	{
		return (a < x&& b < x - a);
	}
	// mathematically add less than/equal
	template<typename T>
	static typename
		enable_if_signedint<T, bool>::type
		AddLE(T a, T b, T x)
	{
		const bool bAP = a >= 0;
		const bool bBP = b >= 0;
		if (bAP ^ bBP)
		{
			return a + b <= x;
		}
		else
		{
			const bool bXP = x >= 0;
			if (bXP ^ bAP)
			{
				return false;
			}
			else
			{
				return (x - a >= b);
			}
		}
	}
	// mathematically add less than/equal
	template<typename T>
	static typename
		enable_if_unsignedint<T, bool>::type
		AddLE(T a, T b, T x)
	{
		return (a <= x && b <= x - a);
	}

	template<typename T, typename = typename enable_if_int<T>::type>
	class XOverflow
	{
	public:
		typedef T	Type;
	public:
		// overflow type B
		template<typename B>
		static bool
			Overflow(T a)
		{
			const B minValue = MinValue<B>();
			const B maxValue = MaxValue<B>();
			return a > maxValue || a < minValue;
		}
		// overflow type B upper
		template<typename B>
		static bool
			OverflowUpper(T a)
		{
			const B maxValue = MaxValue<B>();
			return a > maxValue;
		}
		// overflow type B lower
		template<typename B>
		static bool
			OverflowLower(T a)
		{
			const B minValue = MinValue<B>();
			return a < minValue;
		}

		// multiply overflow type T
		static bool
			MulOF(T a, T b)
		{
			if (a == 0 || b == 0)
			{
				return false;
			}
			const T limit = ((a > 0) ^ (b > 0)) ? MinValue<T>() : MaxValue<T>();
			return a > 0 ? (limit / b < a) : (limit / b > a);
		}
		// multiply overflow type B
		template<typename B>
		static bool
			MulOFB(T a, T b)
		{
			if (a == 0 || b == 0)
			{
				return false;
			}
			const B limit = ((a > 0) ^ (b > 0)) ? MinValue<B>() : MaxValue<B>();
			return a > 0 ? (limit / b < a) : (limit / b > a);
		}
		// mathematically multiply greater than
		static bool
			MulGT(T a, T b, T x)
		{
			if (a == 0 || b == 0)
			{
				return false;
			}
			return b > 0 ? (x / b < a) : (x / b > a);
		}
		// mathematically multiply greater than B
		template<typename B>
		static bool
			MulGTB(T a, T b, B x)
		{
			if (a == 0 || b == 0)
			{
				return false;
			}
			return b > 0 ? (x / b < a) : (x / b > a);
		}
		// mathematically multiply less than
		static bool
			MulLT(T a, T b, T x)
		{
			if (a == 0 || b == 0)
			{
				return false;
			}
			return b > 0 ? (x / b > a) : (x / b < a);
		}
		// mathematically multiply less than B
		template<typename B>
		static bool
			MulLTB(T a, T b, B x)
		{
			if (a == 0 || b == 0)
			{
				return false;
			}
			return b > 0 ? (x / b > a) : (x / b < a);
		}
		// TODO
		//MulGTBOF where a*b>x || a*b<minLimit

		// mathematically add greater than
		static bool
			AddGT(T a, T b, T x)
		{
			return bse::AddGT(a, b, x);
		}
		// mathematically add greater than
		static bool
			AddGE(T a, T b, T x)
		{
			return bse::AddGE(a, b, x);
		}
		// mathematically add less than
		static bool
			AddLT(T a, T b, T x)
		{
			return bse::AddLT(a, b, x);
		}
		// mathematically add less than/equal
		static bool
			AddLE(T a, T b, T x)
		{
			return bse::AddLE(a, b, x);
		}

		// add overflow type T
		static bool
			AddOF(T a, T b)
		{
			const T minValue = MinValue<T>();
			const T maxValue = MaxValue<T>();
			return AddGT(a, b, maxValue) || AddLT(a, b, minValue);
		}
	};


#pragma endregion Limit


#pragma region TypeOf
	/************************
	 Type
	*************************/
#ifdef CUSTOMIZED_TYPEOF

#define EXPAND(...) __VA_ARGS__

	// type_info& as [type]
	template <const std::type_info& type_id>
	struct TypeID {
	};
#define type_id(...) TypeID<typeid(__VA_ARGS__)>

	// Decode type
	template <typename T>struct Decode;
	template <typename T>struct Decode<T*> { typedef T type_t; };

#ifdef SEMIAUTO_TYPEOF

	// Extract type
	template<typename ID>
	struct Extract {};
#define type_extract(...) \
	Extract<type_id(__VA_ARGS__) >::type_t

	// Encode type
	template <typename T>
	struct Encode
	{
		typedef T* type_t;
	};

	// Register type
#define REGISTER_TYPE(type) \
	template<> \
	struct Extract<type_id(type*) > { typedef type* type_t; };

	REGISTER_TYPE(int)

#else

	struct empty_t {};

	// Extract type
	template<typename ID, typename T = empty_t>
	struct Extract;
	template<typename ID>
	struct Extract<ID, empty_t>
	{
		template <bool>
		struct id2type;
	};

	template<typename ID, typename T>
	struct Extract : Extract<ID, empty_t>
	{
		template <>
		struct id2type<true>
		{
			typedef T type_t;
		};
	};

#define type_extract(...) \
	Extract<type_id(__VA_ARGS__) >::id2type<true>::type_t

	// Register type
	template<typename T, typename ID>
	struct Register : Extract<ID, T>
	{
		typedef typename id2type<true>::type_t type_t;
	};

	// Encode type
	template <typename T>
	struct Encode
	{
		typedef T* enc_type_t;
		typedef Register<enc_type_t, type_id(enc_type_t)> reg_type;
		typedef typename reg_type::type_t	type_t;
	};

#endif

	template <typename T>
	typename Encode<T>::type_t encode(const T&);
	template <typename T>
	typename Encode<T>::type_t encode(T&);

#ifndef __GNUC__
	// type_of function
#define type_of(...) \
	Decode<type_extract(encode(__VA_ARGS__))>::type_t
#endif

//int j = 3;
//type_of(j) a = 4;
//Logger::Log0(to_string(a) + "," + string(typeid(a).name()));

#endif
#pragma endregion TypeOf


#pragma region ILifeCycle
/************************
 LifeCycle
*************************/
	class ILifeCycle
	{
	protected:
		LFCTYPE void	ClearPreAssign();
		LFCTYPE void	ClearMove();
		void			Release();
	};


	/************************
	 Follow LifeCycle
	*************************/
	template <class F>
	class IFollowMember
	{
	protected:
		F* _res;

		LFCTYPE void	ClearPreAssign()
		{
			SAFE_DELETE(_res);
		}
		LFCTYPE void	ClearMove()
		{
			_res = nullptr;
			_res = new F();
		}
		void			Release()
		{
			IFollowMember::ClearPreAssign();
		}
	private:
		// Disable copy-ctor, = operator
		IFollowMember(const IFollowMember&);
		IFollowMember& operator = (const IFollowMember&);
	public:
		IFollowMember() :
			_res(nullptr)
		{
			_res = new F();
		}
		IFollowMember(const F& f) :
			_res(nullptr)
		{
			_res = new F(f);
		}
		~IFollowMember()
		{
			Release();
		}

		IFollowMember(IFollowMember&& other) :
			_res(other._res)
		{
			other.IFollowMember::ClearMove();
		}
		int			AssignMove(IFollowMember& other)
		{
			// Clear before assignment
			IFollowMember::ClearPreAssign();

			_res = other._res;

			// Clear after move
			other.IFollowMember::ClearMove();

			return 1;
		}
		IFollowMember& operator = (IFollowMember&& other)
		{
			AssignMove(other);

			return *this;
		}
		F* operator -> ()
		{
			return _res;
		}
		const F* operator -> () const
		{
			return (const F*)_res;
		}
		F& operator*()
		{
			return *_res;
		}
		const F& operator*() const
		{
			return *_res;
		}
		inline bool		HasRes() const
		{
			return _res != nullptr;
		}

		inline F* GetRes()
		{
			return _res;
		}

		inline const F* GetResC() const
		{
			return _res;
		}
		inline void		Reset()
		{
			_res = nullptr;
			//try
			//{
			_res = new F();
			//}
			//catch(std::exception stde)
			//{
			//	// Alloc resource failed
			//}
		}
	};
#pragma endregion ILifeCycle


#pragma region Ticker
	/************************
	 Ticker
	*************************/
	class DLLEXPORT Ticker {
	public:
		typedef LONGLONG		tick_t;
		typedef LONGLONG		freq_t;
	protected:
		static bool		__bPrepared;
		static freq_t	f;// frequency

		tick_t			s;// starttime
		bool			bRun;

		double			c;// cycle
		double			ca;// accumulation of cycle
	public:
		static bool Prepare();

		static inline void		GetSystemTime(SYSTEMTIME& st)
		{
			FILETIME ft;
			GetSystemTimeAsFileTime(&ft);
			FileTimeToSystemTime(&ft, &st);
		}
		static inline string	GetSystemTime()
		{
			SYSTEMTIME st;
			Ticker::GetSystemTime(st);

			char buf[32] = { 0 };
			StringCchPrintfA(buf, 32, "%u-%02u-%02u %u:%02u:%02u.%03u",
				st.wYear, st.wMonth, st.wDay,
				st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

			return string(buf);
		}
		static inline tick_t	GetTick()
		{
			LARGE_INTEGER t = { 0 };
			QueryPerformanceCounter(&t);

			return t.QuadPart;
		}
		static inline double	GetPeriod(tick_t t)
		{
			LARGE_INTEGER e = { 0 };
			QueryPerformanceCounter(&e);
			return (double)(e.QuadPart - t) / f;
		}
		static freq_t GetFreq()
		{
			return f;
		}

		Ticker() :
			s(0),
			bRun(false),
			c(0.0),
			ca(0.0)
		{
			bRun = false;
		}
		~Ticker()
		{
		}

		inline void Start()
		{
			LARGE_INTEGER t = { 0 };
			QueryPerformanceCounter(&t);

			s = t.QuadPart;

			ca = c;

			bRun = true;
		}
		inline void ReStart()
		{
			Start();
		}
		inline double Tick() const
		{
			if (!bRun)
			{
				return -1;
			}

			LARGE_INTEGER t = { 0 };
			QueryPerformanceCounter(&t);

			double sec = (double)(t.QuadPart - s) / f;

			return sec;
		}
		inline double Cycle()
		{
			if (!bRun)
			{
				return -1;
			}

			LARGE_INTEGER t = { 0 };
			QueryPerformanceCounter(&t);

			double sec = (double)(t.QuadPart - s) / f;

			// restart
			s = t.QuadPart;

			return sec;
		}
		inline double Stop()
		{
			if (!bRun)
			{
				return -1;
			}

			bRun = false;

			LARGE_INTEGER t = { 0 };
			QueryPerformanceCounter(&t);

			double sec = (double)(t.QuadPart - s) / f;

			return sec;
		}

		inline void SetCycle(double cycle)
		{
			c = cycle;
		}
		inline bool IfCycleAndDoAutoAccum(double* pSec = NULL)
		{
			if (!bRun)
			{
				if (NULL != pSec)
				{
					*pSec = -1;
				}

				return false;
			}

			double sec = Tick();

			bool bCycle = sec >= ca;
			if (bCycle)
			{
				ca += c;
			}

			if (NULL != pSec)
			{
				*pSec = sec;
			}

			return bCycle;
		}
	};
#pragma endregion Ticker


#pragma region ScopeGuard
	/************************
	 ScopeGuard
	*************************/
	template <class F>
	class ScopeGuard;

	template <class F>
	struct ScopeGuardComp
	{
	public:
		typedef std::function<F>	func_t;

		func_t	_func;
		bool	_enabled;

	public:
		ScopeGuardComp() :_func(), _enabled(false) {}
		ScopeGuardComp(const ScopeGuardComp& scg) :_func(scg._func), _enabled(scg._enabled) {}
		ScopeGuardComp(ScopeGuard<F>&& scg) :_func(scg.GetFunc()), _enabled(scg.IsEnabled())
		{
			scg.Abandon();
		}
		ScopeGuardComp& operator= (ScopeGuard<F>&& scg)
		{
			_func = scg.GetFunc();
			_enabled = scg.IsEnabled();

			scg.Abandon();

			return *this;
		}

		void	Clear()
		{
			_enabled = false;
			_func = func_t();
		}
	};

	template class DLLEXPORT std::function<void()>;
	template struct DLLEXPORT ScopeGuardComp<void()>;
	typedef ScopeGuardComp<void()>	scope_guard_comp;

	template <class F>
	class ScopeGuard
	{
	public:
		typedef std::function<F>	func_t;

	protected:
		func_t	_func;
		bool	_enabled;
	protected:
		LFCTYPE void ClearPreAssign()
		{
			Release();
		}
		LFCTYPE void ClearMove()
		{
			_enabled = false;
			_func = func_t();
		}
	private:
		// Disable copy-ctor, = operator
		ScopeGuard(ScopeGuard& s);
		ScopeGuard& operator = (ScopeGuard& s);
	public:
		ScopeGuard() :_func(), _enabled(false) {}
		explicit ScopeGuard(const func_t& f) :_func(f), _enabled(true) {}
		explicit ScopeGuard(const func_t&& f) :_func(f), _enabled(true) {}
		ScopeGuard(ScopeGuardComp<F>&& scgc) :_func(scgc._func), _enabled(scgc._enabled)
		{
			scgc.Clear();
		}

		ScopeGuard(ScopeGuard&& s) :
			_func(std::forward<func_t>(s._func)),
			_enabled(s._enabled)
		{
			//Logger::g_logger.Log("ScopeGuard::Move ctor");
			s.ScopeGuard::ClearMove();
		}
		ScopeGuard& operator = (ScopeGuard&& s)
		{
			ScopeGuard::ClearPreAssign();

			_func = std::forward<func_t>(s._func);
			_enabled = s._enabled;

			s.ScopeGuard::ClearMove();

			return *this;
		}

		bool NotWork() const
		{
			return Empty() || !IsEnabled();
		}
		bool Empty() const
		{
			return (bool)_func;
		}
		bool SetFunc(const func_t& f)
		{
			// do not handle old guard

			_func = f;
			return true;
		}
		bool Disable()
		{
			_enabled = false;
			return true;
		}
		bool Enable()
		{
			_enabled = true;
			return true;
		}
		bool SetAndEnable(const func_t& f)
		{
			// do not handle old guard

			_func = f;
			_enabled = true;

			return true;
		}
		bool SetFrom(ScopeGuardComp<F>&& scgc)
		{
			_func = scgc._func;
			_enabled = scgc._enabled;

			scgc.Clear();

			return true;
		}
		void Release()
		{
			if (_enabled)
			{
				if (_func)
					_func();

				_enabled = false;
			}

			_func = func_t();
		}
		bool Abandon()
		{
			_enabled = false;
			_func = func_t();

			return true;
		}
		func_t	GetFunc() const
		{
			return _func;
		}
		func_t	ExtractFunc()
		{
			_enabled = false;
			return _func;
		}
		bool	IsEnabled() const
		{
			return _enabled;
		}
		~ScopeGuard()
		{
			//Logger::g_logger.Log("ScopeGuard::dtor");
			Release();
		}
	};

	template class DLLEXPORT std::function<void()>;
	template class DLLEXPORT ScopeGuard<void()>;
	typedef ScopeGuard<void()>	scope_guard;

	inline void funcScopeGuardNew(void* ptr)
	{
		SAFE_DELETE(ptr);
	}

	inline void funcScopeGuardNewList(void* ptr)
	{
		//Logger::Log0("funcScopeGuardNewList: release - " + to_string((_ULonglong)ptr));
		SAFE_DELETE_LIST(ptr);
	}

	inline void funcScopeGuardCTS(CRITICAL_SECTION* pCTS)
	{
		//Logger::Log0("funcScopeGuardCTS: leave critical section - " + to_string((_ULonglong)pCTS));
		if (nullptr != pCTS)
		{
			LeaveCriticalSection(pCTS);
		}
	}

#pragma endregion ScopeGuard

}

#undef _IMGDISP_SOURCE_FILE_LANGBASE_H