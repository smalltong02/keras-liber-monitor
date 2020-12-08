#pragma once
#include <map>
#include <vector>

namespace cchips {

    template<typename KeyT, typename ValueT,
        typename MapType = std::map<KeyT, unsigned>,
        typename VectorType = std::vector<std::pair<KeyT, ValueT>>>
    class MapVector {
    MapType Map;
    VectorType Vector;

    public:
        using value_type = typename VectorType::value_type;
        using size_type = typename VectorType::size_type;

        using iterator = typename VectorType::iterator;
        using const_iterator = typename VectorType::const_iterator;
        using reverse_iterator = typename VectorType::reverse_iterator;
        using const_reverse_iterator = typename VectorType::const_reverse_iterator;

        VectorType takeVector() {
            Map.clear();
            return std::move(Vector);
        }
        size_type size() const { return Vector.size(); }
        void reserve(size_type NumEntries) {
            Map.reserve(NumEntries);
            Vector.reserve(NumEntries);
        }

        iterator begin() { return Vector.begin(); }
        const_iterator begin() const { return Vector.begin(); }
        iterator end() { return Vector.end(); }
        const_iterator end() const { return Vector.end(); }

        reverse_iterator rbegin() { return Vector.rbegin(); }
        const_reverse_iterator rbegin() const { return Vector.rbegin(); }
        reverse_iterator rend() { return Vector.rend(); }
        const_reverse_iterator rend() const { return Vector.rend(); }

        bool empty() const {
            return Vector.empty();
        }

        std::pair<KeyT, ValueT>       &front() { return Vector.front(); }
        const std::pair<KeyT, ValueT> &front() const { return Vector.front(); }
        std::pair<KeyT, ValueT>       &back() { return Vector.back(); }
        const std::pair<KeyT, ValueT> &back()  const { return Vector.back(); }

        void clear() {
            Map.clear();
            Vector.clear();
        }

        void swap(MapVector &RHS) {
            std::swap(Map, RHS.Map);
            std::swap(Vector, RHS.Vector);
        }

        ValueT &operator[](const KeyT &Key) {
            std::pair<KeyT, typename MapType::mapped_type> Pair = std::make_pair(Key, 0);
            std::pair<typename MapType::iterator, bool> Result = Map.insert(Pair);
            auto &I = Result.first->second;
            if (Result.second) {
                Vector.push_back(std::make_pair(Key, ValueT()));
                I = Vector.size() - 1;
            }
            return Vector[I].second;
        }

        ValueT lookup(const KeyT &Key) const {
            typename MapType::const_iterator Pos = Map.find(Key);
            return Pos == Map.end() ? ValueT() : Vector[Pos->second].second;
        }

        std::pair<iterator, bool> insert(const std::pair<KeyT, ValueT> &KV) {
            std::pair<KeyT, typename MapType::mapped_type> Pair = std::make_pair(KV.first, 0);
            std::pair<typename MapType::iterator, bool> Result = Map.insert(Pair);
            auto &I = Result.first->second;
            if (Result.second) {
                Vector.push_back(std::make_pair(KV.first, KV.second));
                I = Vector.size() - 1;
                return std::make_pair(std::prev(end()), true);
            }
            return std::make_pair(begin() + I, false);
        }

        std::pair<iterator, bool> insert(std::pair<KeyT, ValueT> &&KV) {
            std::pair<KeyT, typename MapType::mapped_type> Pair = std::make_pair(KV.first, 0);
            std::pair<typename MapType::iterator, bool> Result = Map.insert(Pair);
            auto &I = Result.first->second;
            if (Result.second) {
                Vector.push_back(std::move(KV));
                I = Vector.size() - 1;
                return std::make_pair(std::prev(end()), true);
            }
            return std::make_pair(begin() + I, false);
        }

        size_type count(const KeyT &Key) const {
            typename MapType::const_iterator Pos = Map.find(Key);
            return Pos == Map.end() ? 0 : 1;
        }

        iterator find(const KeyT &Key) {
            typename MapType::const_iterator Pos = Map.find(Key);
            return Pos == Map.end() ? Vector.end() :
                (Vector.begin() + Pos->second);
        }

        const_iterator find(const KeyT &Key) const {
            typename MapType::const_iterator Pos = Map.find(Key);
            return Pos == Map.end() ? Vector.end() :
                (Vector.begin() + Pos->second);
        }

        void pop_back() {
            typename MapType::iterator Pos = Map.find(Vector.back().first);
            Map.erase(Pos);
            Vector.pop_back();
        }

        typename VectorType::iterator erase(typename VectorType::iterator Iterator) {
            Map.erase(Iterator->first);
            auto Next = Vector.erase(Iterator);
            if (Next == Vector.end())
                return Next;

            size_t Index = Next - Vector.begin();
            for (auto &I : Map) {
                assert(I.second != Index && "Index was already erased!");
                if (I.second > Index)
                    --I.second;
            }
            return Next;
        }
        size_type erase(const KeyT &Key) {
            auto Iterator = find(Key);
            if (Iterator == end())
                return 0;
            erase(Iterator);
            return 1;
        }
        template <class Predicate> void remove_if(Predicate Pred);
    };

    class Function {

    };

    class Module {

    };

} // namespace cchips
