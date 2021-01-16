#define _IMGDISP_SOURCE_FILE_LOCK_CPP

#include "pch.h"

#include "lock.h"



/************************
 Lock
*************************/

#pragma region base_lock

int base_lock::AssignMove(base_lock& lck)
{
	return 1;
}

base_lock& base_lock::operator = (base_lock&& lock)
{
	return *this;
}

int base_lock::LockGuard(scope_guard& scg, ms_t1 msTimeout)
{
	int ret = Lock(msTimeout);
	RET_ON_NP_AND(
		ret,
		Logger::Log0("Lock failed In, ret:" + to_string(ret)););

	//Logger::Log0("Lock In");

	scg.SetAndEnable(
		[this]()
		{
			UnLock();
		});

	return ret;
}

#pragma endregion base_lock

#pragma region naive_lock

naive_lock::naive_lock() :
	base_lock(),
	_tid(INVALID_TID),
	_bLocked(false)
{
}

naive_lock::naive_lock(naive_lock&& lck) :
	base_lock(),
	_tid(INVALID_TID),
	_bLocked(false)
{
	// ======>> LOCK OTHER <<=======
	LOCK_LOCKOTHERSCG_INTERNAL(lck);
	// ======>> LOCK <<=======
	LOCK_LOCKSCG_INTERNAL;

	// Assign
	base_lock::AssignMove(lck);
	NAIVELOCK_ASSIGN_INTERNAL(lck);

	// ======>> UNLOCK <<=======
	LOCK_UNLOCKSCG_INTERNAL;

	// Clear after move
	lck.naive_lock::ClearMove();
}

naive_lock::~naive_lock()
{
	// ======>> LOCK DYING <<=======
	LOCK_LOCKDYING_INTERNAL;

	_bLocked = false;
	_tid = INVALID_TID;
}

int naive_lock::AssignMove(naive_lock& lck)
{
	// ======>> LOCK OTHER <<=======
	LOCK_LOCKOTHERSCG_INTERNAL(lck);
	// ======>> LOCK <<=======
	LOCK_LOCKSCG_INTERNAL;

	// Clear before assignment
	naive_lock::ClearPreAssign();

	// Assign
	base_lock::AssignMove(lck);
	NAIVELOCK_ASSIGN_INTERNAL(lck);

	// ======>> UNLOCK <<=======
	LOCK_UNLOCKSCG_INTERNAL;

	// Clear after move
	lck.naive_lock::ClearMove();

	return LOCK_RET::LOCK_SUCCEED;
}

naive_lock& naive_lock::operator = (naive_lock&& lock)
{
	AssignMove(lock);

	return *this;
}

int naive_lock::Lock(ms_t1 msTimeout)
{
	const sec_t sTimeout = MS_TO_SEC(msTimeout);
	bool bInfiniteTimeout = LOCK_TIME_ISINFINITE(msTimeout);

	Ticker ticker;
	ticker.Start();

	while (true)
	{
		// ======>> LOCK <<=======
		if (LOCK_TRYLOCK_INTERNAL)
		{
			if (!_bLocked)
			{
				// Lock
				NAIVELOCK_LOCK_INTERNAL;

				// ======>> UNLOCK <<=======
				LOCK_UNLOCK_INTERNAL;
				return LOCK_RET::LOCK_SUCCEED;
			}

			// ======>> UNLOCK <<=======
			LOCK_UNLOCK_INTERNAL;
		}

		// Timeout
		if (!bInfiniteTimeout)
		{
			LOCK_CHECK_TIMEOUTSCG(ticker.Tick(), sTimeout);
		}

		Sleep(LOCK_SLEEPCYCLE);
	}

	return LOCK_RET::LOCK_SUCCEED;
}

int naive_lock::UnLock()
{
	// ======>> LOCK <<=======
	LOCK_LOCKSCG_INTERNAL;

	// If not locked
	LOCK_CHECK_LOCKEDSCG(!_bLocked);

	// Check thread ID
	LOCK_CHECK_MISMATCHSCG((tid_t)GET_TID != _tid);

	// Unlock
	NAIVELOCK_UNLOCK_INTERNAL;

	return LOCK_RET::LOCK_SUCCEED;
}

