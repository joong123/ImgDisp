#define _IMGDISP_SOURCE_FILE_LOCK_H
#pragma once

#include <unordered_map>
#include <profileapi.h>
#include <synchapi.h>

#include "langBase.h"


/************************
 Using
*************************/
using std::unordered_map;

using bse::scope_guard;
using bse::Ticker;
using bse::enable_if_pod8;
using bse::funcScopeGuardCTS;


/************************
 Time
*************************/
typedef unsigned __int32		ms_t;
typedef unsigned __int32		s_t;
typedef __int64					ms_t1;
typedef double					sec_t;

#define MS_PER_SEC				1000
#define MS_TO_SEC(MS)			((sec_t)((MS)/(double)MS_PER_SEC))
#define SEC_TO_MS(SEC)			((ms_t)((SEC)*MS_PER_SEC))


/************************
 Thread ID
*************************/
#define INVALID_TID				(-1)
#define	GET_TID					(GetCurrentThreadId())


#pragma region ThreadSafe
/************************
 IThreadSafe
*************************/
#define	LOCKCONTROLED
#define	LOCKCONTROLED_READ
#define	LOCKCONTROLED_WRITE
#define	LOCKCONTROLED_BLOCK

#define LOCK_SLEEPCYCLE				(1)
#define LOCK_TIME_INFINITE			(-1)
#define LOCK_TIME_ISINFINITE(t)		(t < 0)
#define LOCK_TIMEOUT_MS				(SEC_TO_MS(60))
#define LOCK_TIMEOUT_SEC			(LOCK_TIMEOUT_MS/MS_PER_SEC)

typedef scope_guard					lock_guard;

#ifdef USE_ITSSINGLE
#define ITSSINGLE_SLEEPCYCLE				(1)
#define ITSSINGLE_INIT_INTERNAL(_lck)		InitializeCriticalSection(&_lck)
#define ITSSINGLE_TRYLOCK_INTERNAL(_lck)	TryEnterCriticalSection(&_lck)
#define ITSSINGLE_LOCK_INTERNAL(_lck)		EnterCriticalSection(&_lck)
#define ITSSINGLE_UNLOCK_INTERNAL(_lck)		LeaveCriticalSection(&_lck)
#define ITSSINGLE_DELETE_INTERNAL(_lck)		DeleteCriticalSection(&_lck)

class ITSSingle
{
private:
	CRITICAL_SECTION	_section0;// static LifeCycle member(unmove, destroy in dtor)
	ms_t				_msCycle;

private:
	ITSSingle(ITSSingle&);
	ITSSingle& operator = (ITSSingle&);
	ITSSingle& operator = (ITSSingle&&);
public:
	ITSSingle() : _msCycle(ITSSINGLE_SLEEPCYCLE)
	{
		ITSSINGLE_INIT_INTERNAL(_section0);
	}
	ITSSingle(ITSSingle&& its) noexcept : _msCycle(ITSSINGLE_SLEEPCYCLE)
	{
		ITSSINGLE_INIT_INTERNAL(_section0);
	}
	virtual			~ITSSingle()
	{
		// Lock
		ITSSINGLE_LOCK_INTERNAL(_section0);
		ITSSINGLE_DELETE_INTERNAL(_section0);
		memset(&_section0, 0, sizeof(CRITICAL_SECTION));
	}

	inline bool		TryLock()
	{
		return ITSSINGLE_TRYLOCK_INTERNAL(_section0) != 0;
	}
	inline void		Lock()
	{
		// Lock
		ITSSINGLE_LOCK_INTERNAL(_section0);
	}
	inline bool		Lock(ms_t msTimeout)
	{
		const sec_t sTimeout = MS_TO_SEC(msTimeout);

		Ticker::freq_t f = Ticker::GetFreq();
		LARGE_INTEGER s, e;
		QueryPerformanceCounter(&s);
		while (ITSSINGLE_TRYLOCK_INTERNAL(_section0) != 0)
		{
			QueryPerformanceCounter(&e);
			RETVAL_ON_TRUE(
				(sec_t)(e.QuadPart - s.QuadPart) / f > sTimeout,
				false);

			Sleep(_msCycle);
		}
	}
	inline bool		TryLockGuard(lock_guard& scg)
	{
		RETVAL_ON_ZERO(
			ITSSINGLE_TRYLOCK_INTERNAL(_section0),
			false);
		scg.SetAndEnable(std::bind(funcScopeGuardCTS, &_section0));
		return true;
	}
	inline bool		LockGuard(lock_guard& scg)
	{
		ITSSINGLE_LOCK_INTERNAL(_section0);
		scg.SetAndEnable(std::bind(funcScopeGuardCTS, &_section0));
		return true;
	}
	inline void		Unlock()
	{
		ITSSINGLE_UNLOCK_INTERNAL(_section0);
	}
};

#undef ITSSINGLE_INIT_INTERNAL
#undef ITSSINGLE_TRYLOCK_INTERNAL
#undef ITSSINGLE_LOCK_INTERNAL
#undef ITSSINGLE_UNLOCK_INTERNAL
#undef ITSSINGLE_DELETE_INTERNAL

//#define ITSSINGLE_LOCK			ITSSingle::Lock();
//#define ITSSINGLE_TRYLOCK		ITSSingle::TryLock();
//#define ITSSINGLE_UNLOCK		ITSSingle::Unlock();

#endif// ITSSINGLE


template<class T>
class IRefCntObj
{
public:
	typedef unsigned __int64	nref_t;

private:
	nref_t						_nRef;
	T							_obj;

public:
	IRefCntObj();
	IRefCntObj(IRefCntObj&&);
	~IRefCntObj();

	void	AddRef();
	nref_t	GetRef();
};


#define CAS(v, e, u)							_InterlockedCompareExchange(v, u, e)
#define CASSUCCEED(v, e, u)						(CAS(v, e, u) == e)
#define CASFAIL(v, e, u)						(CAS(v, e, u) != e)
#define EQU(v, e)								(CAS(v, e, e) == e)
#define UEQU(v, e)								(CAS(v, e, e) != e)
#define EXC(v, u)								_InterlockedExchange(v, u)

#define ITSCAS_SLEEPCYCLE						(1)
#define ITSCAS_INIT_INTERNAL(v)					DO_SOMETHING(\
													(v) = new cas_t();\
													*(v) = ITSCAS::ITSCAS_FREE;\
													)

#define ITSCAS_TRYLOCK_INTERNAL(v)				CAS(v, ITSCAS::ITSCAS_FREE, ITSCAS::ITSCAS_LOCKED)
#define ITSCAS_TRYLOCKED_INTERNAL(v)			CASSUCCEED(v, ITSCAS::ITSCAS_FREE, ITSCAS::ITSCAS_LOCKED)
#define ITSCAS_TRYLOCKEDNOT_INTERNAL(v)			CASFAIL(v, ITSCAS::ITSCAS_FREE, ITSCAS::ITSCAS_LOCKED)
#define ITSCAS_UNLOCK_INTERNAL(v)				EXC(v, ITSCAS::ITSCAS_FREE)
#define ITSCAS_TRYLOCKDYING_INTERNAL(v)			CAS(v, ITSCAS::ITSCAS_FREE, ITSCAS::ITSCAS_DYING)
#define ITSCAS_TRYLOCKEDNOTDYING_INTERNAL(v)	CASFAIL(v, ITSCAS::ITSCAS_FREE, ITSCAS::ITSCAS_DYING)

#pragma intrinsic(_InterlockedCompareExchange, _InterlockedExchange)

class ITSCAS
{
public:
	typedef unsigned __int64	cas_t;

	enum { ITSCAS_FREE = 0, ITSCAS_LOCKED = 1, ITSCAS_DYING = -1 };
private:
	volatile cas_t* _v = nullptr;
	ms_t			_msCycle = ITSCAS_SLEEPCYCLE;

private:
	ITSCAS(ITSCAS&);
	ITSCAS& operator = (ITSCAS&);
	ITSCAS& operator = (ITSCAS&&) noexcept;
	ITSCAS(ITSCAS&& its) noexcept;
public:
	ITSCAS()
	{
		ITSCAS_INIT_INTERNAL(_v);
	}
	explicit ITSCAS(cas_t* v)
	{
		if (nullptr == _v)
		{
			ITSCAS_INIT_INTERNAL(_v);
		}
	}
	virtual			~ITSCAS()
	{
		// Lock
		while (true)
		{
			cas_t s = ITSCAS_TRYLOCKDYING_INTERNAL(_v);
			if (s == ITSCAS::ITSCAS_FREE || s == ITSCAS::ITSCAS_DYING)
				break;
			Sleep(_msCycle);
		}
		SAFE_DELETE(_v);
	}

