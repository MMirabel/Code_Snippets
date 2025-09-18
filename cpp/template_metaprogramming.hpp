/**
 * Template Metaprogramming Utilities and Type Traits
 * 
 * Header-only C++17 utilities for:
 * - Custom type traits
 * - SFINAE helpers
 * - Compile-time computations
 * - Template specialization patterns
 * 
 * Complessità: O(1) runtime (tutto compile-time)
 * Compatibilità: C++17 o superiore
 */

#ifndef TEMPLATE_METAPROGRAMMING_HPP
#define TEMPLATE_METAPROGRAMMING_HPP

#include <type_traits>
#include <utility>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

namespace meta {

// =============================================================================
// TYPE DETECTION UTILITIES
// =============================================================================

// Helper per rilevare se un tipo ha un metodo specifico
#define DEFINE_HAS_METHOD(method_name) \
    template<typename T, typename = void> \
    struct has_##method_name : std::false_type {}; \
    \
    template<typename T> \
    struct has_##method_name<T, std::void_t<decltype(std::declval<T>().method_name())>> \
        : std::true_type {}; \
    \
    template<typename T> \
    constexpr bool has_##method_name##_v = has_##method_name<T>::value;

// Definizioni per metodi comuni
DEFINE_HAS_METHOD(size)
DEFINE_HAS_METHOD(begin)
DEFINE_HAS_METHOD(end)
DEFINE_HAS_METHOD(push_back)
DEFINE_HAS_METHOD(to_string)

// Type trait per container-like types
template<typename T>
struct is_container : std::conjunction<
    has_begin<T>,
    has_end<T>,
    std::negation<std::is_same<T, std::string>>
> {};

template<typename T>
constexpr bool is_container_v = is_container<T>::value;

// Type trait per iterable types
template<typename T, typename = void>
struct is_iterable : std::false_type {};

template<typename T>
struct is_iterable<T, std::void_t<
    decltype(std::begin(std::declval<T>())),
    decltype(std::end(std::declval<T>()))
>> : std::true_type {};

template<typename T>
constexpr bool is_iterable_v = is_iterable<T>::value;

// =============================================================================
// SFINAE HELPERS
// =============================================================================

// Enable if helpers
template<bool B>
using enable_if_t = typename std::enable_if_t<B>;

template<typename T>
using enable_if_integral_t = enable_if_t<std::is_integral_v<T>>;

template<typename T>
using enable_if_floating_t = enable_if_t<std::is_floating_point_v<T>>;

template<typename T>
using enable_if_container_t = enable_if_t<is_container_v<T>>;

// =============================================================================
// COMPILE-TIME COMPUTATIONS
// =============================================================================

// Fattoriale compile-time
template<int N>
constexpr long long factorial() {
    static_assert(N >= 0, "Factorial of negative number");
    if constexpr (N == 0 || N == 1) {
        return 1;
    } else {
        return N * factorial<N - 1>();
    }
}

// Potenza compile-time
template<int Base, int Exp>
constexpr long long power() {
    static_assert(Exp >= 0, "Negative exponent not supported");
    if constexpr (Exp == 0) {
        return 1;
    } else {
        return Base * power<Base, Exp - 1>();
    }
}

// Fibonacci compile-time
template<int N>
constexpr long long fibonacci() {
    static_assert(N >= 0, "Fibonacci of negative number");
    if constexpr (N == 0) {
        return 0;
    } else if constexpr (N == 1) {
        return 1;
    } else {
        return fibonacci<N - 1>() + fibonacci<N - 2>();
    }
}

// Check se numero è primo (compile-time)
template<int N, int Divisor = N - 1>
constexpr bool is_prime_helper() {
    if constexpr (Divisor == 1) {
        return true;
    } else {
        return (N % Divisor != 0) && is_prime_helper<N, Divisor - 1>();
    }
}

template<int N>
constexpr bool is_prime() {
    static_assert(N > 0, "Prime check for non-positive number");
    if constexpr (N == 1) {
        return false;
    } else if constexpr (N == 2) {
        return true;
    } else {
        return is_prime_helper<N>();
    }
}

// =============================================================================
// TYPE LIST UTILITIES
// =============================================================================

// Lista di tipi
template<typename... Types>
struct type_list {
    static constexpr std::size_t size = sizeof...(Types);
};

// Ottieni tipo per indice
template<std::size_t Index, typename... Types>
struct type_at;

template<std::size_t Index, typename Head, typename... Tail>
struct type_at<Index, Head, Tail...> {
    using type = typename type_at<Index - 1, Tail...>::type;
};

template<typename Head, typename... Tail>
struct type_at<0, Head, Tail...> {
    using type = Head;
};

template<std::size_t Index, typename... Types>
using type_at_t = typename type_at<Index, Types...>::type;

// =============================================================================
// FUNCTION TRAITS
// =============================================================================

template<typename F>
struct function_traits;

// Specializzazione per function pointers
template<typename R, typename... Args>
struct function_traits<R(*)(Args...)> {
    using return_type = R;
    using argument_types = type_list<Args...>;
    static constexpr std::size_t arity = sizeof...(Args);
    
