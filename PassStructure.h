#pragma once
#include <Windows.h>
#include <map>
#include <vector>
#include <atomic>
#include "re2/re2.h"
#include "re2/stringpiece.h"
#include "PeInfo.h"
#include "PeFormat.h"
#include "CapstoneImpl.h"
#include "JumpTargets.h"
#include "Abix86.h"
#include "Abix64.h"
#include "PackageWrapper.h"
#include "MetadataTypeImpl.h"
#include "Debugger.h"

namespace cchips {

    class Module;
    class GlobalIFunc;
    class Function;
    class GlobalVariable;

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
                Vector.emplace_back(std::make_pair(Key, ValueT()));
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
                Vector.emplace_back(std::make_pair(KV.first, KV.second));
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
                Vector.emplace_back(std::move(KV));
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

    class Address {
    public:
#define USER_ADDRESS_SPACE_START 0x10000
        Address() = delete;
        ~Address() = delete;

        static bool validAddress(std::uint8_t* address) {
            if (address == nullptr || address == reinterpret_cast<std::uint8_t*>(-1))
                return false;
            if (address < reinterpret_cast <std::uint8_t*>(USER_ADDRESS_SPACE_START))
                return false;
            return true;
        }
    };

    class Variable : public CBaseStruc {
    public:
#define INVALID_VARIABLE_VALUE (BYTE)0xcc
        using variable_type = enum {
            variable_unknown,
            variable_reg,
            variable_mem,
            variable_stack,
        };
        Variable(std::shared_ptr<CBaseStruc> parent, std::string& name, std::uint8_t* address, variable_type type, std::uint8_t bytes, base_type btype);
        ~Variable() = default;
        bool valid() const {
            if (!m_data || !m_parent.lock())
                return false;
            return true;
        }
        std::string GetName() const {
            if (valid())
                return m_data->GetName();
            return {};
        }
        std::uint64_t address() const { return reinterpret_cast<std::uint64_t>(m_address); }
        std::uint64_t GetBaseAddress() const { return address(); }

    private:
        std::uint8_t* m_address;
        variable_type m_type;
        std::weak_ptr<CBaseStruc> m_parent;
        std::shared_ptr<CObObject> m_data;
    };

    class LocalVariable : public Variable {
    public:
#define LOCAL_PREFIX "stack_"
        LocalVariable(std::shared_ptr<Function> func, std::string& name, std::uint8_t* address, variable_type type, std::uint8_t bytes)
            : Variable(std::static_pointer_cast<CBaseStruc>(func), name, address, type, bytes, base_localvariable) {}
        ~LocalVariable() = default;
    private:
    };

    class GlobalVariable : public Variable {
    public:
#define GLOBAL_REG_PREFIX "gal_reg_"
#define GLOBAL_MEM_PREFIX "gal_mem_"
        GlobalVariable(std::shared_ptr<Module> parent, std::string& name, std::uint8_t* address, variable_type type, std::uint8_t bytes) 
            : Variable(std::static_pointer_cast<CBaseStruc>(parent), name, address, type, bytes, base_globalvariable){}
        ~GlobalVariable() = default;
    private:
    };

    class BasicBlock : public CBaseStruc, public std::enable_shared_from_this<BasicBlock> {
    public:
        using block_type = enum {
            block_unknown = 0,
            block_start = 0x1,
            block_sequnce = 0x2,
            block_branch = 0x4,
            block_linkage = 0x8,
            block_loop = 0x10,
            block_end = 0x20,
            block_invalid = 0xffffffff,
        };

        using CapInsnListType = std::vector<std::shared_ptr<CapInsn>>;
        using iterator = CapInsnListType::iterator;
        using const_iterator = CapInsnListType::const_iterator;

        iterator                begin() { return m_capinsns.begin(); }
        const_iterator          begin() const { return m_capinsns.begin(); }
        iterator                end() { return m_capinsns.end(); }
        const_iterator          end() const { return m_capinsns.end(); }
        size_t                  size() const { return m_capinsns.size(); }
        bool                    empty() const { return m_capinsns.empty(); }