	inline void		SetMSCycle(ms_t ms)
	{
		_msCycle = ms;
	}
	inline bool		IsLocked()
	{
		return ITSCAS::ITSCAS_FREE != *_v;
	}
	inline bool		TryLock()
	{
		return ITSCAS_TRYLOCKED_INTERNAL(_v);
	}
	inline void		Lock()
	{
		// Lock
		while (ITSCAS_TRYLOCKEDNOT_INTERNAL(_v))
		{
			Sleep(_msCycle);
		}
	}
	inline bool		Lock(ms_t msTimeout)
	{
		const sec_t sTimeout = MS_TO_SEC(msTimeout);

		Ticker::freq_t f = Ticker::GetFreq();
		LARGE_INTEGER s, e;
		QueryPerformanceCounter(&s);
		while (ITSCAS_TRYLOCKEDNOT_INTERNAL(_v))
		{
			QueryPerformanceCounter(&e);
			RETVAL_ON_TRUE(
				(sec_t)(e.QuadPart - s.QuadPart) / f > sTimeout,
				false);

			Sleep(_msCycle);
		}

		return true;
	}
	inline bool		TryLockGuard(lock_guard& scg)
	{
		RETVAL_ON_TRUE(
			ITSCAS_TRYLOCKEDNOT_INTERNAL(_v),
			false);

		scg.SetAndEnable(std::bind(&ITSCAS::Unlock, this));
		return true;
	}
	inline bool		LockGuard(lock_guard& scg)
	{
		while (ITSCAS_TRYLOCKEDNOT_INTERNAL(_v))
		{
			Sleep(_msCycle);
		}

		scg.SetAndEnable(std::bind(&ITSCAS::Unlock, this));
		return true;
	}
	inline bool		LockGuard(lock_guard& scg, ms_t msTimeout)
	{
		const sec_t sTimeout = MS_TO_SEC(msTimeout);

		Ticker::freq_t f = Ticker::GetFreq();
		LARGE_INTEGER s, e;
		QueryPerformanceCounter(&s);
		while (ITSCAS_TRYLOCKEDNOT_INTERNAL(_v))
		{
			QueryPerformanceCounter(&e);
			RETVAL_ON_TRUE(
				(sec_t)(e.QuadPart - s.QuadPart) / f > sTimeout,
				false);

			Sleep(_msCycle);
		}

		scg.SetAndEnable(std::bind(&ITSCAS::Unlock, this));
		return true;
	}
	inline void		Unlock()
	{
		ITSCAS_UNLOCK_INTERNAL(_v);
	}
	inline void		LockDying()
	{
		// Lock
		while (ITSCAS_TRYLOCKEDNOTDYING_INTERNAL(_v))
		{
			Sleep(_msCycle);
		}
	}
};


#undef ITSCAS_SLEEPCYCLE
#undef ITSCAS_INIT_INTERNAL

#undef ITSCAS_TRYLOCK_INTERNAL
#undef ITSCAS_TRYLOCKED_INTERNAL
#undef ITSCAS_TRYLOCKEDNOT_INTERNAL
#undef ITSCAS_UNLOCK_INTERNAL
#undef ITSCAS_TRYLOCKDYING_INTERNAL
#undef ITSCAS_TRYLOCKEDNOTDYING_INTERNAL

#pragma endregion ThreadSafe


#pragma region Lock
/************************
 Lock
*************************/
// cas_lock
// base_lock
// its_lock
// naive_lock
// count_lock
// single_lock
// icrwb_lock

enum LOCK_RET
{
	LOCK_ABORT = -5,
	LOCK_MISMATCH = -4,
	LOCK_INVSTATUS = -3,// Not locked
	LOCK_TIMEOUT = -2,
	LOCK_INTERNALERR = -1,
	LOCK_OCCUPIED = 0,
	LOCK_SUCCEED = 1
};


#define LOCK_INFINITE_RES			(-1)
#define LOCK_INFINITE_RES_ACTUAL	(INT_MAX)

#define LOCK_TRYLOCK_INTERNAL			its_t::TryLock()
#define LOCK_UNLOCK_INTERNAL			its_t::Unlock();

#define LOCK_LOCKSCG_INTERNAL			scope_guard scgLock;\
										its_t::LockGuard(scgLock);
#define LOCK_LOCKTIMESCG_INTERNAL(TO, RET)	scope_guard scgLock;\
											bool bInfiniteTimeout = LOCK_TIME_ISINFINITE(TO);\
											RET = (bInfiniteTimeout ? its_t::LockGuard(scgLock) : its_t::LockGuard(scgLock, TO));
#define LOCK_LOCKOTHERSCG_INTERNAL(O)	scope_guard scgLockOther;\
										(O).its_t::LockGuard(scgLockOther);
#define LOCK_UNLOCKSCG_INTERNAL			scgLock.Release();
#define LOCK_UNLOCKOTHERSCG_INTERNAL	scgLockOther.Release();

#define LOCK_LOCKDYING_INTERNAL			its_t::LockDying();

#define LOCK_CHECK_TRYLOCK_INTERNAL		RETVAL_ON_FALSE(\
											its_t::TryLock(),\
											LOCK_RET::LOCK_INTERNALERR);
#define LOCK_CHECK_LOCKED(S)		RETVAL_ON_TRUE_AND(\
										(S),\
										LOCK_RET::LOCK_INVSTATUS,\
										LOCK_UNLOCK_INTERNAL);
#define LOCK_CHECK_LOCKEDSCG(S)		RETVAL_ON_TRUE(\
										(S),\
										LOCK_RET::LOCK_INVSTATUS);
#define LOCK_CHECK_FREE(S)			RETVAL_ON_TRUE_AND(\
										(S),\
										LOCK_RET::LOCK_OCCUPIED,\
										LOCK_UNLOCK_INTERNAL);
#define LOCK_CHECK_MISMATCH(S)		RETVAL_ON_TRUE_AND(\
										(S),\
										LOCK_RET::LOCK_MISMATCH,\
										LOCK_UNLOCK_INTERNAL);
#define LOCK_CHECK_MISMATCHSCG(S)	RETVAL_ON_TRUE(\
										(S),\
										LOCK_RET::LOCK_MISMATCH);
#define LOCK_CHECK_TIMEOUTSCG(T, L)	RETVAL_ON_TRUE(\
										(T) >= (L),\
										LOCK_RET::LOCK_TIMEOUT);

typedef ITSCAS		lock_its_t;// ITSSingle, ITSCAS


/*
 Cas lock
*/

typedef ITSCAS	cas_lock;