#pragma endregion naive_lock

#pragma region count_lock

count_lock::count_lock() :
	base_lock(),
	_nRes(COUNTLOCK_DEFAULT_RES),
	_tid(INVALID_TID),
	_nLock(0)
{
}

count_lock::count_lock(count_lock&& lck) :
	base_lock(),
	_nRes(COUNTLOCK_DEFAULT_RES),
	_tid(INVALID_TID),
	_nLock(0)
{
	// ======>> LOCK OTHER <<=======
	LOCK_LOCKOTHERSCG_INTERNAL(lck);
	// ======>> LOCK <<=======
	LOCK_LOCKSCG_INTERNAL;

	// Assign
	base_lock::AssignMove(lck);
	COUNTLOCK_ASSIGN_INTERNAL(lck);

	// ======>> UNLOCK <<=======
	LOCK_UNLOCKSCG_INTERNAL;

	// Clear after move
	lck.count_lock::ClearMove();
}

count_lock::~count_lock()
{
	// ======>> LOCK DYING <<=======
	LOCK_LOCKDYING_INTERNAL;

	_nLock = 0;
	_tid = INVALID_TID;
}

int count_lock::AssignMove(count_lock& lck)
{
	// ======>> LOCK OTHER <<=======
	LOCK_LOCKOTHERSCG_INTERNAL(lck);
	// ======>> LOCK <<=======
	LOCK_LOCKSCG_INTERNAL;

	// Clear before assignment
	count_lock::ClearPreAssign();

	// Assign
	base_lock::AssignMove(lck);
	COUNTLOCK_ASSIGN_INTERNAL(lck);

	// ======>> UNLOCK <<=======
	LOCK_UNLOCKSCG_INTERNAL;

	// Clear after move
	lck.count_lock::ClearMove();

	return LOCK_RET::LOCK_SUCCEED;
}

count_lock& count_lock::operator = (count_lock&& lock)
{
	AssignMove(lock);

	return *this;
}

int count_lock::Lock(ms_t1 msTimeout)
{
	const sec_t sTimeout = MS_TO_SEC(msTimeout);
	bool bInfiniteTimeout = LOCK_TIME_ISINFINITE(msTimeout);

	Ticker ticker;
	ticker.Start();

	// Get thread ID
	tid_t tid = (tid_t)GET_TID;

	// ======>> LOCK <<=======
	if (LOCK_TRYLOCK_INTERNAL)
	{
		if (COUNTLOCK_AVAILABLE_INTERNAL(tid))
		{
			// Lock
			COUNTLOCK_LOCK_INTERNAL(tid);

			// ======>> UNLOCK <<=======
			LOCK_UNLOCK_INTERNAL;
			return LOCK_RET::LOCK_SUCCEED;
		}

		// ======>> UNLOCK <<=======
		LOCK_UNLOCK_INTERNAL;
	}

	while (true)
	{
		// ======>> LOCK <<=======
		if (LOCK_TRYLOCK_INTERNAL)
		{
			if (COUNTLOCK_AVAILABLE_INTERNAL(tid))
			{
				// Lock
				COUNTLOCK_LOCK_INTERNAL(tid);

				// ======>> UNLOCK <<=======
				LOCK_UNLOCK_INTERNAL;

				return LOCK_RET::LOCK_SUCCEED;
			}

			// ======>> UNLOCK <<=======
			LOCK_UNLOCK_INTERNAL;
		}

		// Timeout
		if (!bInfiniteTimeout)
		{
			LOCK_CHECK_TIMEOUTSCG(ticker.Tick(), msTimeout);
		}

		Sleep(LOCK_SLEEPCYCLE);
	}

	return LOCK_RET::LOCK_SUCCEED;
}

int count_lock::UnLock()
{
	// ======>> LOCK <<=======
	LOCK_LOCKSCG_INTERNAL;

	// If not locked
	LOCK_CHECK_LOCKEDSCG(COUNTLOCK_EMPTY_INTERNAL);

	// Check thread ID
	LOCK_CHECK_MISMATCHSCG((tid_t)GET_TID != _tid);

	// Unlock
	COUNTLOCK_UNLOCK_INTERNAL;

	return LOCK_RET::LOCK_SUCCEED;
}