    template<std::size_t Index>
    using argument_type = type_at_t<Index, Args...>;
};

// Specializzazione per function types (non-pointers)
template<typename R, typename... Args>
struct function_traits<R(Args...)> {
    using return_type = R;
    using argument_types = type_list<Args...>;
    static constexpr std::size_t arity = sizeof...(Args);
    
    template<std::size_t Index>
    using argument_type = type_at_t<Index, Args...>;
};

// Specializzazione per member functions
template<typename R, typename C, typename... Args>
struct function_traits<R(C::*)(Args...)> {
    using return_type = R;
    using class_type = C;
    using argument_types = type_list<Args...>;
    static constexpr std::size_t arity = sizeof...(Args);
    
    template<std::size_t Index>
    using argument_type = type_at_t<Index, Args...>;
};

// =============================================================================
// TEMPLATE SPECIALIZATION PATTERNS
// =============================================================================

// Dispatcher basato sul tipo
template<typename T>
struct type_dispatcher {
    static std::string describe() {
        return "Unknown type";
    }
};

// Specializzazioni per tipi specifici
template<>
struct type_dispatcher<int> {
    static std::string describe() {
        return "32-bit signed integer";
    }
};

template<>
struct type_dispatcher<double> {
    static std::string describe() {
        return "Double precision floating point";
    }
};

template<>
struct type_dispatcher<std::string> {
    static std::string describe() {
        return "String container";
    }
};

// Specializzazione per vector
template<typename T>
struct type_dispatcher<std::vector<T>> {
    static std::string describe() {
        return "Vector of " + type_dispatcher<T>::describe();
    }
};

// =============================================================================
// CONDITIONAL COMPILATION UTILITIES
// =============================================================================

// Esegui funzioni diverse basate sui type traits
template<typename T>
auto process_value(T&& value) {
    if constexpr (std::is_integral_v<std::decay_t<T>>) {
        return std::to_string(value) + " (integer)";
    } else if constexpr (std::is_floating_point_v<std::decay_t<T>>) {
        return std::to_string(value) + " (float)";
    } else if constexpr (is_container_v<std::decay_t<T>>) {
        std::ostringstream oss;
        oss << "Container with " << value.size() << " elements";
        return oss.str();
    } else {
        return std::string("Unknown type");
    }
}

// Stampa contenuto se è un container
template<typename T>
void print_if_container(const T& value) {
    if constexpr (is_container_v<T>) {
        std::cout << "Container contents: ";
        for (const auto& elem : value) {
            std::cout << elem << " ";
        }
        std::cout << "\n";
    } else {
        std::cout << "Not a container: " << value << "\n";
    }
}

// =============================================================================
// TAG DISPATCH PATTERN
// =============================================================================

// Tag types per dispatch
struct integral_tag {};
struct floating_tag {};
struct container_tag {};
struct unknown_tag {};

// Tag selector
template<typename T>
using select_tag = std::conditional_t<
    std::is_integral_v<T>, integral_tag,
    std::conditional_t<
        std::is_floating_point_v<T>, floating_tag,
        std::conditional_t<
            is_container_v<T>, container_tag,
            unknown_tag
        >
    >
>;

// Implementazioni specifiche per tag
template<typename T>
std::string process_with_tag(const T& value, integral_tag) {
    return "Processing integer: " + std::to_string(value);
}

template<typename T>
std::string process_with_tag(const T& value, floating_tag) {
    return "Processing float: " + std::to_string(value);
}

template<typename T>
std::string process_with_tag(const T& value, container_tag) {
    return "Processing container with " + std::to_string(value.size()) + " elements";
}

template<typename T>
std::string process_with_tag(const T&, unknown_tag) {
    return "Processing unknown type";
}

// Interfaccia unificata
template<typename T>
std::string process_with_dispatch(const T& value) {
    return process_with_tag(value, select_tag<T>{});
}

} // namespace meta

// =============================================================================
// USAGE EXAMPLES
// =============================================================================

#ifdef TEMPLATE_METAPROGRAMMING_EXAMPLE