//#define CASLOCK_TRYLOCK_INTERNAL		ITSCAS::TryLock()
//#define CASLOCK_LOCK_INTERNAL			ITSCAS::Lock();
//#define CASLOCK_UNLOCK_INTERNAL			ITSCAS::Unlock();
//#define CASLOCK_CLEAR_INTERNAL			CASLOCK_UNLOCK_INTERNAL
//#define CASLOCK_ASSIGN_INTERNAL(lck)	DO_SOMETHING();
//
//class cas_lock : public ITSCAS
//{
//protected:
//protected:
//	LFCTYPE void	ClearPreAssign()
//	{
//	}
//	LFCTYPE void	ClearMove()
//	{
//		CASLOCK_CLEAR_INTERNAL;
//	}
//	void			Release()
//	{
//	}
//private:
//	cas_lock(cas_lock &);
//	cas_lock&	operator = (cas_lock &);
//public:
//	cas_lock():
//		ITSCAS		()
//	{
//	}
//	cas_lock(cas_lock &&lock):
//		ITSCAS		()
//	{
//	}
//	virtual			~cas_lock() OVERRIDE
//	{
//	}
//
//	//int				AssignMove(its_lock &);
//	cas_lock&		operator = (cas_lock &&lock)
//	{
//		return *this;
//	}
//
//#pragma region Interface
//
//	virtual inline bool	IsLocked()
//	{
//		return ITSCAS::IsLocked();
//	}
//	virtual inline bool	TryLock()
//	{
//		//Ticker t;
//		//t.Start();
//		bool rs = CASLOCK_TRYLOCK_INTERNAL;
//		
//		//double tc = t.Tick();
//		//Logger::Log0("CAS Trylock ms: " + to_string(tc*1000));
//		return rs;
//	}
//	int	TryLockGuard(scope_guard &scg)
//	{
//		Ticker ticker;
//		ticker.Start();
//		bool rs = CASLOCK_TRYLOCK_INTERNAL;
//		double tc = ticker.Tick();
//		RET_ON_FALSE_AND(
//			rs,
//			Logger::Log0("Trylock CAS failed, ms: " + to_string(tc*1000)););
//		
//		Logger::Log0("Trylock CAS ms: " + to_string(tc*1000));
//
//		scg.SetAndEnable(
//			[this](){
//				UnLock();
//			});
//
//		return rs;
//	}
//	int	LockGuard(scope_guard &scg)
//	{
//		int ret = Lock();
//		RET_ON_NP_AND(
//			ret,
//			Logger::Log0("Lock failed CAS, ret:" + to_string(ret)););
//	
//		//Logger::Log0("Lock CAS");
//
//		scg.SetAndEnable(
//			[this]()
//			{
//				UnLock();
//			});
//
//		return ret;
//	}
//	virtual int	LockCycleGuard(
//		scope_guard &scg, ms_t1 msTimeout,
//		const function<bool()> &funcAbort = function<bool()>())
//	{
//		const sec_t sTimeout = MS_TO_SEC(msTimeout);
//		bool bInfiniteTimeout = LOCK_TIME_ISINFINITE(msTimeout);
//		Ticker ticker;
//		ticker.Start();
//
//		int ret = LOCK_RET::LOCK_INTERNALERR;
//		while(true)
//		{
//			int ret = TryLock();
//
//			// Check lock succeed
//			BREAK_ON_POS(
//				ret);
//
//			// Check internal lock failure
//			RET_ON_NP(
//				ret);
//
//			// Check abort
//			if(!funcAbort._Empty())
//			{
//				RETVAL_ON_TRUE(
//					funcAbort(),
//					LOCK_RET::LOCK_ABORT);
//			}
//			
//			// Timeout
//			if(!bInfiniteTimeout)
//			{
//				LOCK_CHECK_TIMEOUTSCG(ticker.Tick(), (double)msTimeout/1000);
//			}
//
//			Sleep(LOCK_SLEEPCYCLE);
//		}
//
//		scg.SetAndEnable(
//			[this]()
//			{
//				UnLock();
//			});
//
//		return ret;
//	}
//	virtual inline int	Lock()
//	{
//		CASLOCK_LOCK_INTERNAL;
//		return LOCK_RET::LOCK_SUCCEED;
//	}
//	virtual inline int	TryUnLock()
//	{
//		CASLOCK_UNLOCK_INTERNAL;
//		return LOCK_RET::LOCK_SUCCEED;
//	}
//	virtual inline int	UnLock()
//	{
//		CASLOCK_UNLOCK_INTERNAL;
//		return LOCK_RET::LOCK_SUCCEED;
//	}
//
//#pragma endregion Interface
//};
//
//#undef CASLOCK_TRYLOCK_INTERNAL
//#undef CASLOCK_LOCK_INTERNAL
//#undef CASLOCK_UNLOCK_INTERNAL
//#undef CASLOCK_CLEAR_INTERNAL
//#undef CASLOCK_ASSIGN_INTERNAL


enum LOCK_MODE
{
	LKM_LOCK = 1,
	LKM_BLOCK = 2,
	LKM_WRITE = 3,
	LKM_READ = 4,
	LKM_IREAD = 5
};

/*
 Base lock
*/
class base_lock : public lock_its_t
{
protected:
	typedef lock_its_t	its_t;
public:
	typedef __int64		tid_t;
protected:
	LFCTYPE void	ClearPreAssign() {}
	LFCTYPE void	ClearMove() {}
	void			Release() {}
private:
	base_lock(base_lock&);
	base_lock& operator = (base_lock&);
public:
	base_lock() :
		its_t() {}
	base_lock(base_lock&& lock) noexcept :
		its_t() {}
	virtual			~base_lock() {}

	int				AssignMove(base_lock&);
	base_lock& operator = (base_lock&& lock) noexcept;

#pragma region Interface

	virtual inline bool	IsLocked() PURE;

	virtual inline int	TryLock() PURE;
	virtual inline int	Lock(ms_t1 msTimeout = LOCK_TIME_INFINITE) PURE;
	virtual inline int	TryUnLock() PURE;
	virtual inline int	UnLock() PURE;
	virtual int	LockGuard(scope_guard& scg, ms_t1 msTimeout = LOCK_TIME_INFINITE);
	int	TryLockGuard(scope_guard& scg)
	{
		//Ticker ticker;
		//ticker.Start();
		int ret = TryLock();
		//double tc = ticker.Tick();
		RET_ON_NP_AND(
			ret,
			Logger::Log0("Trylock Base failed, ret:" + to_string(ret)););
		//Logger::Log0("Trylock Base ms: " + to_string(tc*1000));

		scg.SetAndEnable(
			[this]() {
				UnLock();
			});

		return LOCK_RET::LOCK_SUCCEED;
	}

	virtual int	LockCycleGuard(
		scope_guard& scg, ms_t1 msTimeout,
		const function<bool()>& funcAbort = function<bool()>())
	{
		const sec_t sTimeout = MS_TO_SEC(msTimeout);
		bool bInfiniteTimeout = LOCK_TIME_ISINFINITE(msTimeout);

		Ticker ticker;
		ticker.Start();

		int ret = LOCK_RET::LOCK_INTERNALERR;
		while (true)
		{
			int ret = TryLock();

			// Check internal lock failure
			RET_ON_NP(
				ret);

			// Check lock succeed
			BREAK_ON_POS(
				ret);

			// Check abort
			if (funcAbort)
			{
				RETVAL_ON_TRUE(
					funcAbort(),
					LOCK_RET::LOCK_ABORT);
			}

			// Timeout
			if (!bInfiniteTimeout)
			{
				LOCK_CHECK_TIMEOUTSCG(ticker.Tick(), sTimeout);
			}

			Sleep(LOCK_SLEEPCYCLE);
		}
		double tc = ticker.Tick();
		Logger::Log0("Lock Base ms: " + to_string(tc * 1000));

		scg.SetAndEnable(
			[this]()
			{
				UnLock();
			});

		return LOCK_RET::LOCK_SUCCEED;
	}

#pragma endregion Interface
};

/*
 ITS lock
*/

//#define ITS_LOCK
#ifdef ITS_LOCK

#define ITSLOCK_TRYLOCK_INTERNAL		its_t::TryLock()
#define ITSLOCK_LOCK_INTERNAL(TO, RET)	bool bInfiniteTimeout = LOCK_TIME_ISINFINITE(TO);\
										RET = (bInfiniteTimeout ? its_t::Lock() : its_t::Lock(TO));
#define ITSLOCK_UNLOCK_INTERNAL			its_t::Unlock();
#define ITSLOCK_CLEAR_INTERNAL			LOCK_UNLOCK_INTERNAL
#define ITSLOCK_ASSIGN_INTERNAL(lck)	DO_SOMETHING(\
											_tid		= lck._tid;\
											_bLocked	= lck._bLocked;\
										);
class its_lock : public base_lock
{
protected:
protected:
	LFCTYPE void	ClearPreAssign()
	{
	}
	LFCTYPE void	ClearMove()
	{
		ITSLOCK_CLEAR_INTERNAL;
	}
	void			Release();
private:
	its_lock(its_lock&);
	its_lock& operator = (its_lock&);
public:
	its_lock() :
		base_lock()
	{
	}
	its_lock(its_lock&& lock) :
		base_lock()
	{
	}
	virtual			~its_lock() OVERRIDE
	{
		LOCK_LOCKDYING_INTERNAL;
	}

	//int				AssignMove(its_lock &);
	its_lock& operator = (its_lock&& lock)
	{
		return *this;
	}

#pragma region Interface

