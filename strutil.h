#include <string>
#include <vector>

namespace strutil {

class fail : public std::runtime_error {
public:
    template<class...A>
    fail(std::string s) :std::runtime_error(s) {};
};


inline
std::string lower(const std::string& s){
    std::string r(0, ' ');
    r.reserve(s.size());
    for (const auto& c: s) {
        r.push_back(('A' <= c && c <= 'Z') ? c - ('Z'-'z') : c);
    }
    return r;
}

inline
std::string upper(const std::string& s){
    std::string r(0, ' ');
    r.reserve(s.size());
    for (const auto& c: s) {
        r.push_back(('a' <= c && c <= 'z') ? c + ('Z'-'z') : c);
    }
    return r;
}

inline
std::string rtrim(const std::string& s, const std::string& chars=" \t\r\n") {
    for (size_t i = s.size() - 1;; --i) {
        if (chars.find(s[i]) == std::string::npos) {
            return s.substr(0, i + 1);
        }
        if (i == 0) return "";
    }
}

inline
std::string ltrim(const std::string& s, const std::string& chars=" \t\r\n") {
    size_t len = s.size();
    for (size_t i = 0; i < len; ++i) {
        if (chars.find(s[i]) == std::string::npos) {
            return s.substr(i);
        }
    }
    return "";
}

inline
std::string trim(std::string s, const std::string& chars=" \t\r\n") {
    return ltrim(rtrim(s, chars));
}

inline
std::string padright(std::string s, size_t width, const char c=' ') {
    if (s.size() >= width) return s;
    return s + std::string(width - s.size(), c);
}

inline
std::string padleft(const std::string& s, size_t width, const char c=' ') {
    if (s.size() >= width) return s;
    return std::string(width - s.size(), c) + s;
}

inline
std::string reverse(const std::string& s) {
    size_t len = s.size();
    std::string r(0, ' ');
    r.reserve(len);
    for (size_t i = len - 1;; --i) {
        r.push_back(s[i]);
        if (i == 0) break;
    }
    return r;
}

inline
std::vector<std::string> split(const std::string& s, const char delim)
{
    std::vector<std::string> v;
    std::string buf = "";
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == delim) {
            v.push_back(buf);
            buf.erase();
        } else {
            buf.push_back(s[i]);
        }
    }
    v.push_back(buf);
    return v;
}

inline
std::string u8char(uint32_t uc) {
    if ((uc & ~0x007F) == 0) {
        // U+0000～U+007F
        std::string r(1, '\0');
        r[0] = static_cast<uint8_t>(uc);
        return r;
    } else if ((uc & ~0x07FF) == 0) {
        // U+0080～U+07FF
        std::string r(2, '\0');
        r[0] = static_cast<uint8_t>(0xC0 | (uc >> 6));
        r[1] = static_cast<uint8_t>(0x80 | (uc & 0x3F));
        return r;
    } else if ((uc & ~0xFFFF) == 0) {
        // U+0800～U+FFFF
        std::string r(3, '\0');
        r[0] = static_cast<uint8_t>(0xE0 | (uc >> 12));
        r[1] = static_cast<uint8_t>(0x80 | ((uc >> 6) & 0x3F));
        r[2] = static_cast<uint8_t>(0x80 | (uc & 0x3F));
        return r;
    } else if ((uc & 0x1F0000) != 0) {
        // U+10000～U+10FFFF
        std::string r(4, '\0');
        r[0] = static_cast<uint8_t>(0xF0 | (uc >> 18));
        r[1] = static_cast<uint8_t>(0x80 | ((uc >> 12) & 0x3F));
        r[2] = static_cast<uint8_t>(0x80 | ((uc >> 6) & 0x3F));
        r[3] = static_cast<uint8_t>(0x80 | (uc & 0x3F));
        return r;
    }
    throw fail("unknown unicode");
}

inline bool isdigit_(const char c) { return '0' <= c && c <= '9'; }

struct formaterror : public fail {
    formaterror(const char* s) :fail(s) {}
};

extern void* enabler;
#define ENABLE_WHEN(cond) typename std::enable_if<cond, void>::type*& = enabler

template<size_t I = 0, class F, class...T, ENABLE_WHEN(I == sizeof...(T))>
std::string for_index(int, std::tuple<T...>&, F) {
    return "";
}

template<size_t I = 0, class F, class...T, ENABLE_WHEN(I < sizeof...(T))>
std::string for_index(int index, std::tuple<T...>& t, F f) {
    return index == 0 ? f(std::get<I>(t)) : for_index<I + 1, F, T...>(index-1, t, f);
}

struct formatter_ {
    std::string str;
    char conv = '\0';
    bool minus = false;
    bool plus = false;
    bool space = false;
    bool sharp = false;
    bool zero = false;
    bool dot = false;
    int width = -1;
    int width2 = -1;

    formatter_(const std::string& s, size_t start=0) {
        size_t i = start;
        if (s[i] != '%')
            throw formaterror("invalid format (first char)");
        i++;
        for (; i < s.size(); ++i) {
            switch (s[i]) {
                case '-': { minus = true; continue; }
                case '+': { plus = true; continue; }
                case ' ': { space = true; continue; }
                case '#': { sharp = true; continue; }
                case '0': { zero = true; continue; }
            }
            break;
        }
        if (isdigit_(s[i])) { width = s[i] - '0'; i++; }
        if (isdigit_(s[i])) { width = width * 10 + s[i] - '0'; i++; }
        if (s[i] == '.') {
            dot = true;
            i++;
            if (isdigit_(s[i])) { width2 = s[i] - '0'; i++; }
            if (isdigit_(s[i])) { width2 = width2 * 10 + s[i] - '0'; i++; }
        }
        if (isdigit_(s[i]))
            throw formaterror("invalid format (width or precision too long)");
        conv = s[i++];
        str = s.substr(start, i - start);
    }

