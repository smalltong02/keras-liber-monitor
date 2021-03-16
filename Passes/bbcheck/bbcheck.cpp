#include "bbcheck.h"
#include "PassSupport.h"
#include "utils.h"
#include "LogObject.h"

namespace cchips {

    char BBCheck::ID = 0;

    static RegisterPass<BBCheck> X(
        "BBCheck",
        "BasicBlock check"
    );

    const std::map<BBCheck::bberror_code, std::string> BBCheck::_error_code_descs = {
        {error_success, "block check success."},
        {error_block_unknown, "unknown block."},
        {error_block_invalid, "invalid block."},
        {error_block_size, "wrong block size."},
        {error_block_start, "wrong block start address."},
        {error_block_end, "wrong block end address."},
        {error_block_wrong_parent, "block indicate function is wrong."},
        {error_block_empty, "no insntructions in block."},
        {error_block_invalid_insn, "invalid insn in block."},
        {error_block_link, "block indicate pre or post is wrong."},
    };

    bool BBCheck::runOnBasicBlock(std::shared_ptr<BasicBlock> Block)
    {
        if (!Block)
            return false;
        std::shared_ptr<Function> function = Block->GetParent();
        if (!function)
            return false;
        std::shared_ptr<Module> cur_module = function->GetParent();
        if (!cur_module)
            return false;
        BBCheck::bberror_code code = run(Block);
        if (!BBSTATUS_SUCCESS(code)) {
            std::string error_desc;
            auto& desc = _error_code_descs.find(code);
            if (desc != _error_code_descs.end()) {
                error_desc = desc->second;
            }
            else {
                error_desc = "unknown error.";
            }
            cur_module->GetReportObject().CreateReport(Block, error_desc);
            error_log("bbcheck::runOnBasicBlock block({}), address({}), error desc({})", Block->GetName(), Block->getAddress(), error_desc);
        }
        return true;
    }

    BBCheck::bberror_code BBCheck::run(std::shared_ptr<BasicBlock>& Block)
    {
        if (Block->IsUnknown()) return error_block_unknown;
            if(Block->IsInvalid()) return error_block_invalid;
        if (!Block->getBlockSize()) return error_block_size;
        if (!Block->getAddress()) return error_block_start;
        if (!Block->getEndAddress()) return error_block_end;
        if ((Block->getEndAddress() - Block->getAddress()) != Block->getBlockSize()) return error_block_size;
        if (!Block->GetParent()) return error_block_wrong_parent;
        if (!Block->size()) return error_block_empty;

        size_t block_sizes = 0;
        // check insns and size
        for (auto& insn : *Block) {
            if (!insn) return error_block_invalid_insn;
            block_sizes += insn->size();
        }
        if (block_sizes != Block->getBlockSize())
            return error_block_size;
        // check block link
        switch (Block->GetBlockType())
        {
        case BasicBlock::block_start:
        {
            if (Block->GetNextBlock() == nullptr &&
                Block->GetBranchBlock() == nullptr)
                return error_block_link;
        }
        break;
        case BasicBlock::block_sequnce:
        {
            if (Block->GetNextBlock() == nullptr ||
                Block->GetBranchBlock() != nullptr)
                return error_block_link;
        }
        break;
        case BasicBlock::block_branch:
        case BasicBlock::block_loop:
        {
            if (Block->GetBranchBlock() == nullptr)
                return error_block_link;
        }
        break;
        case BasicBlock::block_linkage:
        {

        }
        break;
        case BasicBlock::block_end:
        {
            if (Block->GetNextBlock() != nullptr ||
                Block->GetBranchBlock() != nullptr)
                return error_block_link;
        }
        break;
        default:
            return error_block_invalid;
        }

        for (auto& pre : Block->GetPreBlockList()) {
            std::shared_ptr<BasicBlock> pre_block = pre.lock();
            if (!pre_block) return error_block_link;
            std::shared_ptr<BasicBlock> pre_next_block = pre_block->GetNextBlock();
            if (Block == pre_next_block) {
                continue;
            }
            std::shared_ptr<BasicBlock> pre_branch_block = pre_block->GetBranchBlock();
            if (Block != pre_branch_block) {
                return error_block_link;
            }
        }

        std::shared_ptr<BasicBlock> next_block = Block->GetNextBlock();
        if (next_block) {
            bool bfind = false;
            for (auto& next_pre : next_block->GetPreBlockList()) {
                if (next_pre.lock() == Block) {
                    bfind = true;
                    break;
                }
            }
            if (!bfind) return error_block_link;
        }
        
        std::shared_ptr<BasicBlock> branch_block = Block->GetBranchBlock();
        if (branch_block) {
            bool bfind = false;
            for (auto& branch_pre : branch_block->GetPreBlockList()) {
                if (branch_pre.lock() == Block) {
                    bfind = true;
                    break;
                }
            }
            if (!bfind) return error_block_link;
        }
        return error_success;
    }
} // namespace cchips
