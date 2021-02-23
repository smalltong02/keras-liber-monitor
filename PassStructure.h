#pragma once
#include <Windows.h>
#include <map>
#include <vector>
#include "PeInfo.h"
#include "CapstoneImpl.h"

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

    class GlobalVariable : public CBaseStruc {
    public:
        GlobalVariable() : CBaseStruc(base_globalvariable) {}
        ~GlobalVariable() = default;
    private:
        std::weak_ptr<Module> m_parent;
    };

    class BasicBlock : public CBaseStruc, public std::enable_shared_from_this<BasicBlock> {
    public:
        using block_type = enum {
            block_invalid = 0,
            block_start,
            block_branch,
            block_loop,
            block_end,
        };
        BasicBlock(std::shared_ptr<Function> parent, std::string& block_name, std::uint8_t* block_address, unsigned int block_no, block_type type = block_invalid) : CBaseStruc(base_basicblock) {
            m_parent = parent;
            m_block_address = block_address;
            m_block_name = block_name;
            m_block_no = block_no;
            m_block_type = type;
        }
        ~BasicBlock() = default;
        bool Valid() const {
            if (!m_parent.lock()) return false;
            if (m_block_address == nullptr) return false;
            if (!m_block_name.length()) return false;
            if (m_block_type == block_invalid || m_block_type > block_end) return false;
            return true;
        }

        bool AddCapInsn(std::shared_ptr<CapInsn> cap_insn) {
            if (!cap_insn) return false;
            if (!cap_insn->valid()) return false;
            cap_insn->SetParent(shared_from_this());
            m_capinsn_list.push_back(std::move(cap_insn));
            return true;
        }

        std::shared_ptr<BasicBlock> SplitBasicBlock() {}
    private:
        unsigned int m_block_no;
        block_type m_block_type;
        std::string m_block_name;
        std::uint8_t* m_block_address;
        std::weak_ptr<Function> m_parent;
        std::vector<std::shared_ptr<CapInsn>> m_capinsn_list;
    };

    class Function : public CBaseStruc, public std::enable_shared_from_this<Function> {
    private:
#define FUNC_NORMAL_SUB "sub_"
#define BLOCK_NORMAL_LOC "loc_"
#define FIRST_BLOCK_NAME "start"
#define FUNC_MAX_BLOCKS 1000
    public:
        using func_type = enum {
            func_oep,
            func_normal,
        };

        Function() = delete;
        Function(std::shared_ptr<Module> parent, std::string& func_name, std::uint8_t* func_address, func_type type = func_normal) : m_parent(parent), CBaseStruc(base_function), m_func_name(func_name), m_func_address(func_address), m_func_type(type){}
        ~Function() = default;
        bool Initialize();
        
    private:
        unsigned int m_blocks_count = 0;
        func_type m_func_type;
        std::string m_func_name;
        std::uint8_t* m_func_address;
        std::vector<std::shared_ptr<BasicBlock>> m_basicblock_list;
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
            union {
                WORD hint;
                WORD ordinal;
            };
            func_type type;
            union {
                DWORD_PTR original_first_thunk;
                DWORD_PTR addroffunc;
            };
            DWORD_PTR first_thunk;
            std::string file_name;
            std::string func_name;
        };

        GlobalIFunc(const std::shared_ptr<Module> parent, std::unique_ptr<func_struc> func_st) : m_parent(parent), CBaseStruc(base_globalifunc), m_func_pointer(std::move(func_st)) {}
        ~GlobalIFunc() = default;
        std::string& GetFileName() const { return m_func_pointer->file_name; }
        std::string& GetFuncName() const { return m_func_pointer->func_name; }
        std::string GetFullName() const { return m_func_pointer->file_name + std::string("!") + m_func_pointer->func_name; }
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

        class ModuleContext {
        private:
            FileDetector::file_format module_format = FileDetector::format_unknown;
            const std::uint8_t* base_address = nullptr;
            const std::uint8_t* end_address = nullptr;
            const std::uint8_t* oep = nullptr;
            std::unique_ptr<byte_array_buffer> pe_ba_buffer = nullptr;
            std::unique_ptr<std::istream> istream_buffer = nullptr;
            std::unique_ptr<PeLib::PeFile> pe_file = nullptr;

            void InitializePeStructure();
        public:
            ModuleContext(const std::uint8_t* address) { 
                module_format = FileDetector::DetectFileFormat(address);
                if (module_format == FileDetector::format_pe) {
                    oep = FileDetector::GetEntryPoint(address);
                    if (oep) {
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
                            oep = nullptr;
                        }
                        else {
                            pe_file = std::move(file);
                            InitializePeStructure();
                        }
                    }
                }
            }
            ~ModuleContext() = default;
            bool Valid() const {
                if (base_address && oep && pe_file && module_format == FileDetector::format_pe)
                    return true;
                return false;
            }
            std::uint8_t* GetOEP() const { return const_cast<std::uint8_t*>(oep); }
            const std::unique_ptr<PeLib::PeFile>& GetPE() const { return pe_file; }
            unsigned int GetBits() const { if (pe_file) return pe_file->getBits(); return 0; }
            const std::uint8_t* GetBaseAddress() const { return base_address; }
            const std::uint8_t* GetEndAddress() const { return end_address; }
        };

        Module() : CBaseStruc(base_module) {}
        ~Module() = default;
        bool Initialize(const std::uint8_t* address) {
            m_module_context = std::make_unique<ModuleContext>(address);
            if (m_module_context && m_module_context->Valid()) {
                InitializeGlobalIFunction();
                if (AddFunction(std::string("entry_point"), m_module_context->GetOEP())) {
                    return true;
                }
            }
            m_module_context = nullptr;
            m_globalvariable_list.clear();
            m_function_list.clear();
            m_globalifunc_list.clear();
            return false;
        }

        const std::unique_ptr<ModuleContext>& GetContext() const { return m_module_context; }

        bool Valid() const {
            if (m_module_context && m_module_context->Valid())
                return true;
            return false;
        }
        bool InitializeGlobalIFunction();
        bool AddFunction(std::string& func_name, std::uint8_t* func_address);
        bool AddGlobalIFunc(std::unique_ptr<GlobalIFunc::func_struc> func_st);
        const GLOBALIFUNC_SYMBOLTABLE& GetGlobalIFuncs() const { return m_globalifunc_list; }
    private:
        std::unique_ptr<ModuleContext> m_module_context = nullptr;
        GLOBAL_SYMBOLTABLE m_globalvariable_list;
        FUNCTION_SYMBOLTABLE m_function_list;
        GLOBALIFUNC_SYMBOLTABLE m_globalifunc_list;
    };
} // namespace cchips
