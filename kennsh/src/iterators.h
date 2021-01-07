#ifndef KENNSH_ITERATORS
#define KENNSH_ITERATORS

#include <deque>

namespace kennsh::iterators {
	template <class Item, class It>
	class take_iterator {
	private:
		long current;
		long count;
		It _begin;
		It _end;
	public:
		take_iterator(It begin, It end, long count);
		take_iterator& operator++();
		take_iterator& begin();
		take_iterator end();
		void operator++(int);
		Item operator*();
		bool operator==(const take_iterator& other) const;
		bool operator==(const It& other) const;
		bool operator!=(const take_iterator& other) const {
			return !this->operator==(other);
		}
		bool operator!=(const It& other) const {
			return !this->operator==(other);
		}
	};

	template <class Item, class It>
	class skip_last_iterator {
	private:
		std::deque<Item> dq;
		long count;
		It _begin;
		It _end;
	public:
		skip_last_iterator(It begin, It end, long count);
		skip_last_iterator& operator++();
		skip_last_iterator& begin();
		skip_last_iterator end();
		void operator++(int);
		Item operator*();
		bool operator==(const skip_last_iterator& other) const;
		bool operator!=(const skip_last_iterator& other) const {
			return !this->operator==(other);
		}
	};
}

#endif