        BasicBlock(std::shared_ptr<Function> parent, std::string& block_name, std::uint8_t* block_address, unsigned int block_no, block_type type = block_invalid) : CBaseStruc(base_basicblock) {
            m_parent = parent;
            m_block_address = block_address;
            m_block_name = block_name;
            m_block_no = block_no;
            m_block_type = type;
            m_size = 0;
        }
        ~BasicBlock() = default;
        bool Valid() const {
            if (!m_parent.lock()) return false;
            if (m_block_address == nullptr) return false;
            if (!m_block_name.length()) return false;
            if (m_block_type == block_invalid) return false;
            return true;
        }
        const std::string& GetName() const { return m_block_name; }
        bool setBlockType(block_type type) { 
            if (type == block_invalid) {
                m_block_type = block_invalid;
                return true;
            }
            if (type == block_unknown) {
                m_block_type = block_unknown;
                return true;
            }
            m_block_type = m_block_type | type;
            return true;
        }
        bool clrBlockType(block_type type) {
            m_block_type &= ~type;
            return true;
        }
        std::uint32_t GetBlockType() const { return m_block_type; }
        std::string getBlockType() const {
            switch (m_block_type)
            {
            case block_invalid:
                return "invalid";
            case block_unknown:
                return "unknown";
            default:
                ;
            }
            std::string block_type;
            if (m_block_type & block_start) {
                block_type = "start";
            }
            if (m_block_type & block_sequnce) {
                if (block_type.length()) {
                    block_type += std::string(" | ");
                }
                block_type += "sequnce";
            }
            if (m_block_type & block_branch) {
                if (block_type.length()) {
                    block_type += std::string(" | ");
                }
                block_type += "branch";
            }
            if (m_block_type & block_linkage) {
                if (block_type.length()) {
                    block_type += std::string(" | ");
                }
                block_type += "linkage";
            }
            if (m_block_type & block_loop) {
                if (block_type.length()) {
                    block_type += std::string(" | ");
                }
                block_type += "loop";
            }
            if (m_block_type & block_end) {
                if (block_type.length()) {
                    block_type += std::string(" | ");
                }
                block_type += "end";
            }
            if (!block_type.length()) {
                block_type = "unknown";
            }
            return block_type;
        }
        bool IsInvalid() const { return m_block_type == block_invalid; }
        bool IsUnknown() const { return m_block_type == block_unknown; }
        bool IsStart() const { return (m_block_type & block_start); }
        bool IsSequnce() const { return (m_block_type & block_sequnce); }
        bool IsBranch() const { return (m_block_type & block_branch); }
        bool IsLinkage() const { return (m_block_type & block_linkage); }
        bool IsLoop() const { return (m_block_type & block_loop); }
        bool IsEnd() const { return (m_block_type & block_end); }
        bool isAfterBlock(std::shared_ptr<BasicBlock> block, std::shared_ptr<const BasicBlock> first = nullptr) const;
        std::shared_ptr<CapInsn> getBeginInsn() const { return *m_capinsns.begin(); }
        std::shared_ptr<CapInsn> getEndInsn() const { return *m_capinsns.rbegin(); }
        std::shared_ptr<CapInsn> getInsn(int index) const { 
            if (index > size())
                return nullptr;
            return m_capinsns.at(index);
        }
        unsigned int getBlockNo() const { return m_block_no; }
        std::uint8_t* getAddress() const { return m_block_address; }
        std::uint8_t* getEndAddress() const { return m_block_address + m_size; }
        std::uint64_t GetBaseAddress() const { return reinterpret_cast<std::uint64_t>(getAddress()); }
        std::size_t getBlockSize() const { return m_size; }
        bool AddCapInsn(std::shared_ptr<CapInsn> cap_insn);