	virtual inline bool	IsLocked() OVERRIDE
	{
		return true;
	}
	virtual inline int	TryLock() OVERRIDE
	{
		//Ticker t;
		//t.Start();
		int ret = ITSLOCK_TRYLOCK_INTERNAL ? LOCK_RET::LOCK_SUCCEED : LOCK_RET::LOCK_OCCUPIED;

		//double tc = t.Tick();
		//Logger::Log0("ITS Trylock ms: " + to_string(tc*1000));
		return ret;
	}
	int	TryLockGuard(scope_guard& scg)
	{
		Ticker ticker;
		ticker.Start();
		int ret = ITSLOCK_TRYLOCK_INTERNAL ? LOCK_RET::LOCK_SUCCEED : LOCK_RET::LOCK_OCCUPIED;
		double tc = ticker.Tick();
		RET_ON_NP_AND(
			ret,
			Logger::Log0("Trylock failed, ret:" + to_string(ret) + ", ms: " + to_string(tc * 1000)););

		//Logger::Log0("Trylock ITS ms: " + to_string(tc*1000));

		scg.SetAndEnable(
			[this]() {
				UnLock();
			});

		return ret;
	}
	virtual inline int	Lock(ms_t1 msTimeout = LOCK_TIME_INFINITE) OVERRIDE
	{
		bool ret = false;
		ITSLOCK_LOCK_INTERNAL(msTimeout, ret);
		RETVAL_ON_FALSE(
			ret,
			LOCK_RET::LOCK_TIMEOUT);

		return LOCK_RET::LOCK_SUCCEED;
	}
	virtual inline int	TryUnLock() OVERRIDE
	{
		ITSLOCK_UNLOCK_INTERNAL;
		return LOCK_RET::LOCK_SUCCEED;
	}
	virtual inline int	UnLock() OVERRIDE
	{
		ITSLOCK_UNLOCK_INTERNAL;
		return LOCK_RET::LOCK_SUCCEED;
	}

#pragma endregion Interface
};

#undef ITSLOCK_TRYLOCK_INTERNAL
#undef ITSLOCK_LOCK_INTERNAL
#undef ITSLOCK_UNLOCK_INTERNAL
#undef ITSLOCK_CLEAR_INTERNAL
#undef ITSLOCK_ASSIGN_INTERNAL

#endif

/*
 Naive lock
*/
#define NAIVELOCK_LOCK_INTERNAL		DO_SOMETHING(\
										_tid = (tid_t)GET_TID;\
										_bLocked = true;\
									);
#define NAIVELOCK_UNLOCK_INTERNAL	DO_SOMETHING(\
										_bLocked = false;\
										_tid = INVALID_TID;\
									);
#define NAIVELOCK_CLEAR_INTERNAL	NAIVELOCK_UNLOCK_INTERNAL
#define NAIVELOCK_ASSIGN_INTERNAL(lck)	DO_SOMETHING(\
										_tid		= lck._tid;\
										_bLocked	= lck._bLocked;\
									);
class naive_lock : public base_lock
{
protected:
	volatile tid_t		_tid;
	volatile bool		_bLocked;

protected:
	LFCTYPE void	ClearPreAssign()
	{
	}
	LFCTYPE void	ClearMove()
	{
		NAIVELOCK_CLEAR_INTERNAL;
	}
	void			Release()
	{
	}
private:
	naive_lock(naive_lock&);
	naive_lock& operator = (naive_lock&);
public:
	naive_lock();
	naive_lock(naive_lock&& lock) noexcept;
	virtual			~naive_lock() OVERRIDE;

	int				AssignMove(naive_lock&) noexcept;
	naive_lock& operator = (naive_lock&& lock) noexcept;

#pragma region Interface

	virtual inline bool	IsLocked() OVERRIDE
	{
		return _bLocked;
	}
	virtual inline int	TryLock() OVERRIDE
	{
		// Try lock internal
		LOCK_CHECK_TRYLOCK_INTERNAL;

		// If already locked
		LOCK_CHECK_FREE(_bLocked);

		// Lock
		NAIVELOCK_LOCK_INTERNAL;

		// Unlock internal
		LOCK_UNLOCK_INTERNAL;

		return LOCK_RET::LOCK_SUCCEED;
	}
	virtual inline int	Lock(ms_t1 msTimeout = LOCK_TIME_INFINITE) OVERRIDE;
	virtual inline int	TryUnLock() OVERRIDE
	{
		// Try lock internal
		LOCK_CHECK_TRYLOCK_INTERNAL;

		// If not locked
		LOCK_CHECK_LOCKED(!_bLocked);

		// Check thread ID
		LOCK_CHECK_MISMATCH((tid_t)GET_TID != _tid);

		// Unlock
		NAIVELOCK_UNLOCK_INTERNAL;

		// Unlock internal
		LOCK_UNLOCK_INTERNAL;

		return LOCK_RET::LOCK_SUCCEED;
	}
	virtual inline int	UnLock() OVERRIDE;

#pragma endregion Interface
};

/*
 Count lock
*/
#define COUNTLOCK_DEFAULT_RES			(10)

#define COUNTLOCK_FULL_INTERNAL			(_nLock >= _nRes)
#define COUNTLOCK_AVAILABLE_INTERNAL(tid)	(_nLock < _nRes &&\
											((_nLock > 0 && tid == _tid) ||\
											(_nLock == 0)))
#define COUNTLOCK_EMPTY_INTERNAL		(_nLock <= 0)
#define COUNTLOCK_LOCK_INTERNAL(tid)	DO_SOMETHING(\
											if(_nLock == 0){\
												_tid = tid;\
											}\
											++_nLock;)
#define COUNTLOCK_UNLOCK_INTERNAL		DO_SOMETHING(\
											--_nLock;\
											if(_nLock == 0){\
												_tid = INVALID_TID;\
											})
#define COUNTLOCK_CLEAR_INTERNAL		DO_SOMETHING(\
											_nLock = 0;\
											_tid = INVALID_TID;)
#define COUNTLOCK_ASSIGN_INTERNAL(lck)	DO_SOMETHING(\
											_nRes		= lck._nRes;\
											_tid		= lck._tid;\
											_nLock		= lck._nLock;)
class count_lock : public base_lock
{
protected:
	int					_nRes;

	volatile tid_t		_tid;
	volatile int		_nLock;

protected:
	LFCTYPE void	ClearPreAssign()
	{
	}
	LFCTYPE void	ClearMove()
	{
		COUNTLOCK_CLEAR_INTERNAL;
	}
	void			Release()
	{
	}
private:
	count_lock(count_lock&);
	count_lock& operator = (count_lock&);
public:
	count_lock();
	count_lock(count_lock&& lock) noexcept;
	virtual			~count_lock() OVERRIDE;

	int				AssignMove(count_lock&) noexcept;
	count_lock& operator = (count_lock&& lock) noexcept;

#pragma region Interface

	virtual inline bool	IsLocked() OVERRIDE
	{
		return _nLock > 0;
	}
	inline bool		SetResource(int nRes)
	{
		if (LOCK_INFINITE_RES == nRes)
		{
			_nRes = LOCK_INFINITE_RES_ACTUAL;
			return true;
		}
		else if (nRes < 0)
		{
			return false;
		}

		_nRes = nRes;

		return true;
	}
	virtual inline int	TryLock() OVERRIDE
	{
		// Try lock internal
		LOCK_CHECK_TRYLOCK_INTERNAL;

		// If full locked
		LOCK_CHECK_FREE(COUNTLOCK_FULL_INTERNAL);

		// Check thread ID
		tid_t tid = (tid_t)GET_TID;
		if (_nLock > 0)
		{
			// Check thread ID
			LOCK_CHECK_MISMATCH(tid != _tid);
		}

		// Lock
		COUNTLOCK_LOCK_INTERNAL(tid);

		// Unlock internal
		LOCK_UNLOCK_INTERNAL;

		return LOCK_RET::LOCK_SUCCEED;
	}
	virtual inline int	Lock(ms_t1 msTimeout = LOCK_TIME_INFINITE) OVERRIDE;
	virtual inline int	TryUnLock() OVERRIDE
	{
		// Try lock internal
		LOCK_CHECK_TRYLOCK_INTERNAL;

		// If not locked
		LOCK_CHECK_LOCKED(COUNTLOCK_EMPTY_INTERNAL);

		// Check thread ID
		LOCK_CHECK_MISMATCH((tid_t)GET_TID != _tid);

		// Unlock
		COUNTLOCK_UNLOCK_INTERNAL;

		// Unlock internal
		LOCK_UNLOCK_INTERNAL;

		return LOCK_RET::LOCK_SUCCEED;
	}
	virtual inline int	UnLock() OVERRIDE;

#pragma endregion Interface
};

