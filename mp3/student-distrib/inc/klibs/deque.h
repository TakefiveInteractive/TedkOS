#ifndef _KLIBS_DEQUE_H
#define _KLIBS_DEQUE_H

#include <stddef.h>
#include <stdint.h>

// No lock.
// This D.S. stores value as instance, not as reference
// (Making two copies and changing one of them will not affect the other)
template <typename T>
class Deque
{
private:

    // sInitialCapacity must be a NON-ZERO value
    static const size_t sInitialCapacity = 2;

    size_t mCapacity;
    size_t mSize;

    // mFront: position to be pop_front (front - 1 is to be pushed)
    // mBack:  position to be pop_back (back + 1 is to be pushed)
    // For mFront&mBack: -1 means no such push has happened.
    size_t mFront, mBack;

    // array of pointers to element.
    T** mContents;

    // delete every pointer INSIDE mContents array
    // it does not delete[] mContents
    void nullifyContent()
    {
        size_t item = mFront;
        for(size_t i = 0; i < mSize; i++)
        {
            delete mContents[item];
            mContents[item] = NULL;
            item = (item + 1) % mCapacity;
        }
    }

    // clear eveything
    void _clear()
    {
        nullifyContent();
        delete[] mContents;
        mContents = NULL;
    }

    // copy other to *this
    // it assumes that nothing is initialized.
    void _copy(const Deque<T>& other)
    {
        mSize = other.mSize;
        mCapacity = other.mCapacity;
        mFront = other.mFront;
        mBack  = other.mBack;

        mContents = new T*[mCapacity];
        size_t item = mFront;
        for(size_t i = 0; i < mSize; i++)
        {
            mContents[item] = new T(*other.mContents[item]);
            item = (item + 1) % mCapacity;
        }
    }

    // initialize to an empty deque. Assuming nothing is initialized.
    void _init()
    {
        mCapacity = sInitialCapacity;
        mSize = 0;
        mFront = mBack = -1;
        mContents = new T*[mCapacity];
        memset(mContents, 0, sizeof(T*) * mCapacity);
    }

    void resetCap(size_t newCap)
    {
        if(newCap <= mSize)
            return;

        T** newArr = new T*[newCap];
        size_t item = mFront;
        for(size_t i = 0; i < mSize; i++)
        {
            newArr[i] = mContents[item];
            item = (item + 1) % mCapacity;
        }

        mFront = 0;
        mBack = mSize - 1;
        mCapacity = newCap;

        delete[] mContents;
        mContents = newArr;
    }

public:
    Deque()
    {
        _init();
    }

    Deque(const Deque<T>& other)
    {
        _copy(other);
    }

    ~Deque()
    {
        _clear();
    }

    Deque& operator= (const Deque<T>& other)
    {
        _clear();
        _copy(other);
    }

    T& operator[] (size_t pos)
    {
        if(pos >= mSize)
        {
            printf("Exception: Deque Access Out of Bound!\n");
        }
        return *mContents[(mFront + pos) % mCapacity];
    }

    const T& operator[] (size_t pos) const
    {
        if(pos >= mSize)
        {
            printf("Exception: Deque Access Out of Bound!\n");
        }
        return *mContents[(mFront + pos) % mCapacity];
    }

    bool empty() const
    {
        return mSize == 0;
    }

    size_t size() const
    {
        return mSize;
    }

    // Current capacity (this value can grow).
    size_t max_size() const
    {
        return mCapacity;
    }

    void clear()
    {
        _clear();
        _init();
    }

    const T* front() const
    {
        if(mSize == 0)
            return NULL;
        return mContents[mFront];
    }

    T* front()
    {
        return const_cast<T*>(const_cast<const Deque*>(this)->front());
    }

    const T* back() const
    {
        if(mSize == 0)
            return NULL;
        return mContents[mBack];
    }

    T* back()
    {
        return const_cast<T*>(const_cast<const Deque*>(this)->back());
    }

    void push_back(const T& val)
    {
        if(mSize == 0)
        {
            mContents[0] = new T(val);
            mBack = mFront = 0;
            mSize ++;
            return;
        }
        if(mSize == mCapacity)
            resetCap(2 * mCapacity);

        mSize ++;

        size_t pos = (mBack + 1) % mCapacity;
        mContents[pos] = new T(val);
        mBack = pos;
    }

    void pop_back()
    {
        if(mSize == 0)
            return;

        delete mContents[mBack];
        mContents[mBack] = NULL;
        mBack = (mBack - 1 + mCapacity) % mCapacity;

        mSize --;
        if(mSize <= mCapacity/2)
            resetCap(mCapacity/2);
    }

    void push_front(const T& val)
    {
        if(mSize == 0)
        {
            mContents[0] = new T(val);
            mBack = mFront = 0;
            mSize ++;
            return;
        }
        if(mSize == mCapacity)
            resetCap(2 * mCapacity);

        mSize ++;

        size_t pos = (mFront - 1 + mCapacity) % mCapacity;
        mContents[pos] = new T(val);
        mFront = pos;
    }

    void pop_front()
    {
        if(mSize == 0)
            return;

        delete mContents[mFront];
        mContents[mFront] = NULL;
        mFront = (mFront + 1) % mCapacity;

        mSize --;
        if(mSize <= mCapacity/2)
            resetCap(mCapacity/2);
    }

};

#endif//_KLIBS_DEQUE_H
