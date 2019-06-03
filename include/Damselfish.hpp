#ifndef DAMSELFISH_HPP
#define DAMSELFISH_HPP

#include <algorithm>
#include <vector>

namespace Damselfish
{
	template <typename T = float, int nItems = 32>
	class Property
	{
	private:
		std::vector<T> items_;
		using size_type = typename std::vector<T>::size_type;
		size_type curItem;
		size_type lastItem;
		size_type count;
	public:
		Property()
		{
			items_.resize(nItems);
			count = 0;
			curItem = 0;
			lastItem = 0;
			items_[curItem] = T();
		}

		bool changed() const
		{
			return items_[lastItem] != items_[curItem];
		}

		T value() noexcept
		{
			return items_[curItem];
		}

		Property<T, nItems> & advance() noexcept
		{
			lastItem = curItem;
			curItem = (curItem + 1) % items_.size();
			count = std::min((count + 1), items_.size());
			return *this;
		}

		T *data() noexcept
		{
			return &items_[curItem];
		}

		const T *cdata() const noexcept
		{
			return &items_[curItem];
		}

		T min() const noexcept
		{
			T x = items_[curItem];
			for (size_t i = 0; i < count; i++)
			{
				size_t j = (curItem + i) % nItems;
				x = std::min(x, items_[j]);
			}
			return x;
		}

		T max() const noexcept
		{
			T x = items_[curItem];
			for (size_t i = 0; i < count; i++)
			{
				size_t j = (curItem + i) % nItems;
				x = std::max(x, items_[j]);
			}
			return x;
		}
		
		T mean() const noexcept
		{
			T x = 0;
			for (size_t i = 0; i < count; i++)
			{
				size_t j = (curItem + i) % nItems;
				x += std::max(x, items_[j]);
			}
			x *= 1.0 / nItems;
			return x;
		}

		Property<T, nItems> & reset() noexcept
		{
			count = 0;
			curItem = 0;
			lastItem = 0;
			items_[curItem] = T();
			return *this;
		}

		// Set the property to the next item
		Property<T, nItems> & operator=(T & x) noexcept {
			advance();
			items_[curItem] = x;
			return *this;
		}

		T & operator*() noexcept {
			return items_[curItem];
		}

		const T & operator*() const noexcept {
			return items_[curItem];
		}
	};
}

#endif
