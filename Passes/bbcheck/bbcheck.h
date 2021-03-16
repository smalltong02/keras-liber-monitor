#pragma once
#include <Windows.h>
#include <map>
#include <memory>
#include <set>
#include <vector>
#include "PassManager.h"

namespace cchips {

    class BBCheck : public BasicBlockPass
    {
    public:
        using bberror_code = enum {
            error_success = 0,
            error_block_unknown = -1,
            error_block_invalid = -2,
            error_block_size = -3,
            error_block_start = -4,
            error_block_end = -5,
            error_block_wrong_parent = -6,
            error_block_empty = -7,
            error_block_invalid_insn = -8,
            error_block_link = -9,
        };

#define BBSTATUS_SUCCESS(st) (st >= 0)

        static char ID;
        BBCheck() : BasicBlockPass(ID) {}
        ~BBCheck() {}

        virtual bool runOnBasicBlock(std::shared_ptr<BasicBlock> Block) override;

    private:
        bberror_code run(std::shared_ptr<BasicBlock>& Block);
        static const std::map<bberror_code, std::string> _error_code_descs;
    };
} // namespace cchips
