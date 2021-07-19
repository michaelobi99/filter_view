#pragma once
#include <ranges>
#include <algorithm>

template <std::ranges::input_range Range, typename Predicate>
requires std::ranges::view<Range>
class filter_view : public std::ranges::view_interface<filter_view<Range, Predicate>> //curiously recurring template pattern
{
private:
	Range base_container{};
	Predicate unary_predicate;
	mutable std::ranges::iterator_t<Range> beginIter{ std::begin(base_container) };
	mutable std::ranges::iterator_t<Range> endIter{ std::end(base_container) };
	mutable bool isRangeEvaluated{ false };
	//function to filter the range while ensuring lazy evaluation
	void filter_range() {
		if (!isRangeEvaluated) {
			endIter = std::remove_if(std::begin(base_container), std::end(base_container), unary_predicate);
			isRangeEvaluated = true;
		}
	}
public:
	filter_view() = default;
	explicit constexpr filter_view(Range base_container, Predicate unary_predicate)
		: base_container{base_container}
		, unary_predicate{unary_predicate}
		, beginIter{ std::begin(base_container) }
		, endIter{ std::end(base_container) }
	{}

	constexpr Range base() const& {
		return base_container;
	}
	constexpr Range base()&& {
		return std::move(base_container);
	}
	constexpr Predicate const& predicate() const {
		return unary_predicate;
	}
	constexpr auto size() const requires std::ranges::sized_range<const Range>{
		return std::distance(beginIter, endIter);
	}
	constexpr auto size() requires std::ranges::sized_range<Range>{
		return std::distance(beginIter, endIter);
	}
	//begin and end functions to retrieve the begin and end iterators.
	//to make sure that the view is lazy, these iterators should only be evaluated in their first use
	constexpr auto begin() {
		filter_range();
		return beginIter;
	}
	constexpr auto end() {
		filter_range();
		return endIter;
	}
};

//template argument deduction guide
template <typename Range, typename Predicate>
filter_view(Range&& range, Predicate predicate)->filter_view<std::ranges::views::all_t<Range>, Predicate>;

namespace details {
	template <typename Predicate>
	struct filter_view_range_adaptor_closure {
		Predicate predicate;
		constexpr filter_view_range_adaptor_closure(Predicate pred) : predicate(pred) {}
		template <std::ranges::viewable_range Range> 
		constexpr auto operator() (Range&& r) const {
			return filter_view(std::forward<Range>(r), predicate);
		}
	};
	struct filter_view_range_adaptor {
		template <std::ranges::viewable_range Range, typename Predicate>
		constexpr auto operator() (Range&& r, Predicate pred){
			return filter_view(std::forward<Range>(r), pred);
		}
		template <typename Predicate>
		constexpr auto operator() (Predicate pred) {
			return filter_view_range_adaptor_closure(pred);
		}
	};
	template <std::ranges::viewable_range Range, typename Predicate>
	constexpr auto operator| (Range&& r, filter_view_range_adaptor_closure<Predicate> const& a) {
		return a(std::forward<Range>(r));
	}
}

namespace views {
	inline static details::filter_view_range_adaptor filter;
}
