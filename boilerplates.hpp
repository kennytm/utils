#ifndef BOILERPLATES_HPP_KJR7FBERPMTGWRK9
#define BOILERPLATES_HPP_KJR7FBERPMTGWRK9 1

#include <ostream>
#include <boost/preprocessor.hpp>

#define IMPLEMENT_ENUM_OSTREAM_PRE_NIVF9GBVKILA0PB9(ENUM_NAME)                  \
    std::ostream& operator<<(std::ostream& stream, ENUM_NAME e)                 \
    {                                                                           \
        const char* name;                                                       \
        switch (e)                                                              \
        {                                                                       \
            default:                                                            \
                name = #ENUM_NAME "::<unknown>";                                \
                break;                                                          \

#define IMPLEMENT_ENUM_OSTREAM_CASE_HZM4576EWJ49GGB9(r, ENUM_NAME, ENUM_MEMBER) \
            case ENUM_NAME::ENUM_MEMBER:                                        \
                name = #ENUM_NAME "::" BOOST_PP_STRINGIZE(ENUM_MEMBER);         \
                break;                                                          \

#define IMPLEMENT_ENUM_OSTREAM_POST_WT03SAMDJQAJ1YVI                            \
       }                                                                        \
       return stream << name;                                                   \
   }

#define IMPLEMENT_ENUM_OSTREAM(ENUM_NAME, ENUM_MEMBERS_SEQ)                     \
        IMPLEMENT_ENUM_OSTREAM_PRE_NIVF9GBVKILA0PB9(ENUM_NAME)                  \
        BOOST_PP_SEQ_FOR_EACH(IMPLEMENT_ENUM_OSTREAM_CASE_HZM4576EWJ49GGB9,     \
                              ENUM_NAME,                                        \
                              ENUM_MEMBERS_SEQ)                                 \
        IMPLEMENT_ENUM_OSTREAM_POST_WT03SAMDJQAJ1YVI

#define IMPLEMENT_STRUCT_OSTREAM_PRE_NLS0KBFD97M(STRUCT_NAME)                   \
    std::ostream& operator<<(std::ostream& stream, STRUCT_NAME s)               \
    {                                                                           \
        return stream << '{'

#define IMPLEMENT_STRUCT_OSTREAM_MEMBER_0B23DUC83WP8(r, d, INDEX, STRUCT_MEMBER)\
        << BOOST_PP_EXPR_IF(INDEX, ", " <<) s.STRUCT_MEMBER

#define IMPLEMENT_STRUCT_OSTREAM_POST_L8YB90ELQ0M                               \
        << '}';                                                                 \
    }

#define IMPLEMENT_STRUCT_OSTREAM(STRUCT_NAME, STRUCT_MEMBERS_SEQ)               \
        IMPLEMENT_STRUCT_OSTREAM_PRE_NLS0KBFD97M(STRUCT_NAME)                   \
        BOOST_PP_SEQ_FOR_EACH_I(IMPLEMENT_STRUCT_OSTREAM_MEMBER_0B23DUC83WP8, , \
                                STRUCT_MEMBERS_SEQ)                             \
        IMPLEMENT_STRUCT_OSTREAM_POST_L8YB90ELQ0M

#define IMPLEMENT_ENUM_BIT_OPS_9MKW9IBN5Y(r, d, ENUM_NAME)                      \
    static inline ENUM_NAME operator|(ENUM_NAME a, ENUM_NAME b) noexcept        \
    { return static_cast<ENUM_NAME>(static_cast<unsigned>(a) | static_cast<unsigned>(b)); } \
    static inline ENUM_NAME operator&(ENUM_NAME a, ENUM_NAME b) noexcept        \
    { return static_cast<ENUM_NAME>(static_cast<unsigned>(a) & static_cast<unsigned>(b)); } \
    static inline ENUM_NAME operator^(ENUM_NAME a, ENUM_NAME b) noexcept        \
    { return static_cast<ENUM_NAME>(static_cast<unsigned>(a) ^ static_cast<unsigned>(b)); } \
    static inline ENUM_NAME operator~(ENUM_NAME a) noexcept                     \
    { return static_cast<ENUM_NAME>(~static_cast<unsigned>(a)); }

#define IMPLEMENT_ENUM_BITWISE_OPERATORS(ENUM_NAMES_SEQ)                        \
    BOOST_PP_SEQ_FOR_EACH(IMPLEMENT_ENUM_BIT_OPS_9MKW9IBN5Y, , ENUM_NAMES_SEQ)

#endif