        bool addPreBlock(std::shared_ptr<BasicBlock>& pre_block) {
            if (!pre_block) return false;
            pre_block_list.emplace_back(pre_block);
            return true;
        }
        bool addNextBlock(std::shared_ptr<BasicBlock>& block) {
            if (!block) return false;
            if (next_block.lock()) return false;
            next_block = block;
            return true;
        }
        bool addBranchBlock(std::shared_ptr<BasicBlock>& block) {
            if (!block) return false;
            if (branch_block.lock()) return false;
            branch_block = block;
            return true;
        }
        bool contains(std::uint8_t* address) const {
            if (getAddress() <= address && getEndAddress() > address)
                return true;
            return false;
        }
        std::shared_ptr<BasicBlock> splitBasicBlockAtAddress(std::uint8_t* address);
        void dump(RapidValue& json_object, rapidjson::MemoryPoolAllocator<>& allocator, Cfg_view_flags flags = Cfg_view_flags::cfg_simple) const;
        std::shared_ptr<Function> GetParent() { return m_parent.lock(); }
        const std::vector<std::weak_ptr<BasicBlock>>& GetPreBlockList() const { return pre_block_list; }
        std::shared_ptr<BasicBlock> GetNextBlock() const { return next_block.lock(); }
        std::shared_ptr<BasicBlock> GetBranchBlock() const { return branch_block.lock(); }
        std::shared_ptr<CapInsn> GetUnPrefinInsn() const {
            for (auto& insn : m_capinsns) {
                if (insn) {
                    if (!insn->IsPrefin())
                        return insn;
                }
            }
            return nullptr;
        }
        std::shared_ptr<CapInsn> GetUnPostfinInsn() const {
            for (auto& insn : m_capinsns) {
                if (insn) {
                    if (!insn->IsPostfin())
                        return insn;
                }
            }
            return nullptr;
        }
    private:
        unsigned int m_block_no;
        std::uint32_t m_block_type;
        size_t m_size;
        std::string m_block_name;
        std::uint8_t* m_block_address;
        std::weak_ptr<Function> m_parent;
        std::vector<std::shared_ptr<CapInsn>> m_capinsns;
        std::vector<std::weak_ptr<BasicBlock>> pre_block_list;
        std::weak_ptr<BasicBlock> next_block;
        std::weak_ptr<BasicBlock> branch_block;
    };

    class Loop : public CBaseStruc
    {
    public:
        using loop_type = enum {
            loop_unknown,
            loop_simple,
            loop_nested,
        };
        Loop() = delete;
        Loop(loop_type type) : CBaseStruc(base_loop), m_type(type) {}
        loop_type getLoopType() const { return m_type; }
        virtual std::uint64_t GetBaseAddress() const = 0;
        static const std::uint64_t invalid_init_value = -1;
    private:
        loop_type m_type;
    };

    //00562356 BF 05 00 00 00       mov         edi,5  
    //0056235B 0F 1F 44 00 00       nop         dword ptr[eax + eax]
    //00562360 57                   push        edi
    //.....
    //.....
    //005623B9 E8 89 69 11 00       call        _Thrd_sleep(0678D47h)
    //005623BE 83 C4 04             add         esp, 4
    //005623C1 83 EF 01             sub         edi, 1
    //005623C4 75 9A                jne         0562360h
    class SimpleLoop : public Loop,  public std::enable_shared_from_this<Loop> {
    public:
        using invariant_type = std::variant<x86_reg, std::pair<std::uint64_t, size_t>>;
        using stepsize_type = std::variant<std::uint64_t, x86_reg>;
        using value_type = std::uint64_t;
        SimpleLoop() = delete;
        SimpleLoop(const invariant_type& invariant, const stepsize_type& stepsize, std::shared_ptr<BasicBlock> block, std::uint64_t init_val = invalid_init_value) 
            : Loop(Loop::loop_simple), m_stepsize(stepsize), m_invariant(invariant), m_block(block), m_init_value(0){}
        ~SimpleLoop() = default;

        std::uint64_t GetBaseAddress() const { 
            std::shared_ptr<BasicBlock> block = m_block.lock();
            if (!block) return 0;
            return block->GetBaseAddress();
        }
        std::uint64_t getInvariant(std::shared_ptr<Debugger::Modifier> ep) const;
        std::uint64_t getInitValue() const { return invalid_init_value; }
        bool setInvariant(std::shared_ptr<Debugger::Modifier> ep, std::uint64_t value) const;
        std::string getInvariantName() const;
    private:
        invariant_type m_invariant;
        stepsize_type m_stepsize;
        value_type m_init_value;
        std::weak_ptr<BasicBlock> m_block;
    };

