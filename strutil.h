#include <string>
#include <vector>
#include <sstream>  // TODO: remove.

namespace strutil {

template<bool cond, class T = void>
using enable_if_t = typename std::enable_if<cond, T>::type;

template<bool cond>
using enable_when = enable_if_t<cond, std::nullptr_t>;


class fail : public std::runtime_error {
public:
    template<class...A>
    fail(std::string s) :std::runtime_error(s) {};
};


inline
std::string lower(const std::string& s) {
    std::string r(0, ' ');
    r.reserve(s.size());
    for (const auto& c: s) {
        r.push_back(('A' <= c && c <= 'Z') ? c - ('Z'-'z') : c);
    }
    return r;
}

inline
std::string upper(const std::string& s) {
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
std::string reversed(const std::string& s) {
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
void reverse(std::string& s) {
    size_t len = s.size();
    for (size_t i = 0; i < len / 2; ++i) {
        char tmp = s[i];
        s[i] = s[len - 1 - i];
        s[len - 1 - i] = tmp;
    }
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

namespace _detail {

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

inline
char itoc(int i, bool upper=false) {
    return i > 9 ? (upper ? 'A' : 'a') + i - 10 : '0' + i;
}

inline
void extend(std::string& s, int count, char c) {
    s.reserve(s.size() + count);
    for (int i = 0; i < count; ++i) s.push_back(c);
}

inline
std::string intformatter(int64_t i, int radix=10, bool upper=false,
                         char sign=0, char padchar=' ', int width=0, bool alignleft=false) {
    if (i < 0) sign = '-';
    if (padchar == '0' && alignleft) padchar = '\0';  // ignore zero flag.
    if (sign && width > 0) width--;
    std::string buf;
    if (width > 0) buf.reserve(width);
    int64_t r = i < 0 ? -i : i;
    while (r > 0) {
        buf.push_back(itoc(r % radix, upper));
        r /= radix;
    }
    if (i == 0) buf = "0";
    int padding = width - buf.size() - (sign ? 1 : 0);
    if (padding > 0 && padchar == '0') {
        extend(buf, padding, '0');
        padding = 0;
    }
    if (sign) {
        buf.push_back(sign);
    }
    if (padding > 0) {
        if (alignleft) {
            reverse(buf);
            extend(buf, padding, padchar);
        } else {
            extend(buf, padding, padchar);
            reverse(buf);
        }
        return buf;
    }
    reverse(buf);
    return buf;
}

inline
std::string uintformatter(uint64_t i, int radix=10, bool upper=false,
                          char sign=0, char padchar=' ', int width=0, bool alignleft=false) {
    if (padchar == '0' && alignleft) padchar = '\0';  // ignore zero flag.
    if (sign && width > 0) width--;
    std::string buf;
    if (width > 0) buf.reserve(width);
    uint64_t r = i;
    while (r > 0) {
        buf.push_back(itoc(r % radix, upper));
        r /= radix;
    }
    if (i == 0) buf = "0";
    int padding = width - buf.size() - (sign ? 1 : 0);
    if (padding > 0 && padchar == '0') {
        extend(buf, padding, '0');
        padding = 0;
    }
    if (sign) {
        buf.push_back(sign);
    }
    if (padding > 0) {
        if (alignleft) {
            reverse(buf);
            extend(buf, padding, padchar);
        } else {
            extend(buf, padding, padchar);
            reverse(buf);
        }
        return buf;
    }
    reverse(buf);
    return buf;
}

inline
std::string floatformatter(double d, int precision=6,
                           char sign='\0', char padchar='\0', int width=0) {
    std::stringstream ss;
    ss << d;
    return ss.str();
}

inline bool isdigit(const char c) { return '0' <= c && c <= '9'; }

template<size_t I = 0, class F, class...T, enable_when<I == sizeof...(T)> = nullptr>
std::string call_for_index(int, std::tuple<T...>&, F) {
    return "";
}

template<size_t I = 0, class F, class...T, enable_when<I < sizeof...(T)> = nullptr>
std::string call_for_index(int index, std::tuple<T...>& t, F f) {
    return index == 0 ? f(std::get<I>(t)) : call_for_index<I + 1, F, T...>(index-1, t, f);
}

}  // namespace _detail

struct formaterror : public fail {
    formaterror(const char* s) :fail(s) {}
};

struct formatter_ {
    std::string str;
    char conv = '\0';
    bool sharp = false;
    char sign = 0;
    char padchar = ' ';
    bool alignleft = false;
    int width = 0;
    int precision = 6;

    inline
    formatter_(const std::string& s, size_t start=0) {
        size_t i = start;
        if (s[i] != '%')
            throw formaterror("invalid format (first char)");
        i++;
        for (; i < s.size(); ++i) {
            switch (s[i]) {
                case '-': { alignleft = true; continue; }
                case '+': { sign = '+'; continue; }
                case ' ': { sign = ' '; continue; }
                case '#': { sharp = true; continue; }
                case '0': { padchar = '0'; continue; }
            }
            break;
        }
        if (_detail::isdigit(s[i])) { width = s[i] - '0'; i++; }
        if (_detail::isdigit(s[i])) { width = width * 10 + s[i] - '0'; i++; }
        if (s[i] == '.') {
            i++;
            if (_detail::isdigit(s[i])) { precision = s[i] - '0'; i++; }
            if (_detail::isdigit(s[i])) { precision = precision * 10 + s[i] - '0'; i++; }
        }
        if (_detail::isdigit(s[i]))
            throw formaterror("invalid format (width or precision too long)");
        conv = s[i++];
        str = s.substr(start, i - start);
    }

    template<class T, enable_when<std::is_integral<T>::value> = nullptr>
    std::string operator()(T t) {
        switch (conv) {
            case 'c': {
                if (sizeof(T) > sizeof(uint32_t))
                    throw formaterror("type mismatch: larger than uint32_t");
                return pad(_detail::u8char(t));
            }
            case 'd': case 'i': { return _detail::intformatter(t, 10, false, sign, padchar, width, alignleft); }
            case 'u': { return _detail::uintformatter(t, 10, false, sign, padchar, width, alignleft); }
            case 'o': { return _detail::uintformatter(t, 8, false, sign, padchar, width, alignleft); }
            case 'x': case 'X': { return _detail::uintformatter(t, 16, conv == 'X', sign, padchar, width, alignleft); }
            case 'b': { return _detail::uintformatter(t, 2, false, sign, padchar, width, alignleft); }
            case 'f': { throw formaterror("type mismatch: int to float"); }
            case 's': { return _detail::intformatter(t, 10, false, sign, padchar, width, alignleft); }
            case 'p': { throw formaterror("type mismatch: int to ptr"); }
        }
        throw formaterror("unknown format");
    }

    template<class T, enable_when<std::is_floating_point<T>::value> = nullptr>
    std::string operator()(T t) {
        switch (conv) {
            case 'c': { throw formaterror("type mismatch: float to char"); }
            case 'd': case 'i':
            case 'u':
            case 'o':
            case 'x': case 'X':
            case 'b': { throw formaterror("type mismatch: float to int"); }
            case 'f': { return _detail::floatformatter(t, precision); }
            case 's': { return _detail::floatformatter(t, precision); }
            case 'p': { throw formaterror("type mismatch: float to ptr"); }
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
            case 'p': { throw formaterror("type mismatch: str to pointer"); }
        }
        throw formaterror("unknown format");
    }

    inline
    std::string operator()(char* t) {
        if (conv == 'p') return ptos(t);
        return operator()(std::string(t));
    }

    inline
    std::string operator()(const char* t) {
        if (conv == 'p') return ptos(t);
        return operator()(std::string(t));
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
        if (alignleft) {
            return padright(s, w, ' ');
        } else {
            return padleft(s, w, padchar);
        }
    }

    template<class T>
    std::string ptos(T* ptr) {
        auto i = reinterpret_cast<uintptr_t>(ptr);
        return "0x" + _detail::uintformatter(static_cast<uint64_t>(i), 16);
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
            buf.append(_detail::call_for_index(argc-1, args, f));
        }
    }
    if (argc != sizeof...(A))
        throw formaterror("too many args");
    return buf;
}


}
