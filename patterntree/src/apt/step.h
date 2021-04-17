#pragma once

#include <functional>
#include <memory>
#include <vector>
#include <set>
#include <unordered_map>
#include <optional>

#include "patterns/pattern.h"
#include "patterns/pattern_split.h"
#include "cluster/team.h"

namespace PatternTree {

class APT;

class Step {

size_t index_;
std::vector<std::shared_ptr<IPattern>> patterns_;
std::unordered_multimap<const IPattern*, std::unique_ptr<PatternSplit>> splits_;

std::unordered_map<const PatternSplit*, std::shared_ptr<Team>> assignment_;
std::unordered_multimap<const Team*, const PatternSplit*> reverse_assigment_;

void add_pattern(std::unique_ptr<IPattern> patterns);

public:
    friend class APT;

    Step(std::vector<std::unique_ptr<IPattern>>& patterns, size_t index);
    Step(std::unique_ptr<IPattern> patterns, size_t index);

    struct Iterator 
	{
		using iterator_category = std::input_iterator_tag;
		using difference_type   = std::ptrdiff_t;
		using value_type        = IPattern;
		using pointer           = IPattern*;
		using reference         = IPattern&;

		Iterator(std::vector<std::shared_ptr<IPattern>>::iterator iter)
		: iter_(iter)
		{}

		reference operator*() const { return *(iter_->get()); }
		pointer operator->() { return iter_->get(); }

		Iterator& operator++()
		{ 
			iter_++;
			return *this;
		}  

		Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }

		friend bool operator== (const Iterator& a, const Iterator& b) { return a.iter_ == b.iter_; };
		friend bool operator!= (const Iterator& a, const Iterator& b) { return a.iter_ != b.iter_; };

		private:
			std::vector<std::shared_ptr<IPattern>>::iterator iter_;
	};

	Step::Iterator begin() { return Iterator( this->patterns_.begin() ); }
    Step::Iterator end()   { return Iterator( this->patterns_.end() ); }

    size_t size() const;
    size_t index() const;
    bool complete() const;
    std::set<std::shared_ptr<Team>> teams() const;
	std::set<std::shared_ptr<Team>> teams(const IPattern& pattern) const;

	std::vector<std::reference_wrapper<const PatternSplit>> splits() const;
	std::vector<std::reference_wrapper<const PatternSplit>> splits(const IPattern& pattern) const;
	std::vector<std::reference_wrapper<const PatternSplit>> split(const IPattern& pattern, std::vector<size_t> sizes);
	std::vector<std::reference_wrapper<const PatternSplit>> split(const IPattern& pattern, size_t n);

    void assign(const IPattern& pattern, std::shared_ptr<Team> team);
    void assign(const PatternSplit& split, std::shared_ptr<Team> team);
    void free(const PatternSplit& split);
    void free(const IPattern& pattern);

    std::vector<std::reference_wrapper<const PatternSplit>> assigned(const Team& team) const;
    std::optional<std::shared_ptr<Team>> assigned(const PatternSplit& split) const;

    bool happensBefore(const IPattern& pattern);
};

}