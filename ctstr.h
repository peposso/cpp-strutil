#pragma once
// #include <iostream>

namespace ctstr {

template<bool B, class T = void>
using enable_if_t = typename std::enable_if<B, T>::type;

template<int...I>
struct index_tuple { constexpr index_tuple() {} };

namespace _detail {

// index_range_next
template<class IndexTuple, int Next>
struct index_range_next;
template<int...I, int Next>
struct index_range_next<index_tuple<I...>, Next> {
    using type = index_tuple<I..., (I + Next)...>;
};

// index_range_next2
template<class IndexTuple, int Next, int Tail>
struct index_range_next2;
template<int...I, int Next, int Tail>
struct index_range_next2<index_tuple<I...>, Next, Tail> {
    using type = index_tuple<I..., (I + Next)..., Tail>;
};

// index_range_impl
template<int First, int Step, int N, class Enable = void>
struct index_range_impl;

template<int First, int Step, int N>
struct index_range_impl<First, Step, N, enable_if_t<(N == 0)>> {
    using type = index_tuple<>;
};

template<int First, int Step, int N>
struct index_range_impl<First, Step, N, enable_if_t<(N == 1)>> {
    using type = index_tuple<First>;
};

template<int First, int Step, int N>
struct index_range_impl<First, Step, N, enable_if_t<(N > 1 && N % 2 == 0)>>
: public index_range_next<typename index_range_impl<First, Step, N / 2>::type, 
                          First + N / 2 * Step>
{};

template<int First, int Step, int N>
struct index_range_impl<First, Step, N, enable_if_t<(N > 1 && N % 2 == 1)>>
: public index_range_next2<typename index_range_impl<First, Step, N / 2>::type,
                           First + N / 2 * Step, First + (N - 1) * Step>
{};

}   // namespace _detail

template<int First, int Last, int Step = 1>
struct index_range
: public _detail::index_range_impl<First, Step,
                                 ((Last - First) + (Step - 1)) / Step>::type
{};


// ----------------------------------------------------------------------------

template<int N>
struct ctstr {
    // ctor
    template<int M>
    constexpr ctstr(const char (&v)[M])
    : ctstr(v, index_range<0, N>(), 0, N)
    {}
    template<int M>
    constexpr ctstr(const char (&v)[M], int first, int last)
    : ctstr(v, index_range<0, N>(), first, last)
    {}
    template<int N1, int N2>
    constexpr ctstr(const char (&v1)[N1], const char (&v2)[N2], int size = -1)
    : ctstr(v1, v2, index_range<0, N>(), size != -1 ? size : N1)
    {}
    constexpr ctstr(const char c, int csize = N)
    : ctstr(c, index_range<0, N>(), csize)
    {}

    //
    template<class Stream>
    friend auto operator<<(Stream& os, const ctstr<N>& ctstr)
    -> decltype(os.write("", (size_t)0))
    {
        os.write(ctstr.data, ctstr.len());
        return os;
    }

    //
    struct chartest {
        const char c;
        constexpr chartest(const char c_) : c(c_) {}
        constexpr bool operator()(const int pos, const ctstr<N>& str) const {
            return str[pos] == c;
        }
    };

    template<class Pred>
    struct finder {
        const int found;
        const bool eos;
        const Pred& pred;
        constexpr finder(const Pred& pred_, const int found_=-1, const bool eos_=false)
        : pred(pred_), found(found_), eos(eos_)
        {}
        constexpr finder<Pred> operator()(const int pos, const ctstr<N>& str) const {
            return finder<Pred>(pred, found != -1 ? found : pred(pos, str) ? pos : -1,
                                 eos || str[pos] == '\0');
        }
        constexpr bool breaks() const { return eos || found != -1; }
        constexpr int found_or(int v) const { return found != -1 ? found : v; }
    };
    template<int M>
    struct comparator {
        const ctstr<M>& rhs;
        const int result;
        const bool eos;
        constexpr comparator(const ctstr<M>& rhs_, int result_=0, bool eos_=false)
        : rhs(rhs_), result(result_), eos(eos_)
        {}
        constexpr comparator<M> operator()(const int pos, const ctstr<N>& lhs) const {
            return comparator<M>(
                rhs,
                breaks() ? result : lhs[pos] > rhs[pos] ? 1 : lhs[pos] < rhs[pos] ? -1 : 0,
                eos || pos+1 >= N || pos+1 >= M || lhs[pos] == '\0' || rhs[pos] == '\0');
        }
        constexpr bool breaks() const { return eos || result != 0; }
    };