/*
 Single lock
*/
#ifdef SINGLE_LOCK

class single_lock : public base_lock
{
protected:
	// Main lock
	CRITICAL_SECTION	_section;// follow LifeCycle member(move & create new, destroy in dtor)
	bool				_bLocked;

protected:
	LFCTYPE void	ClearPreAssign()
	{
		DeleteCriticalSection(&_section);
		SecureZeroMemory(&_section, sizeof(CRITICAL_SECTION));
	}
	LFCTYPE void	ClearMove()
	{
		_bLocked = false;
		SecureZeroMemory(&_section, sizeof(CRITICAL_SECTION));

		// Recreate Lifecycle member
		InitializeCriticalSection(&_section);
	}
	void			Release()
	{
		// Lock dying
		LOCK_LOCKDYING_INTERNAL;

		single_lock::ClearPreAssign();
	}
private:
	single_lock(single_lock&);
	single_lock& operator = (single_lock&);
public:
	single_lock();
	single_lock(single_lock&&);
	virtual			~single_lock() OVERRIDE;

	int				AssignMove(single_lock&);
	single_lock& operator = (single_lock&&);

#pragma region Interface

	inline bool		IsLocked()
	{
		return _bLocked;
	}
	virtual inline int	TryLock() OVERRIDE
	{
		// Try lock internal
		LOCK_CHECK_TRYLOCK_INTERNAL;

		// Try lock
		RET_ON_ZERO_AND(
			TryEnterCriticalSection(&_section),
			LOCK_UNLOCK_INTERNAL);

		_bLocked = true;

		// Unlock internal
		LOCK_UNLOCK_INTERNAL;

		return LOCK_RET::LOCK_SUCCEED;
	}
	virtual inline int	Lock(ms_t1 msTimeout = LOCK_TIME_INFINITE) OVERRIDE;
	virtual inline int	TryUnLock() OVERRIDE
	{
		// Try lock internal
		LOCK_CHECK_TRYLOCK_INTERNAL;

		// If not locked
		LOCK_CHECK_LOCKED(!_bLocked);

		// Unlock
		LeaveCriticalSection(&_section);
		_bLocked = false;

		// Unlock internal
		LOCK_UNLOCK_INTERNAL;

		return LOCK_RET::LOCK_SUCCEED;
	}
	virtual inline int	UnLock() OVERRIDE;

#pragma endregion Interface
};

#endif


/*
 Read-Write-Block lock
*/
#define RWBLOCK_STATUS_LOCK		0x01
#define RWBLOCK_STATUS_IREAD	0x02
#define RWBLOCK_STATUS_READ		0x04
#define RWBLOCK_STATUS_WRITE	0x08
#define RWBLOCK_STATUS_BLOCK	0x10
#define RWBLOCK_STATUS_ISLOCKED(status)		(((status) & RWBLOCK_STATUS_LOCK) != 0)
#define RWBLOCK_STATUS_NOTLOCKED(status)	(((status) & RWBLOCK_STATUS_LOCK) == 0)
#define RWBLOCK_STATUS_IREADLOCKED(status)	(((status) & RWBLOCK_STATUS_IREAD) != 0)
#define RWBLOCK_STATUS_NOTIREADLOCKED(status)	(((status) & RWBLOCK_STATUS_IREAD) == 0)
#define RWBLOCK_STATUS_READLOCKED(status)	(((status) & RWBLOCK_STATUS_READ) != 0)
#define RWBLOCK_STATUS_NOTREADLOCKED(status)	(((status) & RWBLOCK_STATUS_READ) == 0)
#define RWBLOCK_STATUS_WRITELOCKED(status)	(((status) & RWBLOCK_STATUS_WRITE) != 0)
#define RWBLOCK_STATUS_NOTWRITELOCKED(status)	(((status) & RWBLOCK_STATUS_WRITE) == 0)
#define RWBLOCK_STATUS_BLOCKLOCKED(status)	(((status) & RWBLOCK_STATUS_BLOCK) != 0)
#define RWBLOCK_STATUS_NOTBLOCKLOCKED(status)	(((status) & RWBLOCK_STATUS_BLOCK) == 0)
enum RWBLOCK_STATUS
{
	RWB_UNLOCK = 0,
	RWB_IREAD = RWBLOCK_STATUS_LOCK | RWBLOCK_STATUS_IREAD,
	RWB_READ = RWBLOCK_STATUS_LOCK | RWBLOCK_STATUS_READ,
	RWB_WRITE = RWBLOCK_STATUS_LOCK | RWBLOCK_STATUS_WRITE,
	RWB_BLOCK = RWBLOCK_STATUS_LOCK | RWBLOCK_STATUS_BLOCK,
	RWB_IREADWRITE = RWBLOCK_STATUS_LOCK | RWBLOCK_STATUS_IREAD | RWBLOCK_STATUS_WRITE,
	RWB_IREADREAD = RWBLOCK_STATUS_LOCK | RWBLOCK_STATUS_IREAD | RWBLOCK_STATUS_READ
};


#define RWBLOCK_DEFAULT_RES				(10)

// Locked status
#define RWBLOCK_LOCKCOUNTIREAD				(_nLockIR)
#define RWBLOCK_LOCKCOUNTREAD				(_nLockR)
#define RWBLOCK_LOCKED_INTERNAL				(RWBLOCK_STATUS_ISLOCKED(_status))
#define RWBLOCK_NOTLOCKED_INTERNAL			(RWBLOCK_STATUS_NOTLOCKED(_status))
#define RWBLOCK_LOCKEDIREAD_INTERNAL		(RWBLOCK_STATUS_IREADLOCKED(_status))
#define RWBLOCK_NOTLOCKEDIREAD_INTERNAL		(RWBLOCK_STATUS_NOTIREADLOCKED(_status))
#define RWBLOCK_LOCKEDREAD_INTERNAL			(RWBLOCK_STATUS_READLOCKED(_status))
#define RWBLOCK_NOTLOCKEDREAD_INTERNAL		(RWBLOCK_STATUS_NOTREADLOCKED(_status))
#define RWBLOCK_LOCKEDWRITE_INTERNAL		(RWBLOCK_STATUS_WRITELOCKED(_status))
#define RWBLOCK_NOTLOCKEDWRITE_INTERNAL		(RWBLOCK_STATUS_NOTWRITELOCKED(_status))
#define RWBLOCK_LOCKEDBLOCK_INTERNAL		(RWBLOCK_STATUS_BLOCKLOCKED(_status))
#define RWBLOCK_NOTLOCKEDBLOCK_INTERNAL		(RWBLOCK_STATUS_NOTBLOCKLOCKED(_status))

// Full locked
#define RWBLOCK_FULLLOCKEDIREAD_INTERNAL	( RWBLOCK_LOCKEDBLOCK_INTERNAL ||\
											((RWBLOCK_LOCKEDIREAD_INTERNAL) &&\
											(RWBLOCK_LOCKCOUNTIREAD >= _nRes)) )
#define RWBLOCK_FULLLOCKEDREAD_INTERNAL		( RWBLOCK_LOCKEDBLOCK_INTERNAL ||\
											RWBLOCK_LOCKEDWRITE_INTERNAL ||\
											(RWBLOCK_LOCKEDREAD_INTERNAL &&\
											(RWBLOCK_LOCKCOUNTREAD >= _nRes)) )
#define RWBLOCK_FULLLOCKEDWRITE_INTERNAL	( RWBLOCK_LOCKEDBLOCK_INTERNAL ||\
											RWBLOCK_LOCKEDWRITE_INTERNAL ||\
											RWBLOCK_LOCKEDREAD_INTERNAL)
#define RWBLOCK_FULLLOCKEDBLOCK_INTERNAL	( RWBLOCK_LOCKED_INTERNAL)

