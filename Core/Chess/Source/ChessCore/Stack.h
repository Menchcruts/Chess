#pragma once
#include <exception>

template<typename Type, size_t Size>
struct Stack
{
	Type arr[Size];
	int top = -1;

	bool IsEmpty() const;
	bool IsFull() const;
	constexpr void Push( Type value );
	constexpr const Type& Peek() const;
	constexpr Type&& Pop();
};

template<typename Type, size_t Size>
inline bool Stack<Type, Size>::IsEmpty() const
{
	return this->top == -1;
}

template<typename Type, size_t Size>
inline bool Stack<Type, Size>::IsFull() const
{
	return this->top == Size - 1;
}

template<typename Type, size_t Size>
inline constexpr void Stack<Type, Size>::Push( Type value )
{
	if ( this->IsFull() )
		throw std::exception( "Stack is full" );
	this->arr[++this->top] = value;
}

template<typename Type, size_t Size>
inline constexpr const Type& Stack<Type, Size>::Peek() const
{
	if ( this->IsEmpty() )
		throw std::exception( "Stack empty" );
	return this->arr[this->top];
}

template<typename Type, size_t Size>
inline constexpr Type&& Stack<Type, Size>::Pop()
{
	Type value = this->arr[this->top];
	--top;
	return std::move( value );
}