#ifndef MAIN_HPP
#define MAIN_HPP
#include <memory>
#include <type_traits>
#include <compare>
#include <utility>
namespace bit{
template <class T, class Del=std::default_delete<T>>
class unique_allocate{
	private:
	using pointer=T*;
	std::type_identity_t<pointer> p;
	T element_type;
	Del deleter;
	public:
	constexpr unique_allocate()noexcept requires (std::is_default_constructible_v<Del> && !std::is_pointer_v<T>):p{}, deleter{}{}
	constexpr unique_allocate(std::nullptr_t)noexcept requires (std::is_default_constructible_v<Del> && !std::is_pointer_v<T>):p{nullptr}, deleter{}{}
	constexpr explicit unique_allocate(std::type_identity_t<pointer> p)noexcept requires (std::is_default_constructible_v<Del> && !std::is_pointer_v<Del>):p{p}, deleter{}{}
	constexpr unique_allocate(std::type_identity_t<pointer> p, const Del& d)noexcept requires std::is_constructible_v<Del, decltype(d)>:p{p}, deleter{std::forward<decltype(d)>(d)}{}
	constexpr unique_allocate(std::type_identity_t<pointer> p, std::remove_reference_t<Del>&& d)noexcept requires (std::is_move_constructible_v<Del>)&&(std::is_constructible_v<Del, decltype(d)>):p{p}, deleter{std::forward<decltype(d)>(d)}{}
	template <class U, class E>
	constexpr unique_allocate(unique_allocate<U,E>&& d)noexcept requires (!std::is_array_v<U>) && std::is_move_constructible_v<E>:p{d.release()}, deleter{std::forward<Del>(d.get_deleter())}{}
	constexpr unique_allocate(unique_allocate&& d)noexcept requires std::is_move_constructible_v<Del>:p{nullptr}{swap(d);}
	unique_allocate(const unique_allocate& a)noexcept=delete;
	unique_allocate& operator=(unique_allocate&& r)noexcept{
		swap(r);
		return *this;
	}
	template <class U, class E>
		unique_allocate& operator=(unique_allocate<U, E>&& r)noexcept{
			swap(r);
			return *this;
		}
	pointer get()const noexcept{
		return p;
	}
	constexpr Del& get_deleter()noexcept{
		return deleter;
	}
	pointer release()noexcept{
		auto old_ptr=p;
		p=nullptr;
		return old_ptr;
	}
	void reset(pointer ptr=pointer()){
		auto old_ptr=p;
		p=ptr;
		if(old_ptr!=nullptr){
			get_deleter()(old_ptr);
		}
	}
	explicit operator bool()const noexcept{
		if(get()!=nullptr){
			return true;
		}
		return false;
	}
	constexpr T& operator[](std::size_t i)const{
		return get()[i];
	}
	typename std::add_lvalue_reference_t<T> operator*()const noexcept(noexcept(*std::declval<pointer>())){
		return *get();
	}
	void swap(unique_allocate& other)noexcept{
		std::swap(p, other.p);
		std::swap(deleter, other.deleter);
	}
	pointer operator->(){
		return get();
	}
	~unique_allocate(){
		if(get()!=nullptr){
			get_deleter();
		}
	}
};
template <class T, class Deleter>
class unique_allocate<T[], Deleter>{
	private:
	using pointer=T*;
	pointer p;
	T element_type;
	Deleter del;
	public:
	unique_allocate(pointer ptr, Deleter del)noexcept:p{ptr}, del{del}{}
	unique_allocate(pointer ptr)noexcept:p{ptr}, del{std::default_delete<T[]>()}{}
	unique_allocate()noexcept:p{}, del{}{}
	unique_allocate(std::nullptr_t)noexcept:p{nullptr}, del{nullptr}{}
	unique_allocate(unique_allocate&& r)noexcept{
		p=r.release();
	}
	unique_allocate operator=(unique_allocate&& r)noexcept{
		reset(r.release());	
		return *this;
	}
	constexpr Deleter get_deleter()const noexcept{
		return del;
	}
	constexpr auto release()noexcept{
		auto old_ptr=p;
		p=nullptr;
		return old_ptr;
	}
	constexpr auto reset(pointer p=pointer()){
		auto old_ptr=p;
		p=nullptr;
		if(p!=nullptr){
			get_deleter()(p);
		}
	}
	unique_allocate(const unique_allocate& u)noexcept=delete;
	~unique_allocate(){
		if(p!=nullptr){
			get_deleter();
		}
	}
	pointer get()const noexcept{
		return p;
	}
	constexpr T& operator[](std::size_t index)noexcept{
		return get()[index];
	}
	T& operator*()const noexcept{
		return *get();
	}
	T* operator->()const noexcept{
		return get();
	}
};
}
namespace details{
	template <class>
		bool is_ubarray=false;
	template <class T>
		bool is_ubarray<T[]> = true;
	template <class>
		bool is_barray=false;
	template <class T, std::size_t size>
		bool is_barray<T[size]> = true;
}
template <class T1, class D1, class T2, class D2>
constexpr bool operator==(const bit::unique_allocate<T1, D1>& x, const bit::unique_allocate<T2, D2>& y){
	return x.get()==y.get();
}
template <class T1, class D1, class T2, class D2>
constexpr bool operator!=(const bit::unique_allocate<T1, D1>& x, const bit::unique_allocate<T2, D2>& y){
	return x.get()!=y.get();
}
template <class T1, class D1, class T2, class D2>
constexpr bool operator<(const bit::unique_allocate<T1, D1>& x, const bit::unique_allocate<T2, D2>& y){
	return x.get()<y.get();
}
template <class T1, class D1, class T2, class D2>
constexpr bool operator>(const bit::unique_allocate<T1, D2>& x, const bit::unique_allocate<T2, D2>& y){
	return x.get()>y.get();
}
template <class T1, class D1, class T2, class D2>
constexpr bool operator<=(const bit::unique_allocate<T1, D1>& x, const bit::unique_allocate<T2, D2>& y){
	return x.get()<=y.get();
}
template <class T1, class D1, class T2, class D2>
constexpr bool operator>=(const bit::unique_allocate<T1, D1>& x, const bit::unique_allocate<T2, D2>& y){
	return x.get()>=y.get();
}
template <class T1, class D1, class T2, class D2>
constexpr auto operator<=>(const bit::unique_allocate<T1, D1>& x, const bit::unique_allocate<T2, D2>& y){
	return std::compare_three_way{}(x.get(), y.get());
}
template <class T, class D>
constexpr bool operator==(const bit::unique_allocate<T, D>& x, std::nullptr_t){
	return !x;
}
template <class T, class D>
constexpr bool operator==(std::nullptr_t a, const bit::unique_allocate<T, D>& x){
	return !x;
}
template <class T, class D>
constexpr bool operator!=(const bit::unique_allocate<T, D>& x, std::nullptr_t){
	return (bool)x;
}
template <class T, class D>
constexpr bool operator!=(std::nullptr_t, const bit::unique_allocate<T, D>& x){
	return (bool)x;
}
template <class T, class D>
constexpr bool operator<(const bit::unique_allocate<T, D>& x, std::nullptr_t){
	return x.get()<nullptr;
}
template <class T, class D>
constexpr bool operator<(std::nullptr_t, const bit::unique_allocate<T, D>& x){
	return nullptr<x.get();
}
template <class T, class D>
constexpr bool operator>(const bit::unique_allocate<T, D>& x, std::nullptr_t){
	return x.get()>nullptr;
}
template <class T, class D>
constexpr bool operator>(std::nullptr_t, const bit::unique_allocate<T, D>& x){
	return nullptr>x.get();
}
template <class T, class D>
constexpr bool operator<=(const bit::unique_allocate<T, D>& x, std::nullptr_t){
	return x.get()<=nullptr;
}
template <class T, class D>
constexpr bool operator<=(std::nullptr_t, const bit::unique_allocate<T, D>& x){
	return nullptr<=x.get();
}
template <class T, class D>
constexpr bool operator>=(const bit::unique_allocate<T, D>& x, std::nullptr_t){
	return x.get()>=nullptr;
}
template <class T, class D>
constexpr bool operator>=(std::nullptr_t, const bit::unique_allocate<T, D>& x){
	return nullptr>=x.get();
}
template <class T, class D>
constexpr auto operator<=>(const bit::unique_allocate<T, D>& x, std::nullptr_t){
	return std::compare_three_way{}(x.get(), static_cast<typename bit::unique_allocate<T, D>::pointer>(nullptr));
}
template <class T, class... Args>
constexpr std::enable_if_t<!std::is_array_v<T>, bit::unique_allocate<T>> make_unique(Args&&...args){
	return bit::unique_allocate<T>(new T(std::forward<Args>(args)...));
}
template <class T>
constexpr std::enable_if_t<details::is_ubarray<T>, bit::unique_allocate<T>> make_unique(std::size_t g){
	return bit::unique_allocate<T>(new std::remove_extent_t<T>[g]());
}
template <class T, class...Args>
constexpr std::enable_if_t<details::is_barray<T>> make_unique(Args&&...args)=delete;
template <class T>
requires(!std::is_array_v<T>)
constexpr bit::unique_allocate<T> make_unique_for_overwrite(){
	return bit::unique_allocate<T>(new T);
}
template <class T>
requires std::is_unbounded_array_v<T>
constexpr bit::unique_allocate<T> make_unique_for_overwrite(std::size_t size){
	return bit::unique_allocate<T>(new std::remove_extent_t<T>[size]());
}
template <class T, class ...args>
requires std::is_bounded_array_v<T>
void make_unique_for_overwrite(args&&...)=delete;
#endif