// Set
#define RWBLOCK_SETTID_INTERNAL			_tid = (tid_t)GET_TID;
#define RWBLOCK_RESETTID_INTERNAL		_tid = INVALID_TID;
// status TODO
#define RWBLOCK_SETSTATUS_INTERNAL(S)		_status = (S);
#define RWBLOCK_SETSTATUS_IREAD_INTERNAL	_status = (RWBLOCK_STATUS)(_status | RWBLOCK_STATUS::RWB_IREAD);
#define RWBLOCK_SETSTATUS_READ_INTERNAL		_status = (RWBLOCK_STATUS)(_status | RWBLOCK_STATUS::RWB_READ);
#define RWBLOCK_SETSTATUS_WRITE_INTERNAL	_status = (RWBLOCK_STATUS)(_status | RWBLOCK_STATUS::RWB_WRITE);
#define RWBLOCK_SETSTATUS_UNIREAD_INTERNAL	if(_nLockIR <= 0){\
												if(RWBLOCK_LOCKEDWRITE_INTERNAL){\
													_status = RWBLOCK_STATUS::RWB_WRITE;}\
												else if(RWBLOCK_LOCKEDREAD_INTERNAL){\
													_status = RWBLOCK_STATUS::RWB_READ;}\
												else{\
													_status = RWBLOCK_STATUS::RWB_UNLOCK;}\
											}
#define RWBLOCK_SETSTATUS_UNREAD_INTERNAL	if(_nLockR <= 0){\
												if(RWBLOCK_LOCKEDIREAD_INTERNAL){\
													_status = RWBLOCK_STATUS::RWB_IREAD;}\
												else{\
													_status = RWBLOCK_STATUS::RWB_UNLOCK;}\
											}
#define RWBLOCK_SETSTATUS_UNWRITE_INTERNAL	if(RWBLOCK_LOCKEDIREAD_INTERNAL){\
												_status = RWBLOCK_STATUS::RWB_IREAD;}\
											else{\
												_status = RWBLOCK_STATUS::RWB_UNLOCK;}
#define RWBLOCK_SETSTATUS_UNBLOCK_INTERNAL	_status = RWBLOCK_STATUS::RWB_UNLOCK;

// Lock Internal
#define RWBLOCK_LOCKIREAD_INTERNAL		DO_SOMETHING(\
											++_nLockIR;\
											tid_t tid = (tid_t)GET_TID;\
											++_mapLockIR[tid];\
											RWBLOCK_SETSTATUS_IREAD_INTERNAL;)
#define RWBLOCK_LOCKREAD_INTERNAL		DO_SOMETHING(\
											++_nLockR;\
											tid_t tid = (tid_t)GET_TID;\
											++_mapLockR[tid];\
											RWBLOCK_SETSTATUS_READ_INTERNAL;)
#define RWBLOCK_LOCKWRITE_INTERNAL		DO_SOMETHING(\
											RWBLOCK_SETTID_INTERNAL;\
											RWBLOCK_SETSTATUS_WRITE_INTERNAL;)
#define RWBLOCK_LOCKBLOCK_INTERNAL		DO_SOMETHING(\
											RWBLOCK_SETTID_INTERNAL;\
											RWBLOCK_SETSTATUS_INTERNAL(RWBLOCK_STATUS::RWB_BLOCK);)
// Unlock Internal
#define RWBLOCK_UNLOCKIREAD_INTERNAL(IR)	DO_SOMETHING(\
											--_nLockIR;\
											--((IR)->second);\
											RWBLOCK_SETSTATUS_UNIREAD_INTERNAL;)
#define RWBLOCK_UNLOCKREAD_INTERNAL(R)		DO_SOMETHING(\
											--_nLockR;\
											--((R)->second);\
											RWBLOCK_SETSTATUS_UNREAD_INTERNAL;)
#define RWBLOCK_UNLOCKWRITE_INTERNAL	DO_SOMETHING(\
											RWBLOCK_RESETTID_INTERNAL;\
											RWBLOCK_SETSTATUS_UNWRITE_INTERNAL;)
#define RWBLOCK_UNLOCKBLOCK_INTERNAL	DO_SOMETHING(\
											RWBLOCK_RESETTID_INTERNAL;\
											RWBLOCK_SETSTATUS_UNBLOCK_INTERNAL;)

class icrwb_lock : public base_lock
{
public:
	typedef unordered_map<tid_t, int>	rmap;
	typedef __int64						nlock;
protected:
	RWBLOCK_STATUS		_status;
	nlock				_nRes;


	tid_t				_tid;

	nlock				_nLockR;
	rmap				_mapLockR;

	nlock				_nLockIR;
	rmap				_mapLockIR;

protected:
	LFCTYPE void	ClearPreAssign()
	{
	}
	LFCTYPE void	ClearMove()
	{
		_status = RWBLOCK_STATUS::RWB_UNLOCK;
		//_nRes;
		_tid = INVALID_TID;
		_nLockIR = 0;
	}
	void			Release()
	{
	}
private:
	icrwb_lock(icrwb_lock&);
	icrwb_lock& operator = (icrwb_lock&);
public:
	icrwb_lock();
	icrwb_lock(icrwb_lock&& lock) noexcept;
	virtual			~icrwb_lock() OVERRIDE;

	int				AssignMove(icrwb_lock&) noexcept;
	icrwb_lock& operator = (icrwb_lock&& lock) noexcept;

#pragma region Interface

	// Is r/w locked
	virtual inline bool	IsLocked() OVERRIDE
	{
		return RWBLOCK_LOCKED_INTERNAL;
	}
	inline bool		SetResource(int nRes)
	{
		if (LOCK_INFINITE_RES == nRes)
		{
			_nRes = LOCK_INFINITE_RES_ACTUAL;
			return true;
		}
		else if (nRes < 0)
		{
			return false;
		}

		_nRes = nRes;

		return true;
	}
	virtual inline int	TryLock() OVERRIDE
	{
		// Try lock internal
		LOCK_CHECK_TRYLOCK_INTERNAL;

		tid_t tid = (tid_t)GET_TID;

		// Check lock free
		LOCK_CHECK_FREE(RWBLOCK_FULLLOCKEDBLOCK_INTERNAL);

		// Lock
		RWBLOCK_LOCKBLOCK_INTERNAL;

		// Unlock internal
		LOCK_UNLOCK_INTERNAL;

		return LOCK_RET::LOCK_SUCCEED;
	}
	virtual inline int	TryLockIRead()
	{
		// Try lock internal
		LOCK_CHECK_TRYLOCK_INTERNAL;

		// Check lock free
		LOCK_CHECK_FREE(RWBLOCK_FULLLOCKEDIREAD_INTERNAL);

		// Lock
		RWBLOCK_LOCKIREAD_INTERNAL;

		// Unlock internal
		LOCK_UNLOCK_INTERNAL;

		return LOCK_RET::LOCK_SUCCEED;
	}
	virtual inline int	TryLockRead()
	{
		// Try lock internal
		LOCK_CHECK_TRYLOCK_INTERNAL;

		// Check lock free
		LOCK_CHECK_FREE(RWBLOCK_FULLLOCKEDREAD_INTERNAL);

		// Lock
		RWBLOCK_LOCKREAD_INTERNAL;

		// Unlock internal
		LOCK_UNLOCK_INTERNAL;

		return LOCK_RET::LOCK_SUCCEED;
	}
	virtual inline int	TryLockWrite()
	{
		// Try lock internal
		LOCK_CHECK_TRYLOCK_INTERNAL;

		// Check lock free
		LOCK_CHECK_FREE(RWBLOCK_FULLLOCKEDWRITE_INTERNAL);

		// Lock
		RWBLOCK_LOCKWRITE_INTERNAL;

		// Unlock internal
		LOCK_UNLOCK_INTERNAL;

		return LOCK_RET::LOCK_SUCCEED;
	}
	int	TryLockIReadGuard(scope_guard& scg)
	{
		int ret = TryLockIRead();
		RET_ON_NP(
			ret);

		scg.SetAndEnable(
			[this]() {
				UnLockIRead();
			});

		return ret;
	}
	int	TryLockReadGuard(scope_guard& scg)
	{
		int ret = TryLockRead();
		RET_ON_NP(
			ret);

		scg.SetAndEnable(
			[this]() {
				UnLockRead();
			});

		return ret;
	}
	int	TryLockWriteGuard(scope_guard& scg)
	{
		int ret = TryLockWrite();
		RET_ON_NP(
			ret);

		scg.SetAndEnable(
			[this]() {
				UnLockWrite();
			});

		return ret;
	}
	virtual int	Lock(ms_t1 msTimeout = LOCK_TIME_INFINITE) OVERRIDE;
	virtual int	LockIRead(ms_t1 msTimeout = LOCK_TIME_INFINITE);
	virtual int	LockRead(ms_t1 msTimeout = LOCK_TIME_INFINITE);
	virtual int	LockWrite(ms_t1 msTimeout = LOCK_TIME_INFINITE);

