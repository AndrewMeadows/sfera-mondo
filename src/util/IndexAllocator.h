//
// IndexAllocator.h
//
// Distributed under the Apache License, Version 2.0.
// See the accompanying file LICENSE or
// http://www.apache.org/licenses/LICENSE-2.0.html
//
#pragma once

#include <algorithm>
#include <vector>

// NOTE: Index_t can be: int32_t or int16_t

template <typename Index_t>
class IndexAllocator {
public:
    static constexpr Index_t INVALID_INDEX{-1};

    explicit IndexAllocator(Index_t max_num_elements, Index_t preallocated_num_elements = 0) :
        _maxNumElements(max_num_elements),
        _nextNewIndex(preallocated_num_elements)
    {
    }

    bool check(Index_t idx) const {
        return ((idx >= 0) && (idx < _nextNewIndex));
    }
    Index_t getNumLive() const {
        return _nextNewIndex - (Index_t)(_freeIndices.size());
    }
    Index_t getNumFree() const {
        return (Index_t)_freeIndices.size();
    }
    Index_t getNumAllocated() const {
        return _nextNewIndex;
    }

    Index_t allocate() {
        if (_freeIndices.empty()) {
            Index_t idx = _nextNewIndex;
            if (idx >= _maxNumElements) { return INVALID_INDEX; }
            _nextNewIndex++;
            return idx;
        } else {
            if (!_freeIndicesAreSorted) {
                // when recycling we sort (high to low) and use lowest index
                std::sort(_freeIndices.begin(),
                          _freeIndices.end(),
                          std::greater<Index_t>());
                _freeIndicesAreSorted = true;
            }
            Index_t idx = _freeIndices.back();
            _freeIndices.pop_back();

            // in case an index was double-freed we check for dupe indices and
            // remove them
            while (_freeIndices.size() > 0 && _freeIndices.back() == idx) {
                _freeIndices.pop_back();
            }
            return idx;
        }
    }

    void free(Index_t idx) {
        if (check(idx)) {
            if (_freeIndicesAreSorted && _freeIndices.size() > 0 &&
                (idx < _freeIndices.back())) {
                _freeIndicesAreSorted = false;
            }
            // Note: double-free is allowed, we push_back
            // and deal with dupes later when recycling used indices
            _freeIndices.push_back(idx);
        }
    }

    void clear() {
        _freeIndices.clear();
        _nextNewIndex = 0;
    }

private:
    std::vector<Index_t> _freeIndices;
    Index_t _maxNumElements;
    Index_t _nextNewIndex{0};
    bool _freeIndicesAreSorted{true};
};
