#ifndef COMPUTER_H
#define COMPUTER_H

#include <cstdlib>
#include <array>

//TODO Na koniec dać using tylko na używanych funkcjach
using namespace std;


class str_const {
private:
    const char* const id_value;
    const std::size_t length;

public:
    template<std::size_t N>
    constexpr str_const(const char (& a )[N]) : id_value(a), length(N - 1) {
        static_assert(1 <= N - 1 && N - 1 <= 6);
    }

    // TODO Friend do funkcji Id zamiast public
    constexpr char operator[](size_t n) const {
        return n < length ? id_value[n] :
               throw out_of_range("");
    }

    [[nodiscard]] constexpr size_t size() const {
        return length;
    }
};


constexpr str_const Id(const str_const a, const size_t i = 0) {
    return i == a.size() ? a : (((a[i] >= '0' && a[i] <= '0') || (a[i] >= 'A' && a[i] <= 'Z') ||
                                 (a[i] >= 'a' && a[i] <= 'z')) ? Id(a, i + 1) :
                                throw domain_error("Niedozwolone znaki w Id"));
}

template<auto N>
struct Num {
    constexpr static auto value = N;
};

struct Instruction {
};

template<typename... Instructions>
struct Program {
};

template<typename D, typename S>
struct Mov : Instruction {
};

template<typename D, typename S>
struct Add : Instruction {
};

template<typename D, typename S>
struct Sub : Instruction {
};

template<typename D>
using Inc = Add<D, Num<1>>;

template<typename D>
using Dec = Sub<D, Num<1>>;

template<typename A>
struct Mem {
};

template<typename D, typename S>
struct And : Instruction {
};

template<typename D, typename S>
struct Or : Instruction {
};

template<typename D>
struct Not : Instruction {
};

template<typename I, typename N>
struct D : Instruction {
};

template<size_t memory_size, typename memory_unit>
class Computer {
    
    using memory_t = array<memory_unit, memory_size>;
    using aliases_t = array<str_const, memory_size>;  //do mapowania aliasow na adresy (brute), jeszcze nie uzyte

    struct data {
        memory_t mem;
        bool ZF;
        bool SF;
    };
    using data_t = struct data;

    template<typename P>
    struct ASMProgram;

    template<typename... Instructions>
    struct ASMProgram<Program<Instructions...>> {
        constexpr static auto evaluate(data_t& data) {
            static_assert(((is_base_of<Instruction, Instructions>::value)&& ... && true),
                          "Error: a program should contain instructions only!");
            Evaluator<Instructions...>::evaluate(data);
            return data;
        }
    };

    template<typename A>
    struct AddrEvaluator {
    };

    /*template<auto N>
    struct AddrEvaluator<Mem<Num<N>>> {
        static_assert(Num<N>::value >= 0 && Num<N>::value < memory_size, "Error: an incorrect address!");
        constexpr static memory_unit value = Num<N>::value;
    };
    template<typename N>
    struct AddrEvaluator<Mem<N>> { //TODO Źle, dla np. Mem<Mem<Num<N>>> ma dawać memory[Mem<Num<N>>], podobnie jak w RValue
        constexpr static auto val = AddrEvaluator<N>::value;
        static_assert(val >= 0 && val < memory_size, "Error: an incorrect address!");
        constexpr static memory_unit value = val;
    };*/

    //proba poprawy powyzszego
    template<typename N>
    struct AddrEvaluator<Mem<N>> {
        constexpr static auto val = RValue<N>::value;
        static_assert(val >= 0 && val < memory_size, "Error: an incorrect address!");
        constexpr static memory_unit value = val;
    };
    

    template<typename A>
    struct RValue {
        constexpr static memory_unit val(data_t& data);
    };

    template<auto N>
    struct RValue<Num<N>> {
        constexpr static memory_unit val(data_t& data) {
            return Num<N>::value;
        }
    };

    template<typename N>
    struct RValue<Mem<N>> {
        constexpr static memory_unit val(data_t& data) {
            return data::mem[AddrEvaluator<Mem<N>>::value];
        }
    };

    // TODO RValue dla Lea

    template<typename... Instructions>
    struct Evaluator {
        constexpr static void evaluate(data_t& data) {}
    };

    template<typename I, typename... Rest>
    struct Evaluator<I, Rest...> {
        constexpr static void evaluate(data_t& data) {}
    };

    template<typename Arg1, typename Arg2, typename... Rest>
    struct Evaluator<Mov<Arg1, Arg2>, Rest...> {
        constexpr static void evaluate(data_t& data) {
            auto addr = AddrEvaluator<Arg1>::value;
            data::mem[addr] = Arg2::value;
            Evaluator<Rest...>::evaluate(data);
        }
    };

    //TODO Ustawianie flag procesora
    template<typename Arg1, typename Arg2, typename... Rest>
    struct Evaluator<Add<Arg1, Arg2>, Rest...> {
        constexpr static void evaluate(data_t& data) {
            auto addr = AddrEvaluator<Arg1>::value;
            data::mem[addr] += RValue<Arg2>::val(data::mem);
            Evaluator<Rest...>::evaluate(data::mem);
        }
    };

    template<typename Arg1, typename Arg2, typename... Rest>
    struct Evaluator<Sub<Arg1, Arg2>, Rest...> {
        constexpr static void evaluate(data_t& data) {
            auto addr = AddrEvaluator<Arg1>::value;
            data::mem[addr] -= RValue<Arg2>::val(data);
            Evaluator<Rest...>::evaluate(data);
        }
    };

    template<typename Arg1, typename Arg2, typename... Rest>
    struct Evaluator<And<Arg1, Arg2>, Rest...> {
        constexpr static void evaluate(data_t& data) {
            auto addr = AddrEvaluator<Arg1>::value;
            data::mem[addr] &= RValue<Arg2>::val(data);
            Evaluator<Rest...>::evaluate(data);
        }
    };

    template<typename Arg1, typename Arg2, typename... Rest>
    struct Evaluator<Or<Arg1, Arg2>, Rest...> {
        constexpr static void evaluate(data_t& data) {
            auto addr = AddrEvaluator<Arg1>::value;
            data::mem[addr] |= RValue<Arg2>::val(data);
            Evaluator<Rest...>::evaluate(data);
        }
    };

    template<typename Arg, typename... Rest>
    struct Evaluator<Not<Arg>, Rest...> {
        constexpr static void evaluate(data_t& data) {
            auto addr = AddrEvaluator<Arg>::value;
            data::mem[addr] = ~data::mem[addr];
            Evaluator<Rest...>::evaluate(data);
        }
    };

    template<typename I, typename N, typename... Rest>
    struct Evaluator<D<I, N>, Rest...> {
        constexpr static void evaluate(data_t& data) {
            Evaluator<Rest...>::evaluate(data);
        }
    };

public:
    template<typename P>
    constexpr static auto boot() {
        data_t data = {
            {0}, //mem
            false, //ZF
            false //SF
        };
        return ASMProgram<P>::evaluate(data)::mem;
    }
};


#endif //COMPUTER_H
