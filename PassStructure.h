#pragma once
#include <Windows.h>
#include <map>
#include <vector>
#include "PeInfo.h"
#include "PeFormat.h"
#include "CapstoneImpl.h"
#include "JumpTargets.h"
#include "Abix86.h"
#include "Abix64.h"
#include "PackageWrapper.h"

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
        Address() = delete;
        ~Address() = delete;

        static bool validAddress(std::uint8_t* address) {
            if (address == nullptr || address == reinterpret_cast<std::uint8_t*>(-1))
                return false;
            return true;
        }
    };

    class GlobalVariable : public CBaseStruc {
    public:
        GlobalVariable() : CBaseStruc(base_globalvariable) {}
        ~GlobalVariable() = default;
        const std::string& GetName() const { return m_name; }
    private:
        std::string m_name;
        std::weak_ptr<Module> m_parent;
    };

    class BasicBlock : public CBaseStruc, public std::enable_shared_from_this<BasicBlock> {
    public:
        using block_type = enum {
            block_invalid = 0,
            block_start,
            block_sequnce,
            block_branch,
            block_linkage,
            block_loop,
            block_end,
            block_unknown,
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
            if (m_block_type == block_invalid || m_block_type > block_unknown) return false;
            return true;
        }
        const std::string& GetName() const { return m_block_name; }
        bool setBlockType(block_type type) { 
            if (type == block_invalid || type > block_unknown)
                return false;
            m_block_type = type; 
            return true;
        }
        std::string getBlockType() const {
            switch (m_block_type)
            {
            case block_invalid:
                return "invalid";
            case block_start:
                return "start";
            case block_sequnce:
                return "sequnce";
            case block_branch:
                return "branch";
            case block_linkage:
                return "linkage";
            case block_loop:
                return "loop";
            case block_end:
                return "end";
            default:
                ;
            }
            return "unknown";
        }
        bool IsInvalid() const { return m_block_type == block_invalid; }
        bool IsUnknown() const { return m_block_type == block_unknown; }
        bool IsStart() const { return m_block_type == block_start; }
        bool IsSequnce() const { return m_block_type == block_sequnce; }
        bool IsBranch() const { return m_block_type == block_branch; }
        bool IsLinkage() const { return m_block_type == block_linkage; }
        bool IsLoop() const { return m_block_type == block_loop; }
        bool IsEnd() const { return m_block_type == block_end; }
        std::shared_ptr<CapInsn> getBeginInsn() const { return *m_capinsns.begin(); }
        std::shared_ptr<CapInsn> getEndInsn() const { return *m_capinsns.rbegin(); }
        unsigned int getBlockNo() const { return m_block_no; }
        std::uint8_t* getAddress() const { return m_block_address; }
        std::uint8_t* getEndAddress() const { return m_block_address + m_size; }
        std::size_t getBlockSize() const { return m_size; }
        bool AddCapInsn(std::shared_ptr<CapInsn> cap_insn) {
            if (!cap_insn) return false;
            if (!cap_insn->valid()) return false;
            m_size += cap_insn->size();
            cap_insn->SetParent(shared_from_this());
            m_capinsns.emplace_back(cap_insn);
            return true;
        }

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
    private:
        unsigned int m_block_no;
        block_type m_block_type;
        size_t m_size;
        std::string m_block_name;
        std::uint8_t* m_block_address;
        std::weak_ptr<Function> m_parent;
        std::vector<std::shared_ptr<CapInsn>> m_capinsns;
        std::vector<std::weak_ptr<BasicBlock>> pre_block_list;
        std::weak_ptr<BasicBlock> next_block;
        std::weak_ptr<BasicBlock> branch_block;
    };

    class Function : public CBaseStruc, public std::enable_shared_from_this<Function> {
    private:
#define FUNC_NORMAL_SUB "sub_"
#define BLOCK_NORMAL_LOC "loc_"
#define FIRST_BLOCK_NAME "start"
#define FUNC_MAX_BLOCKS 1000
    public:
        using BasicBlockListType = std::map<PVOID, std::shared_ptr<BasicBlock>>;
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
        };

        Function() = delete;
        Function(std::shared_ptr<Module> parent, std::string& func_name, std::uint8_t* func_address, func_type type = func_normal) : m_parent(parent), CBaseStruc(base_function), m_func_name(func_name), m_func_address(func_address), m_func_type(type){}
        ~Function() = default;
        bool Initialize();
        
        const std::string& GetFuncName() const { return m_func_name; }
        unsigned long long GetFuncAddress() const { return reinterpret_cast<unsigned long long>(m_func_address); }
        std::string GetFuncType() const {
            switch (m_func_type)
            {
            case func_oep:
                return "EntryPoint";
            case func_normal:
                return "Normal";
            case func_linkage:
                return "Linkage";
            default:
                ;
            }
            return "unknown";
        }
        bool Decode(JumpTargets& jump_targets);
        bool getJumpTarget(JumpTargets& jump_targets, JumpTarget& jt);
        void decodeJumpTarget(JumpTargets& jump_targets, const JumpTarget& jt);
        std::shared_ptr<BasicBlock> CreateBasicBlock(const JumpTarget& jt);
        std::shared_ptr<BasicBlock> getBasicBlockAtAddress(std::uint8_t* address);
        std::shared_ptr<BasicBlock> getBasicBlockAfter(std::uint8_t* address);
        int getCurrentBlockNo() const { return m_basicblocks.size(); }
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
    private:
        func_type m_func_type;
        std::string m_func_name;
        std::uint8_t* m_func_address;
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
        void dump(RapidValue json_object, rapidjson::MemoryPoolAllocator<> allocator, Cfg_view_flags flags = Cfg_view_flags::cfg_simple) const { ; }
        std::shared_ptr<Module> GetParent() { return m_parent.lock(); }
    private:
        std::unique_ptr<func_struc> m_func_pointer = nullptr;
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

            void InitializePeStructure(std::unique_ptr<PeLib::PeFile> pe_file);
        public:
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
                    unsigned long long offset = 0;
                    std::uint8_t* ep_address = const_cast<std::uint8_t*>(GetBaseAddress());
                    if (pe_format->getEpAddress(offset) && offset) {
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
            const std::unique_ptr<PeFormat>& GetPeFormat() { return pe_format; }
            const std::uint8_t* GetBaseAddress() const { return base_address; }
            const std::uint8_t* GetEndAddress() const { return end_address; }
        };

        Module() : CBaseStruc(base_module) {}
        ~Module() {
            m_module_context = nullptr;
            m_globalvariable_list.clear();
            m_function_list.clear();
            m_globalifunc_list.clear();
        }
        void SetPrecacheAddress(std::uint8_t* address) { m_precache_address = address; }
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
        bool Valid() const {
            if (m_module_context && m_module_context->Valid())
                return true;
            return false;
        }
        bool InitializeAbi();
        bool InitializeGlobalIFunction();
        bool GetLinkageFuncName(std::string& func_name, std::uint8_t* func_address) const;
        bool AddFunction(std::string& func_name, std::uint8_t* func_address);
        std::shared_ptr<Function> GetFunction(std::uint8_t* func_address) const {
            for (auto& func : m_function_list) {
                if (func.first == func_address) {
                    return func.second;
                }
            }
            return {};
        }
        std::shared_ptr<CBaseStruc> GetBaseObjectAtAddress(std::uint8_t* address) const;
        bool AddGlobalIFunc(std::unique_ptr<GlobalIFunc::func_struc> func_st);
        const GLOBALIFUNC_SYMBOLTABLE& GetGlobalIFuncs() const { return m_globalifunc_list; }
    private:
        std::string m_name;
        std::uint8_t* m_precache_address = 0;
        std::unique_ptr<Abi> m_abi = nullptr;
        std::unique_ptr<ModuleContext> m_module_context = nullptr;
        GLOBAL_SYMBOLTABLE m_globalvariable_list;
        FUNCTION_SYMBOLTABLE m_function_list;
        GLOBALIFUNC_SYMBOLTABLE m_globalifunc_list;
    };
} // namespace cchips