    class Function : public CBaseStruc, public std::enable_shared_from_this<Function> {
    private:
#define FUNC_NORMAL_SUB "sub_"
#define BLOCK_NORMAL_LOC "loc_"
#define FIRST_BLOCK_NAME "start"
#define FUNC_MAX_BLOCKS 1000
    public:
        using BasicBlockListType = std::map<PVOID, std::shared_ptr<BasicBlock>>;
        using LoopListType = std::vector<std::shared_ptr<Loop>>;
        using iterator = BasicBlockListType::iterator;
        using const_iterator = BasicBlockListType::const_iterator;

        iterator                begin() { return m_basicblocks.begin(); }
        const_iterator          begin() const { return m_basicblocks.begin(); }
        iterator                end() { return m_basicblocks.end(); }
        const_iterator          end() const { return m_basicblocks.end(); }
        size_t                  size() const { return m_basicblocks.size(); }
        bool                    empty() const { return m_basicblocks.empty(); }

        using func_type = enum {
            func_oep,
            func_normal,
            func_linkage,
            func_export,
            func_max,
        };

        Function() = delete;
        Function(std::shared_ptr<Module> parent, std::string& func_name, unsigned int func_no, std::uint8_t* func_address, func_type type = func_normal) : m_parent(parent), CBaseStruc(base_function), m_func_name(func_name), m_func_address(func_address), m_func_type(type), m_func_no(func_no) {}
        ~Function() = default;
        bool Initialize();
        
