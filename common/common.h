//
//  common.h
//  devbase
//
//  Created by yajun18 on 2022/4/13.
//  Copyright © 2022 shuyj. All rights reserved.
//

#ifndef common_h
#define common_h

#if defined(DEVBASE_EXPORTS)
#  define DEVBASE_EXPORTS __declspec(dllexport)
#else
#  define DEVBASE_EXPORTS __declspec(dllimport)
#endif

#define MMIN(a,b)            (((a) < (b)) ? (a) : (b))


#define CLASS_CTX_NAME(n) n##Ctx

#define CLASS_CTX_DEF(n) \
    class CLASS_CTX_NAME(n); \
    std::unique_ptr<CLASS_CTX_NAME(n)> mCtx

#define SELF    mCtx

#define CLASS_CTX_CREATE(n) std::make_unique<CLASS_CTX_NAME(n)>()

/**
 USAGE:
    .h
    class A{
        CLASS_CTX_DEF(A);
    };
    .cpp
    class A::CLASS_CTX_NAME(A) {
        members;
    }
    A::A(){
        SELF = CLASS_CTX_CREATE(A);
        SELF->members;
    }
 */

#endif /* common_h */
