#pragma once

// library header
#define Enum_EXP(...) __VA_ARGS__
#define Enum_MAX(a, b) ((a) > (b) ? (a) : (b))
#define Enum_LPAREN (
#define EnumMember(Name, _) Name _,
#define EnumString(Name, _) if (e == Name) return # Name;
#define EnumMaxBegin(Name, _) Enum_MAX Enum_LPAREN Name, 
#define EnumMaxEnd(Name, _) )
#define EnumQuantity(Name, _) +1
#define EnumEnumerator(Name, _) { Name, # Name },
#define EnumDef(EnumName, BaseType) \
    namespace EnumName ## _ { \
        enum EnumName BaseType { \
            EnumName ## _values(EnumMember) \
            FinalEnumeratorPlusOne, \
            /* this next one is evil in real code because it's O(n^2), making it basically useless. write a constexpr function! */ \
            /*Highest = Enum_EXP(EnumName ## _values(EnumMaxBegin) (FinalEnumeratorPlusOne - 1) EnumName ## _values(EnumMaxEnd)),*/ \
            Quantity = (EnumName ## _values(EnumQuantity)) \
        }; \
    }; \
    typedef EnumName ## _ :: EnumName EnumName; \
    struct EnumName ## _Enumerator { EnumName value; const char *string; }; \
    namespace EnumName ## _ { \
        const EnumName ## _Enumerator enumerators[EnumName::Quantity] = { EnumName ## _values(EnumEnumerator) }; \
    } \
    const char *EnumName ## _to_string(EnumName e) { \
        using namespace EnumName ## _; \
        EnumName ## _values(EnumString) \
        return ""; \
    } \
    const EnumName ## _Enumerator (&EnumName ## _enumerators)[EnumName::Quantity] = EnumName ## _::enumerators; \
    EnumName operator|(EnumName lhs, EnumName rhs) { return (EnumName)((u64)lhs | (u64)rhs); } \
    EnumName &operator|=(EnumName &lhs, EnumName rhs) { lhs = lhs | rhs; return lhs; } \