        const std::string& GetFuncName() const { return m_func_name; }
        unsigned int getFunctionNo() const { return m_func_no; }
        unsigned long long GetFuncAddress() const { return reinterpret_cast<unsigned long long>(m_func_address); }
        std::uint64_t GetBaseAddress() const { return GetFuncAddress(); }
        std::string GetFuncType() const {
            switch (m_func_type)
            {
            case func_oep:
                return "EntryPoint";
            case func_normal:
                return "Normal";
            case func_linkage:
                return "Linkage";
            case func_export:
                return "Export";
            default:
                ;
            }
            return "unknown";
        }
        void SetFuncType(func_type type) { m_func_type = type; }
        bool IsUnknown() const { if (m_func_type >= func_max) return true; return false; }
        bool IsOep() const { if (m_func_type == func_oep) return true; return false; }
        bool IsNormal() const { if (m_func_type == func_normal) return true; return false; }
        bool IsLinkage() const { if (m_func_type == func_linkage) return true; return false; }
        bool IsExport() const { if (m_func_type == func_export) return true; return false; }
        bool Decode(JumpTargets& jump_targets);
        bool getJumpTarget(JumpTargets& jump_targets, JumpTarget& jt);
        void decodeJumpTarget(JumpTargets& jump_targets, const JumpTarget& jt);
        std::shared_ptr<BasicBlock> CreateBasicBlock(const JumpTarget& jt);
        std::shared_ptr<BasicBlock> getBasicBlockAtAddress(std::uint8_t* address);
        std::shared_ptr<BasicBlock> getBasicBlockAfter(std::uint8_t* address);
        int getCurrentBlockNo() const { return (int)m_basicblocks.size(); }
        bool splitBasicBlockAtAddress(const JumpTarget& jt);
        void linktoFromBasicBlock(std::shared_ptr<BasicBlock> cur_block, const JumpTarget& jt);
        bool linkBasicBlock(std::shared_ptr<BasicBlock>& pre_block, std::shared_ptr<BasicBlock>& post_block, JumpTarget::jmp_type type) {
            if (!pre_block || !post_block) return false;
            if (type == JumpTarget::jmp_type::JMP_CONTROL_FLOW_BR_FALSE) {
                pre_block->addNextBlock(post_block);
                post_block->addPreBlock(pre_block);
                return true;
            }
            else if (type == JumpTarget::jmp_type::JMP_CONTROL_FLOW_BR_TRUE) {
                pre_block->addBranchBlock(post_block);
                post_block->addPreBlock(pre_block);
                return true;
            }
            return false;
        }
        size_t GetPredictSize(const JumpTargets& jump_targets, std::uint8_t* address);
        void processJmpInstruction(std::shared_ptr<BasicBlock> cur_block, std::unique_ptr<CapInsn> cap_insn, std::uint8_t* insn_code, JumpTargets& jump_targets);
        void processLoopInstruction(std::shared_ptr<BasicBlock> cur_block, std::unique_ptr<CapInsn> cap_insn, std::uint8_t* insn_code, JumpTargets& jump_targets);
        void dump(RapidValue& json_object, rapidjson::MemoryPoolAllocator<>& allocator, Cfg_view_flags flags = Cfg_view_flags::cfg_simple) const;
        void viewCFG() {}
        std::shared_ptr<Module> GetParent() { return m_parent.lock(); }
        bool addSimpleLoop(std::shared_ptr<BasicBlock> block, SimpleLoop::invariant_type invariant, SimpleLoop::value_type value, SimpleLoop::stepsize_type stepsize) {
            if (!block) return false;
            std::shared_ptr<SimpleLoop> loop = std::make_shared<SimpleLoop>(invariant, stepsize, std::move(block));
            if (!loop) return false;
            m_loops.emplace_back(std::move(loop));
            return true;
        }
        std::shared_ptr<BasicBlock> GetUnPrefinBlock() const {
            for (auto& bb : m_basicblocks) {
                if (bb.second) {
                    if (!bb.second->IsPrefin())
                        return bb.second;
                }
            }
            return nullptr;
        }
        std::shared_ptr<BasicBlock> GetUnPostfinBlock() const {
            for (auto& bb : m_basicblocks) {
                if (bb.second) {
                    if (!bb.second->IsPostfin())
                        return bb.second;
                }
            }
            return nullptr;
        }
        std::shared_ptr<BasicBlock> GetBasicBlock(std::uint32_t index) const {
            for (auto & bb : m_basicblocks) {
                if (bb.second) {
                    if (bb.second->getBlockNo() == index)
                        return bb.second;
                }
            }
            return nullptr;
        }
        const LoopListType& getLoops() const { return m_loops; }
    private:
        unsigned int m_func_no;
        func_type m_func_type;
        std::string m_func_name;
        std::uint8_t* m_func_address;
        LoopListType m_loops;
        BasicBlockListType m_basicblocks;
        std::weak_ptr<Module> m_parent;
    };

    class GlobalIFunc : public CBaseStruc {
    public:
        using func_struc = struct {
            using func_type = enum {
                func_import = 0,
                func_export,
                func_dynamic,
                func_internal,
                func_dsymbol,
            };
            func_type type;
            unsigned long long ordinal_number;
            std::uint8_t* address;
            std::uint8_t* to_address;
            std::string file_name;
            std::string func_name;
        };

        GlobalIFunc(const std::shared_ptr<Module> parent, std::unique_ptr<func_struc> func_st) : m_parent(parent), CBaseStruc(base_globalifunc), m_func_pointer(std::move(func_st)) {}
        ~GlobalIFunc() = default;
        std::string& GetFileName() const { return m_func_pointer->file_name; }
        std::string& GetFuncName() const { return m_func_pointer->func_name; }
        std::string GetFullName() const { return m_func_pointer->file_name + std::string("!") + m_func_pointer->func_name; }
        func_struc::func_type GetFuncType() const { return m_func_pointer->type; }
        void dump(RapidValue json_object, rapidjson::MemoryPoolAllocator<> allocator, Cfg_view_flags flags = Cfg_view_flags::cfg_simple) const { ; }
        std::shared_ptr<Module> GetParent() { return m_parent.lock(); }
        std::uint64_t GetBaseAddress() const { 
            if(m_func_pointer)
                return reinterpret_cast<std::uint64_t>(m_func_pointer->to_address); 
            return 0;
        }
        bool IsMainfunc(const std::string& funcname) const {
            for (auto& str : _main_func_universal_name) {
                RE2 pattern_main(str, RE2::Quiet);
                if (RE2::FullMatch(funcname, pattern_main)) {
                    return true;
                }
            }
            return false;
        }
    private:
        static const std::vector<std::string> _main_func_universal_name;
        std::unique_ptr<func_struc> m_func_pointer = nullptr;
        std::weak_ptr<Module> m_parent;
    };

