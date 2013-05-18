// Source code for the implementation of AnyString in the
// slides with corrected move semantics thanks to guidance
// from Rob Stewart.

#include <string>

class AnyString {
    typedef std::string    string;
    typedef std::u16string u16string;
    typedef std::u32string u32string;
public:
    enum Contains { t_none, t_s08, t_s16, t_s32 };
private:
    Contains t_;

    union {                  // Anonymous union
        string    s08_;
        u16string s16_;
        u32string s32_;
    };

    // Private helpers.  Note placement new.
    void Assign()                              // Default
        {                                       t_ = t_none;}
    void Assign(const string& s08)             // Copy
        {new (&s08_)    string(s08);            t_ = t_s08; }
    void Assign(string&& s08)                  // Move
        {new (&s08_)    string(std::move(s08)); t_ = t_s08; }
    void Assign(const u16string& s16)          // Copy
        {new (&s16_) u16string(s16);            t_ = t_s16; }
    void Assign(u16string&& s16)               // Move
        {new (&s16_) u16string(std::move(s16)); t_ = t_s16; }
    void Assign(const u32string& s32)          // Copy
        {new (&s32_) u32string(s32);            t_ = t_s32; }
    void Assign(u32string&& s32)               // Move
        {new (&s32_) u32string(std::move(s32)); t_ = t_s32; }

    void Assign(const AnyString& anyStr)  // Copy assign
    {
        switch (anyStr.t_) {
        case t_none: Assign();            break;
        case t_s08:  Assign(anyStr.s08_); break;
        case t_s16:  Assign(anyStr.s16_); break;
        case t_s32:  Assign(anyStr.s32_); break;
        }
    }

    void Assign(AnyString&& anyStr)      // Move assign
    {
        Contains anyStr_t = anyStr.t_;
        anyStr.t_ = t_none;
        switch (anyStr_t) {
        case t_none: Assign();                     ; break;
        case t_s08:  Assign(std::move(anyStr.s08_)); break;
        case t_s16:  Assign(std::move(anyStr.s16_)); break;
        case t_s32:  Assign(std::move(anyStr.s32_)); break;
        }
    }

    // Assign from string literals uses move semantics

    void Assign(const     char* cptr)
        { Assign(   string(cptr)); }

    void Assign(const char16_t* cptr)
        { Assign(u16string(cptr)); }

    void Assign(const char32_t* cptr)
        { Assign(u32string(cptr)); }

    void Destroy() noexcept {
        switch (t_) {
        case t_none:                   break;
        case t_s08: s08_.~string();    break;
        case t_s16: s16_.~u16string(); break;
        case t_s32: s32_.~u32string(); break;
        }
        t_ = t_none;
    };

public:
    // Implicitly deleted constructors
    AnyString()
	    { Assign();                     }
    AnyString(const AnyString& str)
	    { Assign(str);                  }
    AnyString(AnyString&& str)
	    { Assign(std::move(str));       }

    // Perfect forwarding constructor
    template <typename T> AnyString(T&& str)              
        { Assign(std::forward<T>(str)); }

    // Implicitly deleted assignment from const reference
    AnyString& operator=(const AnyString& str)
    {
        if (this != &str) {  // Assignment to self?
            Destroy();       // Explicit destroy
            Assign(str);     // Placement new
        }
        return *this;
    } 

    // Perfect forwarding assignment
    template <typename T>
    AnyString& operator=(T&& str)
    {
        if (this != 
            reinterpret_cast<volatile const void*>(&str))
        {
            Destroy();
            Assign(std::forward<T>(str));
        }
        return *this;
    }

    ~AnyString() noexcept 
    {
        Destroy();
    }
};