#pragma endregion count_lock

#pragma region single_lock

#ifdef SINGLE_LOCK

single_lock::single_lock() :
	base_lock(),
	_section(),
	_bLocked(false)
{
	InitializeCriticalSection(&_section);
}

single_lock::single_lock(single_lock&& lck) :
	base_lock(),
	_section(),
	_bLocked(false)
{
	// ======>> LOCK OTHER <<=======
	LOCK_LOCKOTHERSCG_INTERNAL(lck);
	// ======>> LOCK <<=======
	LOCK_LOCKSCG_INTERNAL;

	memcpy_s(&_section, sizeof(CRITICAL_SECTION), &lck._section, sizeof(CRITICAL_SECTION));
	_bLocked = lck._bLocked;

	// ======>> UNLOCK <<=======
	LOCK_UNLOCKSCG_INTERNAL;

	// Clear after move
	lck.single_lock::ClearMove();
}

single_lock::~single_lock()
{
	Release();
}

int single_lock::AssignMove(single_lock& lck)
{
	// ======>> LOCK OTHER <<=======
	LOCK_LOCKOTHERSCG_INTERNAL(lck);
	// ======>> LOCK <<=======
	LOCK_LOCKSCG_INTERNAL;

	// Clear before assignment
	single_lock::ClearPreAssign();

	// Assign
	base_lock::AssignMove(std::forward<base_lock>(lck));
	memcpy_s(&_section, sizeof(CRITICAL_SECTION), &lck._section, sizeof(CRITICAL_SECTION));
	_bLocked = lck._bLocked;

	// ======>> UNLOCK <<=======
	LOCK_UNLOCKSCG_INTERNAL;

	// Clear after move
	lck.single_lock::ClearMove();

	return LOCK_RET::LOCK_SUCCEED;
}

single_lock& single_lock::operator = (single_lock&& lck)
{
	AssignMove(std::forward<single_lock>(lck));

	return *this;
}

int single_lock::Lock(ms_t1 msTimeout)
{
	// Lock internal
	bool bLock = false;
	LOCK_LOCKTIMESCG_INTERNAL(msTimeout, bLock);
	RETVAL_ON_FALSE(
		bLock,
		LOCK_RET::LOCK_TIMEOUT);

	// Lock
	// Unused(msTimeout)
	EnterCriticalSection(&_section);
	_bLocked = true;

	return LOCK_RET::LOCK_SUCCEED;
}

int single_lock::UnLock()
{
	// ======>> LOCK <<=======
	LOCK_LOCKSCG_INTERNAL;

	// If not locked
	LOCK_CHECK_LOCKEDSCG(!_bLocked);

	// Unlock
	LeaveCriticalSection(&_section);
	_bLocked = false;

	return LOCK_RET::LOCK_SUCCEED;
}

#endif

#pragma endregion single_lock

#pragma region icrwb_lock

icrwb_lock::icrwb_lock() :
	base_lock(),
	_status(RWBLOCK_STATUS::RWB_UNLOCK),
	_nRes(RWBLOCK_DEFAULT_RES),
	_tid(INVALID_TID),
	_nLockR(0),
	_mapLockR(),
	_nLockIR(0),
	_mapLockIR()
{
}

icrwb_lock::icrwb_lock(icrwb_lock&& lock) :
	base_lock(),
	_status(RWBLOCK_STATUS::RWB_UNLOCK),
	_nRes(RWBLOCK_DEFAULT_RES),
	_tid(INVALID_TID),
	_nLockR(0),
	_mapLockR(),
	_nLockIR(0),
	_mapLockIR()
{
	// ======>> LOCK OTHER <<=======
	LOCK_LOCKOTHERSCG_INTERNAL(lock);
	// ======>> LOCK <<=======
	LOCK_LOCKSCG_INTERNAL;

	// Assign
	base_lock::AssignMove(lock);
	_nRes = lock._nRes;
	_tid = lock._tid;
	_nLockR = lock._nLockR;
	_mapLockR = std::forward<rmap>(lock._mapLockR);
	_nLockIR = lock._nLockIR;
	_mapLockIR = std::forward<rmap>(lock._mapLockIR);
	_status = lock._status;

	// ======>> UNLOCK <<=======
	LOCK_UNLOCKSCG_INTERNAL;

	// Clear after move
	lock.icrwb_lock::ClearMove();
}