    class ControlFlowGraph : public CBaseStruc {
    public:
        using cfg_type = enum {
            cfg_insnflow = 1,
            cfg_blockflow = 2,
            cfg_funcflow = 4,
            cfg_apiflow = 8,
        };
        ControlFlowGraph(std::shared_ptr<Module> parent, cfg_type type) : m_parent(parent), CBaseStruc(base_cfgraph), m_cfg_type(type){}
        ~ControlFlowGraph() {}
        bool AddDatabase(const std::string& cfg_name, const std::vector<std::string>& database) {
            m_cfg_database[cfg_name] = database;
            return true;
        }
        std::uint64_t GetBaseAddress() const { return 0; }
        void dump(RapidValue& json_object, rapidjson::MemoryPoolAllocator<>& allocator, Cfg_view_flags flags = Cfg_view_flags::cfg_simple) const;
    private:
        std::map<std::string, std::vector<std::string>> m_cfg_database;
        cfg_type m_cfg_type = cfg_apiflow;
        std::weak_ptr<Module> m_parent;
    };

    class Module : public CBaseStruc, public std::enable_shared_from_this<Module> {
    public:
        using GLOBAL_SYMBOLTABLE = std::map<PVOID, std::shared_ptr<GlobalVariable>>;
        using FUNCTION_SYMBOLTABLE = std::map<PVOID, std::shared_ptr<Function>>;
        using GLOBALIFUNC_SYMBOLTABLE = std::map<PVOID, std::shared_ptr<GlobalIFunc>>;

        using global_iterator = GLOBAL_SYMBOLTABLE::iterator;
        using const_global_iterator = GLOBAL_SYMBOLTABLE::const_iterator;
        using function_iterator = FUNCTION_SYMBOLTABLE::iterator;
        using const_function_iterator = FUNCTION_SYMBOLTABLE::const_iterator;
        using ifunc_iterator = GLOBALIFUNC_SYMBOLTABLE::iterator;
        using const_ifunc_iterator = GLOBALIFUNC_SYMBOLTABLE::const_iterator;

        function_iterator begin() { return m_function_list.begin(); }
        const_function_iterator begin() const { return m_function_list.begin(); }
        function_iterator end() { return m_function_list.end(); }
        const_function_iterator end() const { return m_function_list.end(); }
        size_t size() const { return m_function_list.size(); }