	int LockIReadGuard(
		scope_guard& scg, ms_t1 msTimeout = LOCK_TIME_INFINITE);
	int LockReadGuard(
		scope_guard& scg, ms_t1 msTimeout = LOCK_TIME_INFINITE);
	int LockWriteGuard(
		scope_guard& scg, ms_t1 msTimeout = LOCK_TIME_INFINITE);

	virtual inline int	TryUnLock() OVERRIDE
	{
		// Try lock internal
		LOCK_CHECK_TRYLOCK_INTERNAL;

		// Check locked
		LOCK_CHECK_LOCKED(RWBLOCK_NOTLOCKEDBLOCK_INTERNAL);

		// Check thread ID
		LOCK_CHECK_MISMATCH((tid_t)GET_TID != _tid);

		// Unlock
		RWBLOCK_UNLOCKBLOCK_INTERNAL;

		// Unlock internal
		LOCK_UNLOCK_INTERNAL;

		return LOCK_RET::LOCK_SUCCEED;
	}
	virtual inline int	TryUnLockIRead()
	{
		// Try lock internal
		LOCK_CHECK_TRYLOCK_INTERNAL;

		// Check locked
		LOCK_CHECK_LOCKED(RWBLOCK_NOTLOCKEDIREAD_INTERNAL);

		tid_t tid = (tid_t)GET_TID;
		rmap::iterator itr = _mapLockIR.find(tid);
		// Check thread ID
		LOCK_CHECK_MISMATCH(_mapLockIR.end() == itr);
		LOCK_CHECK_MISMATCH(itr->second <= 0);

		// Unlock
		RWBLOCK_UNLOCKIREAD_INTERNAL(itr);

		// Unlock internal
		LOCK_UNLOCK_INTERNAL;

		return LOCK_RET::LOCK_SUCCEED;
	}
	virtual inline int	TryUnLockWrite()
	{
		// Try lock internal
		LOCK_CHECK_TRYLOCK_INTERNAL;

		// Check locked
		LOCK_CHECK_LOCKED(RWBLOCK_NOTLOCKEDWRITE_INTERNAL);

		// Check thread ID
		LOCK_CHECK_MISMATCH((tid_t)GET_TID != _tid);

		// Unlock
		RWBLOCK_UNLOCKWRITE_INTERNAL;

		// Unlock internal
		LOCK_UNLOCK_INTERNAL;

		return LOCK_RET::LOCK_SUCCEED;
	}
	virtual int	UnLock() OVERRIDE;
	virtual int	UnLockIRead();
	virtual int	UnLockRead();
	virtual int	UnLockWrite();

#pragma endregion Interface
};

#pragma region Lock


#pragma region Atomic
/************************
 Atomic
*************************/
#define ATOM_INIT_INTERNAL(v, T)			v = new T();

#define ATOM_CAS_INTERNAL(v, e, u)			CAS(v, e, u)
#define ATOM_CASF_INTERNAL(v, e, u)			CASFAIL(v, e, u)
#define ATOM_CASS_INTERNAL(v, e, u)			CASSUCCEED(v, e, u)
#define ATOM_SET_INTERNAL(v, u)				EXC((v), (u))
#define ATOM_EQU_INTERNAL(v, e)				EQU((v), (e))

#define PURE_ATOM_GET
#ifdef PURE_ATOM_GET
#define ATOM_GET_INTERNAL(v, val)			val		= *(v);
#else
#define ATOM_GET_INTERNAL(v, val)			val		= *(v);\
											while(!ATOM_EQU_INTERNAL(v, val)){\
												val		= *(v);}
#endif

#define ATOM_TYPEAT_INTERNAL(vv, T)				*((T*)(&vv))
#define ATOM_SETVAL_INTERNAL(vv, val, T)		ATOM_TYPEAT_INTERNAL(vv, T) = val;
#define ATOM_GETVAL_INTERNAL(vv, val, T)		val = ATOM_TYPEAT_INTERNAL(vv, T);

template<class T, typename = typename enable_if_pod8<T>::type>
class atom8
{
protected:
	typedef unsigned __int64	cas_t;

protected:
	volatile cas_t* _obj;

protected:
	LFCTYPE void	ClearPreAssign();
	LFCTYPE void	ClearMove();
	void			Release()
	{
		SAFE_DELETE(_obj);
	}
public:
	atom8() :
		_obj(nullptr)
	{
		ATOM_INIT_INTERNAL(_obj, cas_t);
	}
	atom8(T vT) :
		_obj(nullptr)
	{
		ATOM_INIT_INTERNAL(_obj, cas_t);

		cas_t vO = cas_t();
		ATOM_SETVAL_INTERNAL(vO, vT, T);

		ATOM_SET_INTERNAL(_obj, vO);
	}
	atom8(const atom8& atm) :
		_obj(nullptr)
	{
		ATOM_INIT_INTERNAL(_obj, cas_t);

		Assign(atm);
	}
	~atom8()
	{
		SAFE_DELETE(_obj);
	}

	int				Assign(const atom8& atm)
	{
		// Get Other obj value
		cas_t vO;
		ATOM_GET_INTERNAL(atm._obj, vO);

		ATOM_SET_INTERNAL(_obj, vO);

		return 1;
	}
	atom8& operator = (const atom8& atm)
	{
		Assign(atm);

		return *this;
	}

	int		clear()
	{
		ATOM_SET_INTERNAL(_obj, T());

		return 1;
	}

	void	inc()
	{
		// Get obj value
		cas_t vO;
		ATOM_GET_INTERNAL(_obj, vO);

		// Get T value
		T vT;
		ATOM_GETVAL_INTERNAL(vO, vT, T);
		// Increment
		T vTU = vT;
		++vTU;

		// Get obj value of inced
		cas_t vOU = cas_t();
		ATOM_SETVAL_INTERNAL(vOU, vTU, T);

		while (ATOM_CASF_INTERNAL(_obj, vO, vOU))
		{
			// Get obj value
			ATOM_GET_INTERNAL(_obj, vO);

			// Get T value
			ATOM_GETVAL_INTERNAL(vO, vT, T);
			// Increment
			vTU = vT;
			++vTU;

			// Get obj value of inced
			ATOM_SETVAL_INTERNAL(vOU, vTU, T);
		}
	}

	void	dec()
	{
		// Get obj value
		cas_t vO;
		ATOM_GET_INTERNAL(_obj, vO);

		// Get T value
		T vT;
		ATOM_GETVAL_INTERNAL(vO, vT, T);
		// Decrement
		T vTU = vT;
		--vTU;

		// Get obj value of inced
		cas_t vOU = cas_t();
		ATOM_SETVAL_INTERNAL(vOU, vTU, T);

		while (ATOM_CASF_INTERNAL(_obj, vO, vOU))
		{
			// Get obj value
			ATOM_GET_INTERNAL(_obj, vO);

			// Get T value
			ATOM_GETVAL_INTERNAL(vO, vT, T);
			// Decrement
			vTU = vT;
			--vTU;

			// Get obj value of inced
			ATOM_SETVAL_INTERNAL(vOU, vTU, T);
		}
	}
	//T*				operator -> ()
	//{
	//	return &_obj;
	//}
	//T&				operator*()
	//{
	//	return _obj;
	//}
	//const T&		operator*() const
	//{
	//	return _obj;
	//}
	T		get() const
	{
		// Get obj value
		cas_t vO;
		ATOM_GET_INTERNAL(_obj, vO);

		// Get T value
		T vT;
		ATOM_GETVAL_INTERNAL(vO, vT, T);

		return vT;
	}
	bool	set(const T& vT)
	{
		cas_t vO = cas_t();
		ATOM_SETVAL_INTERNAL(vO, vT, T);

		// Set and get old value
		cas_t vOOld = ATOM_SET_INTERNAL(_obj, vO);

		// Get old T value
		T vTOld;
		ATOM_GETVAL_INTERNAL(vOOld, vTOld, T);

		return (vTOld != vT);
	}
};

template<>
class atom8<bool>
{
protected:
	typedef unsigned __int64	cas_t;

protected:
	volatile cas_t* _obj;

protected:
	LFCTYPE void	ClearPreAssign();
	LFCTYPE void	ClearMove();
	void			Release();
public:
	atom8();
	atom8(bool vT);
	atom8(const atom8& atm);
	~atom8();

	int				Assign(atom8& atm);
	atom8& operator = (atom8& atm);

	int Clear();

