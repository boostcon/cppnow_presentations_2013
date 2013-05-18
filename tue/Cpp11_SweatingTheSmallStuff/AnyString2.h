// Source code for a more exception safe AnyString.
// Developed with guidance from Rob Stewart.

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

    // Private helper function does destructor's work
    void Destroy() noexcept
    {
        switch (t_) {
        case t_none:                   return;
        case t_s08: s08_.~string();    break;
        case t_s16: s16_.~u16string(); break;
        case t_s32: s32_.~u32string(); break;
        }
        t_ = t_none;
    }

public:
    // Implicitly deleted constructors
    AnyString() : t_{t_none}        {  }
    AnyString(const AnyString& str)
    {
        t_ = str.t_;
        switch (str.t_)
        {
        case t_none:                                 break;
        case t_s08: new (&s08_)    string(str.s08_); break;
        case t_s16: new (&s16_) u16string(str.s16_); break;
        case t_s32: new (&s32_) u32string(str.s32_); break;
        }
    }

    AnyString(AnyString&& str)
    {
        t_ = str.t_;
        switch (t_)
        {
        case t_none:                                            break;
        case t_s08: new (&s08_)    string(std::move(str.s08_)); break;
        case t_s16: new (&s16_) u16string(std::move(str.s16_)); break;
        case t_s32: new (&s32_) u32string(std::move(str.s32_)); break;
        }
        str.Destroy();
    }

    // Constructors from other string-like types
    AnyString(string s08)
    {
        new (&s08_) string(std::move(s08));
        t_ = t_s08;
    }

    AnyString(u16string s16)
    {
        new (&s16_) u16string(std::move(s16));
        t_ = t_s16;
    }

    AnyString(u32string s32)
    {
        new (&s32_) u32string(std::move(s32));
        t_ = t_s32;
    }

    // Implicitly deleted assignment from const reference
    AnyString& operator=(const AnyString& str)
    {
        if (this != &str) {  // Assignment to self?
            AnyString tmp(str);
            swap(tmp);
        }
        return *this;
    } 

    // Perfect forwarding assignment
    template <typename T>
    AnyString& operator=(T&& str)
    {
        if (this != reinterpret_cast<volatile const void*>(&str))
        {
            AnyString tmp(std::forward<T>(str));
            swap(tmp);
        }
        return *this;
    }

    ~AnyString() noexcept 
    {
        Destroy();
    }

    void swap(AnyString& rhs)
    {
        // Optimization if the contained strings are the same type
        if (t_ == rhs.t_) {
            switch (t_) {
            case t_none:                           break;
            case t_s08: std::swap(s08_, rhs.s08_); break;
            case t_s16: std::swap(s16_, rhs.s16_); break;
            case t_s32: std::swap(s32_, rhs.s32_); break;
            }
            return;
        }
        // General case.
        // Move rhs's contents to a local temporary.  
        AnyString tmp(std::move(rhs));
        // Move *this into rhs
        switch (t_) {
        case t_none:                                            break;
        case t_s08: new (&rhs.s08_)    string(std::move(s08_)); break;
        case t_s16: new (&rhs.s16_) u16string(std::move(s16_)); break;
        case t_s32: new (&rhs.s32_) u32string(std::move(s32_)); break;
        }
        rhs.t_ = t_;
        // Move tmp's contents into *this
        Destroy();
        switch (tmp.t_) {
        case t_none:                                            break;
        case t_s08: new (&s08_)    string(std::move(tmp.s08_)); break;
        case t_s16: new (&s16_) u16string(std::move(tmp.s16_)); break;
        case t_s32: new (&s32_) u32string(std::move(tmp.s32_)); break;
        }
        t_ = tmp.t_;
    }
};