icrwb_lock::~icrwb_lock()
{
	// ======>> LOCK DYING <<=======
	LOCK_LOCKDYING_INTERNAL;

	// Force unlock all
	_nLockR = 0;
	_mapLockR.clear();
	_nLockIR = 0;
	_mapLockIR.clear();
	_tid = INVALID_TID;

	_status = RWBLOCK_STATUS::RWB_UNLOCK;
}

int icrwb_lock::AssignMove(icrwb_lock& lck)
{
	// ======>> LOCK OTHER <<=======
	LOCK_LOCKOTHERSCG_INTERNAL(lck);
	// ======>> LOCK <<=======
	LOCK_LOCKSCG_INTERNAL;

	// Clear before assignment
	icrwb_lock::ClearPreAssign();

	// Assign
	base_lock::AssignMove(lck);
	_status = lck._status;
	_nRes = lck._nRes;
	_tid = lck._tid;
	_nLockR = lck._nLockR;
	_mapLockR = std::forward<rmap>(lck._mapLockR);
	_nLockIR = lck._nLockIR;
	_mapLockIR = std::forward<rmap>(lck._mapLockIR);

	// ======>> UNLOCK <<=======
	LOCK_UNLOCKSCG_INTERNAL;

	// Clear after move
	lck.icrwb_lock::ClearMove();

	return LOCK_RET::LOCK_SUCCEED;
}

icrwb_lock& icrwb_lock::operator = (icrwb_lock&& lock)
{
	AssignMove(lock);

	return *this;
}

int icrwb_lock::Lock(ms_t1 msTimeout)
{
	const sec_t sTimeout = MS_TO_SEC(msTimeout);
	bool bInfiniteTimeout = LOCK_TIME_ISINFINITE(msTimeout);

	Ticker ticker;
	ticker.Start();
	while (true)
	{
		// ======>> LOCK <<=======
		if (LOCK_TRYLOCK_INTERNAL)
		{
			if (!RWBLOCK_FULLLOCKEDBLOCK_INTERNAL)
			{
				// Lock
				RWBLOCK_LOCKBLOCK_INTERNAL;

				// ======>> UNLOCK <<=======
				LOCK_UNLOCK_INTERNAL;

				return LOCK_RET::LOCK_SUCCEED;
			}

			// ======>> UNLOCK <<=======
			LOCK_UNLOCK_INTERNAL;
		}

		// Timeout
		if (!bInfiniteTimeout)
		{
			LOCK_CHECK_TIMEOUTSCG(ticker.Tick(), sTimeout);
		}

		Sleep(LOCK_SLEEPCYCLE);
	}

	return LOCK_RET::LOCK_SUCCEED;
}

int icrwb_lock::LockIRead(ms_t1 msTimeout)
{
	const sec_t sTimeout = MS_TO_SEC(msTimeout);
	bool bInfiniteTimeout = LOCK_TIME_ISINFINITE(msTimeout);

	Ticker ticker;
	ticker.Start();
	while (true)
	{
		// ======>> LOCK <<=======
		if (LOCK_TRYLOCK_INTERNAL)
		{
			// Check read available
			if (!RWBLOCK_FULLLOCKEDIREAD_INTERNAL)
			{
				// Lock
				RWBLOCK_LOCKIREAD_INTERNAL;

				// ======>> UNLOCK <<=======
				LOCK_UNLOCK_INTERNAL;

				return LOCK_RET::LOCK_SUCCEED;
			}

			// ======>> UNLOCK <<=======
			LOCK_UNLOCK_INTERNAL;
		}

		// Timeout
		if (!bInfiniteTimeout)
		{
			LOCK_CHECK_TIMEOUTSCG(ticker.Tick(), sTimeout);
		}

		Sleep(LOCK_SLEEPCYCLE);
	}

	return LOCK_RET::LOCK_SUCCEED;
}