        class ModuleContext {
        private:
            FileDetector::file_format module_format = FileDetector::format_unknown;
            const std::uint8_t* base_address = nullptr;
            const std::uint8_t* end_address = nullptr;
            std::unique_ptr<PeFormat> pe_format = nullptr;
            std::unique_ptr<PeLib::PeFile> GetPeFile(const std::string& path);
            void InitializePeStructure(std::unique_ptr<PeLib::PeFile> pe_file);
        public:
            ModuleContext(const std::string& path) {
                auto file = GetPeFile(path);
                if (!file) {
                    module_format = FileDetector::format_unknown;
                    base_address = nullptr;
                }
                else {
                    base_address = file->getLoadedBytesData();
                    end_address = base_address + file->getLoadedFileLength();
                    InitializePeStructure(std::move(file));
                }
            }
            ModuleContext(const std::uint8_t* address) { 
                std::unique_ptr<byte_array_buffer> pe_ba_buffer = nullptr;
                std::unique_ptr<std::istream> istream_buffer = nullptr;

                module_format = FileDetector::DetectFileFormat(address);
                if (module_format == FileDetector::format_pe) {
                    size_t size_image = FileDetector::GetSizeOfImage(address);
                    base_address = address;
                    end_address = base_address + size_image;
                    pe_ba_buffer = std::make_unique<byte_array_buffer>(base_address, size_image);
                    if (!pe_ba_buffer) return;
                    istream_buffer = std::make_unique<std::istream>(pe_ba_buffer.get());
                    std::unique_ptr<PeLib::PeFile> file(FileDetector::ReadImageMemory(*istream_buffer));
                    if (!file) {
                        module_format = FileDetector::format_unknown;
                        base_address = nullptr;
                    }
                    else {
                        InitializePeStructure(std::move(file));
                    }
                }
            }
            ~ModuleContext() = default;
            bool Valid() const {
                if (pe_format)
                    return true;
                return false;
            }
            std::uint8_t* GetOEP() const { 
                if (Valid()) {
                    unsigned long long rva = 0;
                    std::uint8_t* ep_address = const_cast<std::uint8_t*>(GetBaseAddress());
                    if (pe_format->getEpAddress(rva) && rva) {
                        uint32_t offset = pe_format->getValidOffsetFromRva((uint32_t)rva);
                        return reinterpret_cast<std::uint8_t*>(ep_address + offset);
                    }
                }
                return nullptr;
            }
            unsigned int GetBits() const { 
                if (Valid()) {
                    return pe_format->getBits();
                }
                return 0; 
            }
            bool isLoadingFile() const { if (pe_format) return pe_format->isLoadingFile(); else return false; }
            bool isLoadingMemory() const { if (pe_format) return pe_format->isLoadingMemory(); else return false; }
            std::uint64_t getValidRvaAddressFromFileAddress(std::uint64_t address) const { if (pe_format) return pe_format->getValidRvaAddressFromFileAddress(address); else return address; }
            std::uint64_t getValidOffsetFromMemRva(std::uint64_t rva) const { if (pe_format) return pe_format->getValidOffsetFromMemRva(rva); else return rva; }
            std::uint64_t getValidOffsetFromRva(std::uint32_t rva) const { if (pe_format) return pe_format->getValidOffsetFromRva(rva); else return rva; }
            const std::unique_ptr<PeFormat>& GetPeFormat() { return pe_format; }
            const std::uint8_t* GetBaseAddress() const { return base_address; }
            const std::uint8_t* GetEndAddress() const { return end_address; }
            const std::unique_ptr<ExportTable>&  getExportTable() const { return pe_format->getExportTable(); }
        };

        class ReportObject {
        public:
            struct report_object {
                CBaseStruc::base_type type;
                std::uint64_t address;
                std::weak_ptr<CBaseStruc> object;
                bool operator<(const report_object& rr) const {
                    if (type == rr.type)
                        return address < rr.address;
                    return type < rr.type;
                }
                bool operator>(const report_object& rr) const {
                    if (type == rr.type)
                        return address > rr.address;
                    return type > rr.type;
                }
                bool operator==(const report_object& rr) const {
                    if (type == rr.type)
                        return address == rr.address;
                    return false;
                }
            };
            using REPORT_TABLE = std::map<report_object, std::string>;
            void CreateReport(std::shared_ptr<CBaseStruc> object, std::string& desc);
            void dump(RapidValue& json_object, rapidjson::MemoryPoolAllocator<>& allocator, Cfg_view_flags flags = Cfg_view_flags::cfg_simple) const;
            void GenerateJsonReport(int count, std::shared_ptr<CBaseStruc> object, std::string desc, RapidValue& json_object, rapidjson::MemoryPoolAllocator<>& allocator, Cfg_view_flags flags = Cfg_view_flags::cfg_simple) const;
        private:
            REPORT_TABLE m_report_list;
        };

