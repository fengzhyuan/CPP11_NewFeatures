/*
	a brief note about new features in c++ 11;
	taken from http://www.stroustrup.com/C++11FAQ.html#learn
*/

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
using namespace std;

/// enum class
enum class Color:int{red, green, blue};

/// constexpr
struct structFoo {
	int x{ 0 }, y{ 0 };
	constexpr structFoo(int x_, int y_) :x(x_), y(y_) {}
};

/// decltype
typedef decltype(1.0f) ReturnType;
ReturnType foo() { ReturnType t = 1; return t; }

/// delegating constructures
// old
class classFoo {
	int a;
	void validate(int x) { if (x >= INT_MAX) throw exception("bad number"); }
public:
	classFoo(int x) { validate(x); }
	classFoo() { validate(42); }
	classFoo(string s){}
};
// new
class clsFoo_new {
	int a;
	void validate(int x) { if (x >= INT_MAX) throw exception("bad number"); }
public:
	clsFoo_new(int x) { validate(x); }
	clsFoo_new() :clsFoo_new{ 42 }{}
	clsFoo_new(string s) {/*whatever*/}
};

/// inherited constructures
struct structA {
	void f(double x) {}
	structA(int x_) :x(x_) {}
	int x;
};
struct structD:structA {
	void f(int x) {}
	int x;
};
/*
	1.
	lifting base function
	old method: 
		using structA::f;
	new method:
		using structA::structA;
	2.
	once applynig 1, coding like `structD d(0)` is illegal;
	instead the `member-initializer` can be used to avoid such error
*/
struct structD_new :structA {
	using structA::structA;
	void f(int x) {}
	int x{ 0 };
};

/**
	static (compiler time) assertions: static_assert
*/
template<typename T>
struct structS {
	structS(T t) { static_assert(sizeof(T) >= 4, "param need to be at least 32-bit length"); }
};

/// suffix return type syntax
template<class T, class U>
auto mul(T t, U u) -> decltype(t*u) {
	return t*u;
}

/// variadic templates
/*
	a neat introduction http://www.cnblogs.com/qicosmos/p/4325949.html
*/
/*	definition: followed by `...`
	T...: arg pack, contains N >= 0 # of parameters
	can't get every parameter directly, have to expand each param one by one
*/
template <class... T>
void variadic_f(T... args) {} 

/*
	1. expand param pack recursively
*/
// recursion termination func
void print() {
	cout << "empty" << endl;
}
// expansion func
template <class T, class... Args>
void print(T head, Args... rest) {
	cout << "param " << head << endl;
	print(rest...);
}
// or defined as
template<class T>
void print_new(T t) {
	cout << t << endl;
}

template<class T, class... Args>
void print_new(T head, Args... rest) {
	cout << "param " << head << endl;
	print_new(rest...);
}
// another example
template<typename T>
T sum(T t) { 
	return t; 
}

template<typename T, typename... Args>
T sum(T first, Args... rest) {
	return first + sum<T>(rest...);
}

/*
	2. expand params via comma expression
*/
template <typename T>
void print_arg(T t) {
	cout << t << endl;
}

template <typename ...Args> 
void print_arg(Args... args) {
	int arr[] = { (print_arg(args), 0)... };
}

/*
	remove duplicate code via variadic template
*/

//// before
template<typename T>
T* Instance() { return new T(); }

template<typename T, typename T0>
T* Instance(T0 arg0) { return new T(arg0); }

template<typename T, typename T0, typename T1>
T* Instance(T0 arg0, T1 arg1) { return new T(arg0, arg1); }

template<typename T, typename T0, typename T1, typename T2>
T* Instance(T0 arg0, T1 arg1, T2 arg2) { return new T(arg0, arg1, arg2); }

//// after
template<typename T, typename... Args>
T* Instance(Args&&... args) { return new T(std::forward<Args>(args)...); }

struct A {
	A(int) {}
	void foo(int i) { cout << i << endl; }
	void foo2(int i, double j) { cout << i << " " << j << endl; }
};
struct B {
	B(int, double) {}
};

/*
	implement delegate via variadic template
	only onsider member function for simplicity
*/
template<class T, class R, typename ...Args>
class MyDelegate {
public:
	MyDelegate(T* t, R(T::*f)(Args...)) :m_t(t), m_f(f) {}
	R operator() (Args&&... args) {
		return (m_t->*m_f)(std::forward<Args>(args)...);
	}
private:
	T* m_t;
	R(T::*m_f)(Args...);
};

template<class T, class R, typename ...Args>
MyDelegate<T, R, Args...> CreateDelegate(T* t, R(T::*f)(Args...)) {
	return MyDelegate<T, R, Args...>(t, f);
}


int main()
{
	Color c = Color::red;
	int i = (int)c;
	int j = 1.0;	// OK
	//j = { 1.0 };	// error: narrowing

	constexpr structFoo f(0, 1);

	structA a(3);
	structD_new d(3); // d.x will be 0

	structS<int> s(0);
	//structS<char> s2('0'); // will cause compiler-time error: `param need to be at least 32-bit length`
	auto r = mul(1, 2.0);
	cout << "suffix return type syntax\n" << typeid(r).name() << endl; // will print double
	
	// variadic template
	// expansion process:
	// print(1, 2, 3, 4)
	// print(2, 3, 4)
	// print(3, 4)
	// print(4)
	// print()
	cout << "variadic tempalte\n";
	print(1, 2, 3, 4);
	// expansion process
	// print_new(1, 2, 3, 4)
	// print_new(2, 3, 4)
	// print_new(3, 4)
	// print_new(4)
	cout << "variadic template_new\n";
	print_new(1, 2, 3, 4);

	cout << "variadic example, recursive expansion\n";
	cout << sum(1, 2, 3, 4) << endl;

	cout << "variadic example, comma expansion \n";
	print_arg(1, 2, 3, 4);

	A* pa = Instance<A>(1);
	B* pb = Instance<B>(1, 2);
	delete pa, pb;

	cout << "delegate implementation example\n";
	A delegateA(0);
	auto delegate_d = CreateDelegate(&delegateA, &A::foo);
	delegate_d(1);
	auto delegate_d2 = CreateDelegate(&delegateA, &A::foo2);
	delegate_d2(1, 2.1);


    return 0;
}