    template<class T, ENABLE_WHEN(std::is_integral<T>::value)>
    std::string operator()(T t) {
        switch (conv) {
            case 'c': {
                if (sizeof(T) > sizeof(uint32_t))
                    throw formaterror("type mismatch: larger than uint32_t");
                return pad(u8char(t));
            }
            case 'd': case 'i': { return itos(t); }
            case 'u': { return utos(t); }
            case 'o': { return utos(t, 8); }
            case 'x': case 'X': { return itos(t, 16, conv == 'X'); }
            case 'b': { return itos(t, 2); }
            case 'f': { throw formaterror("type mismatch: int to float"); }
            case 's': { return itos(t); }
            case 'p': { throw formaterror("type mismatch: int to ptr"); }
        }
        throw formaterror("unknown format");
    }

    template<class T, ENABLE_WHEN(std::is_floating_point<T>::value)>
    std::string operator()(T t) {
        switch (conv) {
            case 'c': { throw formaterror("type mismatch: float to char"); }
            case 'd': case 'i':
            case 'u':
            case 'o':
            case 'x': case 'X':
            case 'b': { throw formaterror("type mismatch: float to int"); }
            case 'f': { return ftos(t); }
            case 's': { return ftos(t); }
            case 'p': { return ptos((void*)(uint64_t)t); }
        }
        throw formaterror("unknown format");
    }

    inline
    std::string operator()(const std::string& t) {
        switch (conv) {
            case 'c': { throw formaterror("type mismatch: str to char"); }
            case 'd': case 'i':
            case 'u':
            case 'o':
            case 'x': case 'X':
            case 'b': { throw formaterror("type mismatch: str to int"); }
            case 'f': { throw formaterror("type mismatch: str to float"); }
            case 's': { return pad(t); }
            case 'p': { return ptos((void*)t.c_str()); }
        }
        throw formaterror("unknown format");
    }

    inline
    std::string operator()(char* t) {
        if (conv == 'p' || conv == 's') return ptos(t);
        return std::string(t);
    }

    inline
    std::string operator()(void* ptr) {
        if (conv == 'p') return ptos(ptr);
        throw formaterror("unknown format");
    }

    inline
    std::string pad(const std::string& s, int w=-1) {
        if (w == -1) w = width;
        if (s.size() > 99) return s;
        if (w <= (int)s.size()) return s;
        if (minus) {
            return padright(s, w, ' ');
        } else {
            return padleft(s, w, zero ? '0' : ' ');
        }
    }

    inline
    std::string itos(int64_t i, int radix=10, bool upper=false) {
        std::string buf;
        int64_t n = i < 0 ? -i : i;
        while (n > 0) {
            int d = n % radix;
            if (d > 9) {
                buf.push_back((upper ? 'A' : 'a') + d - 10);
            } else {
                buf.push_back('0' + d);
            }
            n /= radix;
        }
        if (buf.size() == 0) buf = "0";
        char pre = i < 0 ? '-' :
                   plus ? '+' :
                   space ? ' ' : 0;
        buf = reverse(buf);
        buf = pad(buf, width - (pre != 0 ? 1 : 0));
        if (pre != 0) { buf.insert(0, 1, pre); }
        return buf;
    }

    inline
    std::string utos(uint64_t u, int radix=10, bool upper=false) {
        std::string buf;
        uint64_t n = u;
        while (n > 0) {
            int d = n % radix;
            if (d > 9) {
                buf.push_back((upper ? 'A' : 'a') + d - 10);
            } else {
                buf.push_back('0' + d);
            }
            n /= radix;
        }
        if (buf.size() == 0) buf = "0";
        return pad(reverse(buf));
    }

    template<class T>
    std::string ptos(T* ptr) {
        width = -1;
        zero = true;
        auto i = reinterpret_cast<uintptr_t>(ptr);
        return "0x" + utos(static_cast<uint64_t>(i), 16);
    }

    inline
    std::string ftos(double d) {
        // TODO: rafactoring
        auto tmp_width = width;
        auto tmp_minus = minus;
        auto tmp_space = space;
        auto tmp_zero = zero;

        if (minus) {
            width = -1;
            minus = false;
        } else {
            width = width - (width2 >= 0 ? width2 : 6) - 1;
        }
        auto r1 = itos(static_cast<int64_t>(d));

        width = width2 >= 0 ? width2 : 6;
        zero = true;
        auto d2 = d - static_cast<int64_t>(d);
        d2 = d2 < 0 ? -d2 : d2;
        for (auto i = 0; i < width; ++i) d2 *= 10.0;
        auto r2 = utos(static_cast<uint64_t>(d2));

        width = tmp_width;
        minus = tmp_minus;
        space = tmp_space;
        zero = tmp_zero;
        auto r = r1 + "." + r2;
        return pad(r);
    }
};

template<class...A>
std::string format(const std::string& fmt, A...a) {
    auto args = std::make_tuple(a...);
    size_t len = fmt.size();
    std::string buf = "";
    int argc = 0;
    for (size_t i = 0; i < len; ++i) {
        const auto& c = fmt[i];
        if (c != '%') {
            buf.push_back(c);
        } else if (i >= len - 1) {
            buf.push_back(c);
        } else if (fmt[i+1] == '%') {
            buf.push_back(c);
            i++;
        } else {
            if (++argc > sizeof...(A))
                throw fail("formaterror(): no arg");
            auto f = formatter_(fmt, i);
            i += f.str.size() - 1;
            buf.append(for_index(argc-1, args, f));
        }
    }
    if (argc != sizeof...(A))
        throw formaterror("too many args");
    return buf;
}


}
