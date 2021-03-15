#pragma once
#include <Windows.h>
#include <string>
#include <set>
#include <vector>

namespace cchips {

    class JumpTarget
    {
    public:
        enum class jmp_type
        {
            // Jump targets discovered in control flow changing instructions.
            JMP_CONTROL_FLOW_BR_FALSE = 0,
            JMP_CONTROL_FLOW_BR_TRUE,
            JMP_CONTROL_FLOW_SWITCH_CASE,
            JMP_CONTROL_FLOW_CALL_TARGET,
            JMP_CONTROL_FLOW_RETURN_TARGET,
            // Jump targets from various other sources.
            JMP_CONFIG,
            JMP_ENTRY_POINT,
            JMP_SELECTED_RANGE_START,
            JMP_IMPORT,
            JMP_DEBUG,
            JMP_SYMBOL,
            JMP_EXPORT,
            JMP_STATIC_CODE,
            JMP_VTABLE,
            JMP_LEFTOVER,
            JMP_NORMAL,
            // Default jump target.
            JMP_UNKNOWN,
        };

        using from_type = typename std::pair<std::uint8_t*, jmp_type>;
        using from_list = typename std::vector<from_type>;

        using iterator = typename from_list::iterator;
        using const_iterator = typename from_list::const_iterator;

        JumpTarget() = default;
        JumpTarget(std::uint8_t* to_addr, jmp_type type, from_type& from) : m_to_address(to_addr), m_type(type) { 
            if(from.first != 0 && from.second < jmp_type::JMP_UNKNOWN)
                m_from_list.emplace_back(from); 
        }

        bool operator<(const JumpTarget& o) const {
            if (getType() == o.getType())
            {
                if (getToAddress() == o.getToAddress())
                {
                        return getFromSize() < o.getFromSize();
                }
                else
                {
                    return getToAddress() < o.getToAddress();
                }
            }
            else
            {
                return getType() < o.getType();
            }
        }

        iterator begin() { return m_from_list.begin(); }
        const_iterator begin() const { return m_from_list.begin(); }
        iterator end() { return m_from_list.end(); }
        const_iterator end() const { return m_from_list.end(); }
        from_type& push(from_type& from) { return m_from_list.emplace_back(from); }

        std::uint8_t* getToAddress() const { return m_to_address; }
        jmp_type getType() const { return m_type; }
        size_t getFromSize() const { return m_from_list.size(); }

        //friend std::ostream& operator<<(std::ostream &out, const JumpTarget& jt) { return {}; }

    private:
        // This address will be tried to be decoded.
        std::uint8_t* m_to_address;
        // The type of jump target - determined by its source.
        jmp_type m_type = jmp_type::JMP_UNKNOWN;
        /// Address from which this jump target was created.
        from_list m_from_list;
    };

    /**
     * Jump target container.
     */
    class JumpTargets
    {
    public:
        auto begin() { return m_data.begin(); }
        auto end() { return m_data.end(); }

        bool empty() { return m_data.empty(); }
        std::size_t size() const { return m_data.size(); }
        void clear() { m_data.clear(); }
        const JumpTarget& top() { return *m_data.begin(); }
        void pop() { m_data.erase(top()); }

        const JumpTarget* push(std::uint8_t* to_addr, JumpTarget::jmp_type type, JumpTarget::from_type& from) { 
            const JumpTarget* target = add(to_addr, from);
            if (target) return target;
            return &(*m_data.emplace(to_addr, type, from).first); 
        }
        const JumpTarget* add(std::uint8_t* to_addr, JumpTarget::from_type& from) {
            for (auto& target : m_data) {
                if (to_addr == target.getToAddress()) {
                    JumpTarget& tar = const_cast<JumpTarget&>(target);
                    tar.push(from);
                    return &target;
                }
            }
            return nullptr;
        }
        std::uint8_t* getAddressAfter(std::uint8_t* address) const {
            std::uint8_t* result = 0;
            for (auto& jmp : m_data) {
                if (jmp.getToAddress() > address) {
                    if (!result) result = jmp.getToAddress();
                    if (jmp.getToAddress() < result)
                        result = jmp.getToAddress();
                }
            }
            return result;
        }

        //friend std::ostream& operator<<(std::ostream &out, const JumpTargets& jts) { return {}; }

    public:
        std::set<JumpTarget> m_data;
    };
} // namespace cchips
