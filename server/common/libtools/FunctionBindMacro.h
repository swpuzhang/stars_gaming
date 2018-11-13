#ifndef FUNCTION_BIND_MACRO_H
#define FUNCTION_BIND_MACRO_H
#include <boost/bind.hpp>
#include <memory>
#include <functional>
#include "AsioTypes.h"


#define MEMFUN_BIND(fun) std::bind(&SELF_TYPE::fun, this->shared_from_this())
#define MEMFUN_BIND1(fun, arg1) std::bind(&SELF_TYPE::fun, this->shared_from_this(), arg1)
#define MEMFUN_BIND2(fun, arg1, arg2) std::bind(&SELF_TYPE::fun, this->shared_from_this(), arg1, arg2)
#define MEMFUN_BIND3(fun, arg1, arg2, arg3) std::bind(&SELF_TYPE::fun, this->shared_from_this(), arg1, arg2, arg3)
#define MEMFUN_BIND4(fun, arg1, arg2, arg3, arg4) std::bind(&SELF_TYPE::fun, this->shared_from_this(), arg1, arg2, arg3, arg4)

#define MEMFUN_HOLDER_BIND1(fun) std::bind(&SELF_TYPE::fun, this->shared_from_this(), PLACEHOLDER::_1)
#define MEMFUN_HOLDER_BIND2(fun) std::bind(&SELF_TYPE::fun, this->shared_from_this(), PLACEHOLDER::_1, PLACEHOLDER::_2)
#define MEMFUN_HOLDER_BIND3(fun) std::bind(&SELF_TYPE::fun, this->shared_from_this(), PLACEHOLDER::_1, PLACEHOLDER::_2, PLACEHOLDER::_3)

#define MEMFUN_THIS_BIND(fun) std::bind(&SELF_TYPE::fun, this)
#define MEMFUN_THIS_BIND1(fun, arg1) std::bind(&SELF_TYPE::fun, this, arg1)
#define MEMFUN_THIS_BIND2(fun, arg1, arg2) std::bind(&SELF_TYPE::fun, this, arg1, arg2)
#define MEMFUN_THIS_BIND3(fun, arg1, arg2, arg3) std::bind(&SELF_TYPE::fun, this, arg1, arg2, arg3)

#define MEMFUN_THIS_HOLDER_BIND1(fun) std::bind(&SELF_TYPE::fun, this, PLACEHOLDER::_1)
#define MEMFUN_THIS_HOLDER_BIND2(fun) std::bind(&SELF_TYPE::fun, this, PLACEHOLDER::_1, PLACEHOLDER::_2)
#define MEMFUN_THIS_HOLDER_BIND3(fun) std::bind(&SELF_TYPE::fun, this, PLACEHOLDER::_1, PLACEHOLDER::_2, PLACEHOLDER::_3)


#define MEMFUN_CAST_BIND(fun) std::bind(&SELF_TYPE::fun, std::dynamic_pointer_cast<SELF_TYPE>(this->shared_from_this()))
#define MEMFUN_CAST_BIND1(fun, arg1) std::bind(&SELF_TYPE::fun, std::dynamic_pointer_cast<SELF_TYPE>(this->shared_from_this()), arg1)
#define MEMFUN_CAST_BIND2(fun, arg1, arg2) std::bind(&SELF_TYPE::fun, std::dynamic_pointer_cast<SELF_TYPE>(this->shared_from_this()), arg1, arg2)
#define MEMFUN_CAST_BIND3(fun, arg1, arg2, arg3) std::bind(&SELF_TYPE::fun, std::dynamic_pointer_cast<SELF_TYPE>(this->shared_from_this()), arg1, arg2, arg3)
#define MEMFUN_CAST_BIND4(fun, arg1, arg2, arg3, arg4) std::bind(&SELF_TYPE::fun, std::dynamic_pointer_cast<SELF_TYPE>(this->shared_from_this()), arg1, arg2, arg3, arg4)

#define MEMFUN_CAST_HOLDER_BIND1(fun) std::bind(&SELF_TYPE::fun, std::dynamic_pointer_cast<SELF_TYPE>(this->shared_from_this()), PLACEHOLDER::_1)
#define MEMFUN_CAST_HOLDER_BIND2(fun) std::bind(&SELF_TYPE::fun, std::dynamic_pointer_cast<SELF_TYPE>(this->shared_from_this()), PLACEHOLDER::_1, PLACEHOLDER::_2)
#define MEMFUN_CAST_HOLDER_BIND3(fun) std::bind(&SELF_TYPE::fun, std::dynamic_pointer_cast<SELF_TYPE>(this->shared_from_this()), PLACEHOLDER::_1, PLACEHOLDER::_2, PLACEHOLDER::_3)
#endif