int icrwb_lock::LockRead(ms_t1 msTimeout)
{
	const sec_t sTimeout = MS_TO_SEC(msTimeout);
	bool bInfiniteTimeout = LOCK_TIME_ISINFINITE(msTimeout);

	Ticker ticker;
	ticker.Start();
	while (true)
	{
		// ======>> LOCK <<=======
		if (LOCK_TRYLOCK_INTERNAL)
		{
			// Check read available
			if (!RWBLOCK_FULLLOCKEDREAD_INTERNAL)
			{
				// Lock
				RWBLOCK_LOCKREAD_INTERNAL;

				// ======>> UNLOCK <<=======
				LOCK_UNLOCK_INTERNAL;

				return LOCK_RET::LOCK_SUCCEED;
			}

			// ======>> UNLOCK <<=======
			LOCK_UNLOCK_INTERNAL;
		}

		// Timeout
		if (!bInfiniteTimeout)
		{
			LOCK_CHECK_TIMEOUTSCG(ticker.Tick(), sTimeout);
		}

		Sleep(LOCK_SLEEPCYCLE);
	}

	return LOCK_RET::LOCK_SUCCEED;
}

int icrwb_lock::LockWrite(ms_t1 msTimeout)
{
	const sec_t sTimeout = MS_TO_SEC(msTimeout);
	bool bInfiniteTimeout = LOCK_TIME_ISINFINITE(msTimeout);

	Ticker ticker;
	ticker.Start();
	while (true)
	{
		// ======>> LOCK <<=======
		if (LOCK_TRYLOCK_INTERNAL)
		{
			// Check write available
			if (!RWBLOCK_FULLLOCKEDWRITE_INTERNAL)
			{
				// Lock
				RWBLOCK_LOCKWRITE_INTERNAL;

				// ======>> UNLOCK <<=======
				LOCK_UNLOCK_INTERNAL;

				return LOCK_RET::LOCK_SUCCEED;
			}

			// ======>> UNLOCK <<=======
			LOCK_UNLOCK_INTERNAL;
		}

		// Timeout
		if (!bInfiniteTimeout)
		{
			LOCK_CHECK_TIMEOUTSCG(ticker.Tick(), sTimeout);
		}

		Sleep(LOCK_SLEEPCYCLE);
	}

	return LOCK_RET::LOCK_SUCCEED;
}

int icrwb_lock::LockIReadGuard(
	scope_guard& scg, ms_t1 msTimeout)
{
	int ret = LockIRead(msTimeout);
	RET_ON_NP_AND(
		ret,
		Logger::Log0("Lock Inconsistent Read failed, ret:" + to_string(ret)););

	//Logger::Log0("Lock read");

	scg.SetAndEnable(
		[this]()
		{
			UnLockIRead();
		});

	return ret;
}

int icrwb_lock::LockReadGuard(
	scope_guard& scg, ms_t1 msTimeout)
{
	int ret = LockRead(msTimeout);
	RET_ON_NP_AND(
		ret,
		Logger::Log0("Lock Read failed, ret:" + to_string(ret)););

	//Logger::Log0("Lock read");

	scg.SetAndEnable(
		[this]()
		{
			UnLockRead();
		});

	return ret;
}

int icrwb_lock::LockWriteGuard(
	scope_guard& scg, ms_t1 msTimeout)
{
	int ret = LockWrite(msTimeout);
	RET_ON_NP_AND(
		ret,
		Logger::Log0("Lock Write failed, ret:" + to_string(ret)););

	//Logger::Log0("Lock write");

	scg.SetAndEnable(
		[this]()
		{
			UnLockWrite();
		});

	return ret;
}

int icrwb_lock::UnLock()
{
	// ======>> LOCK <<=======
	LOCK_LOCKSCG_INTERNAL;

	// Check locked
	LOCK_CHECK_LOCKEDSCG(RWBLOCK_NOTLOCKEDBLOCK_INTERNAL);

	// Check thread ID
	LOCK_CHECK_MISMATCHSCG((tid_t)GET_TID != _tid);

	// Unlock
	RWBLOCK_UNLOCKBLOCK_INTERNAL;

	return LOCK_RET::LOCK_SUCCEED;
}