    template<class F>
    constexpr auto for_each(const F& f, const int begin=0, const int end=N) const -> F {
        return
            f.breaks() ? f :
            end - begin <= 0 ? f :
            end - begin > 2 ?
                (end - begin) % 2 == 0 ?
                     for_each(for_each(f, begin, end-(end-begin)/2),
                              begin+(end-begin)/2, end) :
                 for_each(for_each(f, begin, end-(end-begin)/2-1),
                          begin+(end-begin)/2, end) :
            end - begin == 2 ?
                for_each(f(begin, *this), begin+1, end) :
            f(begin, *this);
    }

    //
    constexpr int len() const {
        return for_each(finder<chartest>(chartest('\0'))).found_or(N);
    }
    template<class Pred>
    constexpr int find(const Pred& pred, int begin=0) const {
        return for_each(finder<Pred>(pred), begin, len()).found;
    }
    constexpr int find(const char c, int begin=0) const {
        return for_each(finder<chartest>(chartest(c))).found;
    }
    constexpr int capacity() const { return N; }
    constexpr ctstr<N> slice(const int first, const int last = 0) const {
        return ctstr<N>(data, first < 0 ? len() + first : first, last <= 0 ? len() + last : last);
    }
    template<int M>
    constexpr auto concat(const char (&v)[M]) const
    -> ctstr<N+M> {
        return ctstr<N+M>(data, v, len());
    }
    template<int M>
    constexpr auto concat(const ctstr<M>& s) const
    -> ctstr<N+M> {
        return ctstr<N+M>(data, s.data, len());
    }
    template<int M>
    constexpr int compare(const ctstr<M>& s) const {
        return for_each(comparator<M>(s)).result;
    }
    template<int M>
    constexpr int compare(const char (&v)[M]) const {
        return for_each(comparator<M>(ctstr<M>(v))).result;
    }
    template<int M>
    constexpr bool equals(const ctstr<M>& s) const { return compare(s) == 0; }
    template<int M>
    constexpr bool equals(const char (&v)[M]) const { return compare(v) == 0; }

    // operators
    constexpr char operator[](const int index) const { return data[index]; }

    template<int M>
    constexpr auto operator+(const char (&v)[M]) const -> ctstr<N+M> { return concat(v); }
    template<int M>
    constexpr auto operator+(const ctstr<M>& s) const -> ctstr<N+M> { return concat(s); }

    template<int M>
    constexpr bool operator==(const char (&v)[M]) const { return equals(v); }
    template<int M>
    constexpr bool operator==(const ctstr<M>& s) const { return equals(s); }

    //
    const char data[N+1];

private:
    template<int M, int...I>
    constexpr ctstr(const char (&v)[M], index_tuple<I...>, int first, int last)
    : data{(first+I < last ? v[first+I] : '\0')..., '\0'}
    {}
    template<int N1, int N2, int...I>
    constexpr ctstr(const char (&v1)[N1], const char (&v2)[N2], index_tuple<I...>, int size)
    : data{(I < size ? v1[I] : I-size < N2 ? v2[I-size] : '\0')..., '\0'}
    {}
    template<int...I>
    constexpr ctstr(const char c, index_tuple<I...>, int size)
    : data{(I < size ? c : '\0')..., '\0'}
    {}
};


inline
constexpr auto test(bool is)
-> decltype(is ? 10L : 10.f) {
    return is ? 10L : 10.f;
}

constexpr auto v = test(true);
auto tname = typeid(decltype(v)).name();
// static_assert(std::is_same<decltype(v), long>::value, "not long");
// static_assert(std::is_same<decltype(test(false)), float>::value, "not float");

template<int N>
constexpr auto make(const char (&v)[N])
-> ctstr<N> {
    return ctstr<N>(v);
}



}