	void	flip();
	//T*				operator -> ()
	//{
	//	return &_obj;
	//}
	//T&				operator*()
	//{
	//	return _obj;
	//}
	//const T&		operator*() const
	//{
	//	return _obj;
	//}
	bool	get() const;
	bool	set(bool vT);
};



#define ATOM_TRYLOCK_INTERNAL				its_t::TryLock()
#define ATOM_UNLOCK_INTERNAL				its_t::Unlock();

#define ATOM_LOCKSCG_INTERNAL				scope_guard scgLock;\
											its_t::LockGuard(scgLock);
#define ATOM_LOCKTIMESCG_INTERNAL(TO, RET)	scope_guard scgLock;\
											bool bInfiniteTimeout = LOCK_TIME_ISINFINITE(msTimeout);\
											RET = (bInfiniteTimeout ? its_t::LockGuard(scgLock) : its_t::LockGuard(scgLock, TO));
#define ATOM_LOCKOTHERSCG_INTERNAL(O)		scope_guard scgLockOther;\
											(O).its_t::LockGuard(scgLockOther);
#define ATOM_UNLOCKSCG_INTERNAL				scgLock.Release();
#define ATOM_UNLOCKOTHERSCG_INTERNAL		scgLockOther.Release();

#define ATOM_LOCKDYING_INTERNAL				its_t::LockDying();

template<class T>
class atom : protected ITSCAS
{
protected:
	typedef ITSCAS	its_t;
protected:
	volatile T		_obj;

protected:
	LFCTYPE void	ClearPreAssign();
	LFCTYPE void	ClearMove();
	void			Release()
	{
	}
private:
	// Disable copy-ctor, = operator
	atom(atom& atm);
	atom& operator = (atom& atm);
public:
	atom() :
		_obj()
	{
	}
	atom(const T& vT) :
		_obj()
	{
		// ======>> LOCK <<=======
		ATOM_LOCKSCG_INTERNAL;

		_obj = vT;
	}
	atom(atom&& atm) :
		_obj()
	{
		// Assign
		AssignMove(atm);
	}
	~atom()
	{
		// ======>> LOCK DYING <<=======
		ATOM_LOCKDYING_INTERNAL;
	}

	int				AssignMove(atom& atm)
	{
		// ======>> LOCK OTHER <<=======
		ATOM_LOCKOTHERSCG_INTERNAL(atm);
		// ======>> LOCK <<=======
		ATOM_LOCKSCG_INTERNAL;

		// Assign
		_obj = std::forward<atom>(atm._obj);

		return 1;
	}
	atom& operator = (atom&& atm)
	{
		AssignMove(atm);

		return *this;
	}

	int		clear()
	{
		// ======>> LOCK <<=======
		ATOM_LOCKSCG_INTERNAL;

		_obj = T();

		return 1;
	}

	//T*				operator -> ()
	//{
	//	return &_obj;
	//}
	//T&				operator*()
	//{
	//	return _obj;
	//}
	//const T&		operator*() const
	//{
	//	return _obj;
	//}
	void	inc()
	{
		// ======>> LOCK <<=======
		ATOM_LOCKSCG_INTERNAL;

		_obj = _obj + 1;
	}
	void	dec()
	{
		// ======>> LOCK <<=======
		ATOM_LOCKSCG_INTERNAL;

		_obj = _obj - 1;
	}
	T		get()
	{
		// ======>> LOCK <<=======
		ATOM_LOCKSCG_INTERNAL;

		return _obj;
	}
	T		get(bool& bGot, ms_t1 msTimeout)
	{
		// Init flag
		bGot = false;

		// ======>> LOCK <<=======
		bool bInfiniteTimeout = LOCK_TIME_ISINFINITE(msTimeout);
		scope_guard scgLock;
		bool bLock = (bInfiniteTimeout ? its_t::LockGuard(scgLock) : its_t::LockGuard(scgLock, msTimeout));
		RETVAL_ON_FALSE(
			bLock,
			-1);

		// Update flag
		bGot = true;

		return _obj;
	}
	int		set(const T& vT)
	{
		// ======>> LOCK <<=======
		ATOM_LOCKSCG_INTERNAL;

		T objOld = _obj;
		_obj = vT;

		return (objOld != vT) ? 1 : 0;
	}
	int		set(const T& vT, ms_t1 msTimeout)
	{
		// ======>> LOCK <<=======
		bool bInfiniteTimeout = LOCK_TIME_ISINFINITE(msTimeout);
		scope_guard scgLock;
		bool bLock = (bInfiniteTimeout ? its_t::LockGuard(scgLock) : its_t::LockGuard(scgLock, msTimeout));
		RETVAL_ON_FALSE(
			bLock,
			-1);

		T objOld = _obj;
		_obj = vT;

		return (objOld != vT) ? 1 : 0;
	}
	int		lock(scope_guard& scgLock, ms_t1 msTimeout)
	{
		// ======>> LOCK <<=======
		bool bInfiniteTimeout = LOCK_TIME_ISINFINITE(msTimeout);
		bool bLock = (bInfiniteTimeout ? its_t::LockGuard(scgLock) : its_t::LockGuard(scgLock, (ms_t)msTimeout));
		RETVAL_ON_FALSE(
			bLock,
			-1);

		return 1;
	}
	template<class T, class R>
	int		execute(const function<R(T*)>& func, R& ret, ms_t1 msTimeout)
	{
		// ======>> LOCK <<=======
		bool bInfiniteTimeout = LOCK_TIME_ISINFINITE(msTimeout);
		scope_guard scgLock;
		bool bLock = (bInfiniteTimeout ? its_t::LockGuard(scgLock) : its_t::LockGuard(scgLock, (ms_t)msTimeout));
		RETVAL_ON_FALSE(
			bLock,
			-1);

		ret = func((T*)&_obj);

		return 1;
	}
	template<class T, class R>
	int		execute(const function<R(const T*)>& func, R& ret, ms_t1 msTimeout) const
	{
		// ======>> LOCK <<=======
		bool bInfiniteTimeout = LOCK_TIME_ISINFINITE(msTimeout);
		scope_guard scgLock;
		bool bLock = (bInfiniteTimeout ? its_t::LockGuard(scgLock) : its_t::LockGuard(scgLock, (ms_t)msTimeout));
		RETVAL_ON_FALSE(
			bLock,
			-1);

		ret = func((const T*)&_obj);

		return 1;
	}
	template<class T>
	int		execute(const function<void(T*)>& func, ms_t1 msTimeout)
	{
		// ======>> LOCK <<=======
		bool bInfiniteTimeout = LOCK_TIME_ISINFINITE(msTimeout);
		scope_guard scgLock;
		bool bLock = (bInfiniteTimeout ? its_t::LockGuard(scgLock) : its_t::LockGuard(scgLock, (ms_t)msTimeout));
		RETVAL_ON_FALSE(
			bLock,
			-1);

		func((T*)&_obj);

		return 1;
	}
};


#define ATOMBOOL_XOR_MASK	0x01

template<>
class atom<bool>
{
public:
	typedef unsigned int	cas_t;

	enum ATOMBOOL { ATOM_BOOL_FALSE = 0, ATOM_BOOL_TRUE = 1 };
private:
	volatile cas_t* _v;

protected:
	LFCTYPE void	ClearPreAssign();
	LFCTYPE void	ClearMove();
	void			Release();
public:
	atom();
	atom(bool vT);
	atom(const atom& atm);
	~atom();

	int				Assign(const atom& atm);
	atom<bool>& operator = (const atom& atm);

	int		clear();

	//T*				operator -> ()
	//{
	//	return &_obj;
	//}
	//T&				operator*()
	//{
	//	return _obj;
	//}
	//const T&		operator*() const
	//{
	//	return _obj;
	//}
	void	flip();
	bool	get() const;
	bool	set(bool vT);
	bool	checkandflip(bool chk);
};


//#undef ATOM_INIT_INTERNAL
//
//#undef ATOM_CAS_INTERNAL
//#undef ATOM_CASF_INTERNAL
//#undef ATOM_SET_INTERNAL
//#undef ATOM_EQU_INTERNAL
//#undef ATOM_GET_INTERNAL
//
//#undef ATOM_TYPEAT_INTERNAL
//#undef ATOM_SETVAL_INTERNAL
//#undef ATOM_GETVAL_INTERNAL

#pragma endregion Atomic

#undef _IMGDISP_SOURCE_FILE_LOCK_H