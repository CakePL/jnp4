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

    constexpr static bool compare(const str_const& a, const str_const& b) {
        //TODO
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

template<typename A, typename B>
struct Cmp : Instruction {
};

template<typename I>
struct Lea : Instruction {
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
    using aliases_t = array<str_const, memory_size>;  //do mapowania aliasow na adresy (brute)

    using data_t = struct data {
        memory_t mem;
        bool ZF;
        bool SF;
        aliases_t aliases;
        std::size_t index;
    };
    

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

    template<>
    struct RValue<Lea<str_const alias>> {
        constexpr static memory_unit val(data_t& data) {
            size_t i = 0;
            while (i < data::index && !compare(alias, data::mem[i]))
                ++i;
            static_assert(i < data::index, "Error: variable not declared!");
            return (memory_unit)i;
        };
    };

    template<typename... Instructions>
    struct DeclarationEvaluator {
        constexpr static void evaluate(data_t& data) {}
    };

    template<typename I, typename... Rest>
    struct DeclarationEvaluator<I, Rest...> {
        constexpr static void evaluate(data_t& data) {
            DeclarationEvaluator<Rest...>::evaluate(data);
        }
    };

    template<auto N, typename... Rest>
    struct DeclarationEvaluator<D<str_const A, Num<N>, Rest...> {
        constexpr static void evaluate(data_t& data) {
            static_assert(data::index < memory_size, "Error: not enough memory!");
            data::aliases[index] = A;
            ++data::index;
            Evaluator<Rest...>::evaluate(data);
        }
    };

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

    template<typename Arg1, typename Arg2, typename... Rest>
    struct Evaluator<Add<Arg1, Arg2>, Rest...> {
        constexpr static void evaluate(data_t& data) {
            auto addr = AddrEvaluator<Arg1>::value;
            data::mem[addr] += RValue<Arg2>::val(data::mem);
            data::ZF = data::mem[addr] == 0;
            data::SF = data::mem[addr] < 0;
            Evaluator<Rest...>::evaluate(data::mem);
        }
    };

    template<typename Arg1, typename Arg2, typename... Rest>
    struct Evaluator<Sub<Arg1, Arg2>, Rest...> {
        constexpr static void evaluate(data_t& data) {
            auto addr = AddrEvaluator<Arg1>::value;
            data::mem[addr] -= RValue<Arg2>::val(data);
            data::ZF = data::mem[addr] == 0;
            data::SF = data::mem[addr] < 0;
            Evaluator<Rest...>::evaluate(data);
        }
    };

    template<typename Arg1, typename Arg2, typename... Rest>
    struct Evaluator<Cmp<Arg1, Arg2>, Rest...> {
        constexpr static void evaluate(data_t& data) {
            memory_unit result = RValue<Arg1>::val(data) - RValue<Arg2>::val(data);
            data::ZF = data::mem[result] == 0;
            data::SF = data::mem[result] < 0;
            Evaluator<Rest...>::evaluate(data);
        }
    };

    template<typename Arg1, typename Arg2, typename... Rest>
    struct Evaluator<And<Arg1, Arg2>, Rest...> {
        constexpr static void evaluate(data_t& data) {
            auto addr = AddrEvaluator<Arg1>::value;
            data::mem[addr] &= RValue<Arg2>::val(data);
            data::ZF = data::mem[addr] == 0;
            Evaluator<Rest...>::evaluate(data);
        }
    };

    template<typename Arg1, typename Arg2, typename... Rest>
    struct Evaluator<Or<Arg1, Arg2>, Rest...> {
        constexpr static void evaluate(data_t& data) {
            auto addr = AddrEvaluator<Arg1>::value;
            data::mem[addr] |= RValue<Arg2>::val(data);
            data::ZF = data::mem[addr] == 0;
            Evaluator<Rest...>::evaluate(data);
        }
    };

    template<typename Arg, typename... Rest>
    struct Evaluator<Not<Arg>, Rest...> {
        constexpr static void evaluate(data_t& data) {
            auto addr = AddrEvaluator<Arg>::value;
            data::mem[addr] = ~data::mem[addr];
            data::ZF = data::mem[addr] == 0;
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
        //TODO upewnic sie, czy inicjalizacja jest poprawna technicznie, ew zmienic jej forme dbajac o robienie tego w compiletime
        data_t data = {
            {0}, //mem
            false, //ZF
            false, //SF
            , //TODO inicjalizacja aliases
            0
        };
        return ASMProgram<P>::evaluate(data)::mem;
    }
};


#endif //COMPUTER_H