int icrwb_lock::UnLockIRead()
{
	// ======>> LOCK <<=======
	LOCK_LOCKSCG_INTERNAL;

	// Check locked
	LOCK_CHECK_LOCKEDSCG(RWBLOCK_NOTLOCKEDIREAD_INTERNAL);

	tid_t tid = (tid_t)GET_TID;
	rmap::iterator itr = _mapLockIR.find(tid);
	// Check thread ID
	LOCK_CHECK_MISMATCH(_mapLockIR.end() == itr);
	LOCK_CHECK_MISMATCH(itr->second <= 0);

	// Unlock
	RWBLOCK_UNLOCKIREAD_INTERNAL(itr);

	return LOCK_RET::LOCK_SUCCEED;
}

int icrwb_lock::UnLockRead()
{
	// ======>> LOCK <<=======
	LOCK_LOCKSCG_INTERNAL;

	// Check locked
	LOCK_CHECK_LOCKEDSCG(RWBLOCK_NOTLOCKEDREAD_INTERNAL);

	tid_t tid = (tid_t)GET_TID;
	rmap::iterator itr = _mapLockR.find(tid);
	// Check thread ID
	LOCK_CHECK_MISMATCH(_mapLockR.end() == itr);
	LOCK_CHECK_MISMATCH(itr->second <= 0);

	// Unlock
	RWBLOCK_UNLOCKREAD_INTERNAL(itr);

	return LOCK_RET::LOCK_SUCCEED;
}

int icrwb_lock::UnLockWrite()
{
	// ======>> LOCK <<=======
	LOCK_LOCKSCG_INTERNAL;

	// Check locked
	LOCK_CHECK_LOCKEDSCG(RWBLOCK_NOTLOCKEDWRITE_INTERNAL);

	// Check thread ID
	LOCK_CHECK_MISMATCHSCG((tid_t)GET_TID != _tid);

	// Unlock
	RWBLOCK_UNLOCKWRITE_INTERNAL;

	return LOCK_RET::LOCK_SUCCEED;
}

#pragma endregion icrwb_lock

#pragma region atom

atom8<bool>::atom8() :
	_obj(nullptr)
{
	ATOM_INIT_INTERNAL(_obj, cas_t);
}

atom8<bool>::atom8(bool vT) :
	_obj(nullptr)
{
	ATOM_INIT_INTERNAL(_obj, cas_t);

	cas_t vO = cas_t();
	ATOM_SETVAL_INTERNAL(vO, vT, bool);

	ATOM_SET_INTERNAL(_obj, vO);
}

atom8<bool>::atom8(const atom8<bool>& atm) :
	_obj(nullptr)
{
	ATOM_INIT_INTERNAL(_obj, cas_t);

	// Get Other obj value
	cas_t vO;
	ATOM_GET_INTERNAL(atm._obj, vO);

	ATOM_SET_INTERNAL(_obj, vO);
}

atom8<bool>::~atom8()
{
	SAFE_DELETE(_obj);
}

int atom8<bool>::Assign(atom8<bool>& atm)
{
	// Get Other obj value
	cas_t vO;
	ATOM_GET_INTERNAL(atm._obj, vO);

	ATOM_SET_INTERNAL(_obj, vO);

	return 1;
}

atom8<bool>& atom8<bool>::operator = (atom8<bool>& atm)
{
	Assign(atm);

	return *this;
}

int atom8<bool>::Clear()
{
	ATOM_SET_INTERNAL(_obj, bool());

	return 1;
}