        Module() : CBaseStruc(base_module) {}
        ~Module() {
            m_module_context = nullptr;
            m_globalvariable_list.clear();
            m_function_list.clear();
            m_globalifunc_list.clear();
            ClrInsnCounts();
        }
        void SetPrecacheAddress(std::uint8_t* address) { m_precache_address = address; }
        void SetPrecachePath(const std::string& path) { m_precache_path = path; }
        bool Initialize();
        void SetModuleName(std::string& name) { m_name = name; }
        const std::string& GetModuleName() const { return m_name; }
        const std::string GetArchitecture() const { 
            if (Valid()) {
                switch (m_module_context->GetBits())
                {
                case 32:
                    return "x86";
                case 64:
                    return "x64";
                default:
                    ;
                }
            }
            return "unknown";
        }
        const std::unique_ptr<ModuleContext>& GetContext() const { return m_module_context; }
        void dump(const std::string& output_file, Cfg_view_flags flags = Cfg_view_flags::cfg_simple) const;
        void dump(const std::unique_ptr<cchips::CRapidJsonWrapper>& document, Cfg_view_flags flags = Cfg_view_flags::cfg_simple) const;
        bool Valid() const {
            if (m_module_context && m_module_context->Valid())
                return true;
            return false;
        }
        bool InitializeAbi();
        bool InitializeCommonRegister();
        bool InitializeGlobalIFunction();
        bool GetLinkageFuncName(std::string& func_name, std::uint8_t* func_address) const;
        bool AddFunction(std::string& func_name, std::uint8_t* func_address);
        std::shared_ptr<Function> GetFunction(std::uint8_t* func_address) const {
            auto it = m_function_list.find(func_address);
            if (it == m_function_list.end())
                return nullptr;
            return it->second;
        }
        std::shared_ptr<Function> GetFunction(std::uint32_t index) const {
            for (auto& func : m_function_list) {
                if (func.second && func.second->getFunctionNo() == index) {
                    return func.second;
                }
            }
            return {};
        }
        std::shared_ptr<Function> GetUnPrefinFunc() const {
            for (auto& func : m_function_list) {
                if (func.second) {
                    if (!func.second->IsPrefin())
                        return func.second;
                }
            }
            return nullptr;
        }
        std::shared_ptr<Function> GetUnPostfinFunc() const {
            for (auto& func : m_function_list) {
                if (func.second) {
                    if (!func.second->IsPostfin())
                        return func.second;
                }
            }
            return nullptr;
        }
        std::uint64_t GetBaseAddress() const { 
            if(m_module_context)
                return reinterpret_cast<std::uint64_t>(m_module_context->GetBaseAddress());
            return 0;
        }
        std::shared_ptr<Abi> getAbi() const { return m_abi; }
        std::shared_ptr<CBaseStruc> GetBaseObjectAtAddress(std::uint8_t* address) const;
        bool AddGlobalIFunc(std::unique_ptr<GlobalIFunc::func_struc> func_st);
        const GLOBALIFUNC_SYMBOLTABLE& GetGlobalIFuncs() const { return m_globalifunc_list; }
        ReportObject& GetReportObject() { return m_report_object; }
        std::uint64_t GetInsnCounts() const { return m_insn_counts.load(); }
        void IncInsnCounts() { m_insn_counts.fetch_add(1); }
        void DecInsnCounts() { m_insn_counts.fetch_sub(1); }
        void ClrInsnCounts() { m_insn_counts.exchange(0); }
        bool AddDatabaseToCfgraph(const std::string& cfg_name, const std::vector<std::string>& database) {
            if (!m_cfgraph)
                return false;
            return m_cfgraph->AddDatabase(cfg_name, database);
        }
    private:
        std::string m_name;
        std::uint8_t* m_precache_address = 0;
        std::string m_precache_path;
        std::shared_ptr<Abi> m_abi = nullptr;
        std::unique_ptr<ModuleContext> m_module_context = nullptr;
        GLOBAL_SYMBOLTABLE m_globalvariable_list;
        FUNCTION_SYMBOLTABLE m_function_list;
        GLOBALIFUNC_SYMBOLTABLE m_globalifunc_list;
        std::unique_ptr<ControlFlowGraph> m_cfgraph;
        ReportObject m_report_object;
        static std::atomic_uint64_t m_insn_counts;
    };
} // namespace cchips
