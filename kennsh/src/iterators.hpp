#ifndef KENNSH_ITERATORS_HPP
#define KENNSH_ITERATORS_HPP

#include "iterators.h"

namespace kennsh::iterators {
	template <class Item, class It>
	take_iterator<Item, It>::take_iterator(It begin, It end, long count) :
		current(0),
		count(count),
		_begin(begin),
		_end(end) {

	}

	template <class Item, class It>
	take_iterator<Item, It>& take_iterator<Item, It>::operator++() {
		if (this->current < this->count) {
			this->current++;
			this->_begin++;
		}
		return *this;
	}

	template <class Item, class It>
	void take_iterator<Item, It>::operator++(int) {
		this->operator++();
	}

	template <class Item, class It>
	take_iterator<Item, It>& take_iterator<Item, It>::begin() {
		return *this;
	}

	template <class Item, class It>
	take_iterator<Item, It> take_iterator<Item, It>::end() {
		auto n = *this;
		n.current = n.count;
		n._begin = n._end;
		return n;
	}

	template <class Item, class It>
	Item take_iterator<Item, It>::operator*() {
		return this->_begin.operator*();
	}

	template <class Item, class It>
	bool take_iterator<Item, It>::operator==(const take_iterator<Item, It>& other) const {
		if (this->current == this->count && other.current == other.count) {
			return true;
		}
		return this->_begin == other._begin;
	}

	template <class Item, class It>
	bool take_iterator<Item, It>::operator==(const It& other) const {
		return this->_begin == other;
	}
	

	template <class Item, class It>
	skip_last_iterator<Item, It>::skip_last_iterator(It begin, It end, long count) :
		count(count),
		_begin(begin),
		_end(end) {
		this->operator++();
	}

	template <class Item, class It>
	skip_last_iterator<Item, It>& skip_last_iterator<Item, It>::operator++() {
		if (this->dq.size() > 0) {
			this->dq.pop_front();
		}
		while (this->_begin != this->_end && this->dq.size() <= this->count) {
			this->dq.push_back(this->_begin.operator*());
			this->_begin++;
		}
		return *this;
	}

	template <class Item, class It>
	void skip_last_iterator<Item, It>::operator++(int) {
		this->operator++();
	}

	template <class Item, class It>
	skip_last_iterator<Item, It>& skip_last_iterator<Item, It>::begin() {
		return *this;
	}

	template <class Item, class It>
	skip_last_iterator<Item, It> skip_last_iterator<Item, It>::end() {
		auto n = *this;
		n._begin = n._end;
		return n;
	}

	template <class Item, class It>
	Item skip_last_iterator<Item, It>::operator*() {
		return this->dq.front();
	}

	template <class Item, class It>
	bool skip_last_iterator<Item, It>::operator==(const skip_last_iterator<Item, It>& other) const {
		if (this->_begin == this->_end && other._begin == other._end) {
			return true;
		}
		return this->_begin == other._begin;
	}

}

#endif