void atom8<bool>::flip()
{
	// Get obj value
	cas_t vO;
	ATOM_GET_INTERNAL(_obj, vO);

	// Get T value
	bool vT;
	ATOM_GETVAL_INTERNAL(vO, vT, bool);
	// Decrement
	bool vTU = vT;
	vTU = !vTU;

	// Get obj value of inced
	cas_t vOU = cas_t();
	ATOM_SETVAL_INTERNAL(vOU, vTU, bool);

	while (ATOM_CASF_INTERNAL(_obj, vO, vOU))
	{
		// Get obj value
		ATOM_GET_INTERNAL(_obj, vO);

		// Get T value
		ATOM_GETVAL_INTERNAL(vO, vT, bool);
		// Decrement
		vTU = vT;
		vTU = !vTU;

		// Get obj value of inced
		ATOM_SETVAL_INTERNAL(vOU, vTU, bool);
	}
}

bool atom8<bool>::get() const
{
	// Get obj value
	cas_t vO;
	ATOM_GET_INTERNAL(_obj, vO);

	// Get T value
	bool vT;
	ATOM_GETVAL_INTERNAL(vO, vT, bool);

	return vT;
}

bool atom8<bool>::set(bool vT)
{
	cas_t vO = cas_t();
	ATOM_SETVAL_INTERNAL(vO, vT, bool);

	// Set and get old value
	cas_t vOOld = ATOM_SET_INTERNAL(_obj, vO);

	// Get old T value
	bool vTOld;
	ATOM_GETVAL_INTERNAL(vOOld, vTOld, bool);

	return (vTOld != vT);
}

atom<bool>::atom() :
	_v(nullptr)
{
	_v = new cas_t(ATOM_BOOL_FALSE);
}

atom<bool>::atom(bool vT) :
	_v(nullptr)
{
	_v = new cas_t(ATOM_BOOL_FALSE);

	cas_t v = vT ? ATOM_BOOL_TRUE : ATOM_BOOL_FALSE;

	ATOM_SET_INTERNAL(_v, v);
}

atom<bool>::atom(const atom<bool>& atm) :
	_v(nullptr)
{
	_v = new cas_t(ATOM_BOOL_FALSE);

	cas_t v = *(atm._v);// pure atom read

	// Assign TODO
	ATOM_SET_INTERNAL(_v, v);
}

atom<bool>::~atom()
{
	SAFE_DELETE(_v);
}

int atom<bool>::Assign(const atom<bool>& atm)
{
	cas_t v = *(atm._v);// pure atom read

	// Assign TODO
	ATOM_SET_INTERNAL(_v, v);

	return 1;
}

atom<bool>& atom<bool>::operator = (const atom<bool>& atm)
{
	Assign(atm);

	return *this;
}

int atom<bool>::clear()
{
	ATOM_SET_INTERNAL(_v, ATOM_BOOL_FALSE);

	return 1;
}

void atom<bool>::flip()
{
	cas_t v = *_v;// pure atom read
	cas_t vNew = (v ^ ATOMBOOL_XOR_MASK);

	while (ATOM_CASF_INTERNAL(_v, v, vNew))
	{
		v = *_v;
		vNew = (v ^ ATOMBOOL_XOR_MASK);
	}
}

bool atom<bool>::get() const
{
	cas_t v = *_v;// pure atom read

	return v == ATOM_BOOL_TRUE;
}

bool atom<bool>::set(bool vT)
{
	cas_t v = vT ? ATOM_BOOL_TRUE : ATOM_BOOL_FALSE;
	cas_t vOld = *_v;// pure atom read
	ATOM_SET_INTERNAL(_v, v);

	return vOld == ATOM_BOOL_TRUE;
}

bool atom<bool>::checkandflip(bool chk)
{
	cas_t vChk = chk ? ATOM_BOOL_TRUE : ATOM_BOOL_FALSE;
	cas_t vNew = (vChk ^ ATOMBOOL_XOR_MASK);

	return ATOM_CASS_INTERNAL(_v, vChk, vNew);
}

#undef ATOM_INIT_INTERNAL

#undef ATOM_CAS_INTERNAL
#undef ATOM_CASF_INTERNAL
#undef ATOM_SET_INTERNAL
#undef ATOM_EQU_INTERNAL
#undef ATOM_GET_INTERNAL

#undef ATOM_TYPEAT_INTERNAL
#undef ATOM_SETVAL_INTERNAL
#undef ATOM_GETVAL_INTERNAL

#pragma endregion atom

#undef _IMGDISP_SOURCE_FILE_LOCK_CPP