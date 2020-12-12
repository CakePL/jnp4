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

template<size_t memory_size, typename memory_unit>
class Computer {
    using memory_t = array<memory_unit, memory_size>;

    template<typename P>
    struct ASMProgram;

    template<typename... Instructions>
    struct ASMProgram<Program<Instructions...>> {
        constexpr static auto evaluate(memory_t& mem) {
            static_assert(((is_base_of<Instruction, Instructions>::value)&& ... && true),
                          "Error: a program should contain instructions only!");
            Evaluator<Instructions...>::evaluate(mem);
            return mem;
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
        constexpr static memory_unit val(memory_t& mem);
    };

    template<auto N>
    struct RValue<Num<N>> {
        constexpr static memory_unit val(memory_t& mem) {
            return Num<N>::value;
        }
    };

    template<typename N>
    struct RValue<Mem<N>> {
        constexpr static memory_unit val(memory_t& mem) {
            return mem[AddrEvaluator<Mem<N>>::value];
        }
    };
    // TODO RValue dla Lea


    template<typename... Instructions>
    struct Evaluator {
        constexpr static void evaluate(memory_t& mem) {}
    };

    template<typename I, typename... Rest>
    struct Evaluator<I, Rest...> {
        constexpr static void evaluate(memory_t& mem) {}
    };

    template<typename Arg1, typename Arg2, typename... Rest>
    struct Evaluator<Mov<Arg1, Arg2>, Rest...> {
        constexpr static void evaluate(memory_t& mem) {
            auto addr = AddrEvaluator<Arg1>::value;
            mem[addr] = Arg2::value;
            Evaluator<Rest...>::evaluate(mem);
        }
    };

    //TODO Ustawianie flag procesora
    template<typename Arg1, typename Arg2, typename... Rest>
    struct Evaluator<Add<Arg1, Arg2>, Rest...> {
        constexpr static void evaluate(memory_t& mem) {
            auto addr = AddrEvaluator<Arg1>::value;
            mem[addr] += RValue<Arg2>::val(mem);
            Evaluator<Rest...>::evaluate(mem);
        }
    };

    template<typename Arg1, typename Arg2, typename... Rest>
    struct Evaluator<Sub<Arg1, Arg2>, Rest...> {
        constexpr static void evaluate(memory_t& mem) {
            auto addr = AddrEvaluator<Arg1>::value;
            mem[addr] -= RValue<Arg2>::val(mem);
            Evaluator<Rest...>::evaluate(mem);
        }
    };

    template<typename Arg1, typename Arg2, typename... Rest>
    struct Evaluator<And<Arg1, Arg2>, Rest...> {
        constexpr static void evaluate(memory_t& mem) {
            auto addr = AddrEvaluator<Arg1>::value;
            mem[addr] &= RValue<Arg2>::val(mem);
            Evaluator<Rest...>::evaluate(mem);
        }
    };

    template<typename Arg1, typename Arg2, typename... Rest>
    struct Evaluator<Or<Arg1, Arg2>, Rest...> {
        constexpr static void evaluate(memory_t& mem) {
            auto addr = AddrEvaluator<Arg1>::value;
            mem[addr] |= RValue<Arg2>::val(mem);
            Evaluator<Rest...>::evaluate(mem);
        }
    };

    template<typename Arg, typename... Rest>
    struct Evaluator<Not<Arg>, Rest...> {
        constexpr static void evaluate(memory_t& mem) {
            auto addr = AddrEvaluator<Arg>::value;
            mem[addr] = ~mem[addr];
            Evaluator<Rest...>::evaluate(mem);
        }
    };


public:
    template<typename P>
    constexpr static auto boot() {
        memory_t memory{0};
        return ASMProgram<P>::evaluate(memory);
    }
};


#endif //COMPUTER_H