#include <array>

// Esempio di funzione per test
int test_function(double x, int y) {
    return static_cast<int>(x) + y;
}

// Classe per test
class TestClass {
public:
    void method1() {}
    std::string to_string() const { return "TestClass"; }
    std::size_t size() const { return 42; }
};

void example_type_traits() {
    std::cout << "=== Type Traits Example ===\n";
    
    // Test has_method traits
    std::cout << "std::vector<int> has size(): " << meta::has_size_v<std::vector<int>> << "\n";
    std::cout << "int has size(): " << meta::has_size_v<int> << "\n";
    std::cout << "TestClass has to_string(): " << meta::has_to_string_v<TestClass> << "\n";
    
    // Test is_container
    std::cout << "std::vector<int> is container: " << meta::is_container_v<std::vector<int>> << "\n";
    std::cout << "std::string is container: " << meta::is_container_v<std::string> << "\n";
    std::cout << "int is container: " << meta::is_container_v<int> << "\n";
    
    std::cout << "\n";
}

void example_compile_time_computations() {
    std::cout << "=== Compile-time Computations ===\n";
    
    // Test compile-time computations
    constexpr auto fact5 = meta::factorial<5>();
    constexpr auto pow23 = meta::power<2, 3>();
    constexpr auto fib10 = meta::fibonacci<10>();
    constexpr auto prime17 = meta::is_prime<17>();
    constexpr auto prime18 = meta::is_prime<18>();
    
    std::cout << "5! = " << fact5 << "\n";
    std::cout << "2^3 = " << pow23 << "\n";
    std::cout << "fibonacci(10) = " << fib10 << "\n";
    std::cout << "17 is prime: " << prime17 << "\n";
    std::cout << "18 is prime: " << prime18 << "\n";
    
    std::cout << "\n";
}

void example_function_traits() {
    std::cout << "=== Function Traits Example ===\n";
    
    using traits = meta::function_traits<decltype(&test_function)>;
    
    std::cout << "Function arity: " << traits::arity << "\n";
    std::cout << "Return type is int: " 
              << std::is_same_v<traits::return_type, int> << "\n";
    std::cout << "First arg is double: " 
              << std::is_same_v<traits::argument_type<0>, double> << "\n";
    std::cout << "Second arg is int: " 
              << std::is_same_v<traits::argument_type<1>, int> << "\n";
    
    std::cout << "\n";
}

void example_type_dispatcher() {
    std::cout << "=== Type Dispatcher Example ===\n";
    
    std::cout << "int: " << meta::type_dispatcher<int>::describe() << "\n";
    std::cout << "double: " << meta::type_dispatcher<double>::describe() << "\n";
    std::cout << "std::string: " << meta::type_dispatcher<std::string>::describe() << "\n";
    std::cout << "std::vector<int>: " << meta::type_dispatcher<std::vector<int>>::describe() << "\n";
    
    std::cout << "\n";
}

void example_conditional_compilation() {
    std::cout << "=== Conditional Compilation Example ===\n";
    
    int integer_val = 42;
    double float_val = 3.14;
    std::vector<int> container_val = {1, 2, 3, 4, 5};
    std::string string_val = "hello";
    
    std::cout << meta::process_value(integer_val) << "\n";
    std::cout << meta::process_value(float_val) << "\n";
    std::cout << meta::process_value(container_val) << "\n";
    std::cout << meta::process_value(string_val) << "\n";
    
    meta::print_if_container(container_val);
    meta::print_if_container(integer_val);
    
    std::cout << "\n";
}

void example_tag_dispatch() {
    std::cout << "=== Tag Dispatch Example ===\n";
    
    int integer_val = 42;
    double float_val = 3.14;
    std::vector<int> container_val = {1, 2, 3};
    std::string string_val = "hello";
    
    std::cout << meta::process_with_dispatch(integer_val) << "\n";
    std::cout << meta::process_with_dispatch(float_val) << "\n";
    std::cout << meta::process_with_dispatch(container_val) << "\n";
    std::cout << meta::process_with_dispatch(string_val) << "\n";
    
    std::cout << "\n";
}

int main() {
    std::cout << "Template Metaprogramming Examples\n";
    std::cout << "=================================\n\n";
    
    example_type_traits();
    example_compile_time_computations();
    example_function_traits();
    example_type_dispatcher();
    example_conditional_compilation();
    example_tag_dispatch();
    
    std::cout << "All examples completed!\n";
    return 0;
}

#endif // TEMPLATE_METAPROGRAMMING_EXAMPLE

#endif // TEMPLATE_METAPROGRAMMING_HPP