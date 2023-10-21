//
// Created by Administrator on 2022.11.30.
//

#ifndef WSAPATCH_ERRNORESTORER_H
#define WSAPATCH_ERRNORESTORER_H

#ifdef _WIN32

#include <windows.h>

namespace platform {

class ErrnoRestorer {
public:
    ErrnoRestorer() : saved_errno_(GetLastError()) {}

    ~ErrnoRestorer() {
        SetLastError(saved_errno_);
    }

    // disable copy and assign

    ErrnoRestorer(const ErrnoRestorer&) = delete;

    ErrnoRestorer& operator=(const ErrnoRestorer&) = delete;

    ErrnoRestorer(ErrnoRestorer&&) = delete;

    ErrnoRestorer& operator=(ErrnoRestorer&&) = delete;

    explicit operator int() const { return saved_errno_; }

    // Allow this object to be used as part of && operation.
    explicit operator bool() const { return true; }

private:
    const DWORD saved_errno_;
};

}

#else

#include <cerrno>

namespace platform {

class ErrnoRestorer {
public:
    ErrnoRestorer() : saved_errno_(errno) {}

    ~ErrnoRestorer() {
        errno = saved_errno_;
    }

    // disable copy and assign

    ErrnoRestorer(const ErrnoRestorer&) = delete;

    ErrnoRestorer& operator=(const ErrnoRestorer&) = delete;

    ErrnoRestorer(ErrnoRestorer&&) = delete;

    ErrnoRestorer& operator=(ErrnoRestorer&&) = delete;

    explicit operator int() const { return saved_errno_; }

    // Allow this object to be used as part of && operation.
    explicit operator bool() const { return true; }

private:
    const int saved_errno_;
};

}

#endif

#endif //WSAPATCH_ERRNORESTORER_H
