#include "loopinit.h"
#include "PassSupport.h"

namespace cchips {

    char LoopInit::ID = 0;

    static RegisterPass<LoopInit> X(
        "LoopInit",
        "Loop initializing",
        PassInfo::passreg_post
    );

    bool LoopInit::runOnFunction(std::shared_ptr<Function> Func)
    {
        if (!Func) return false;
        if (Func->getCurrentBlockNo() == 0)
            return false;
        if (!Func->GetParent())
            return false;
        _abi = Func->GetParent()->getAbi();
        if (!_abi)
            return false;
        return run(Func);
    }

    bool LoopInit::run(std::shared_ptr<Function>& Func)
    {
        for (auto& bb : *Func) {
            if (!bb.second->IsBranch())
                continue;
            auto b_ptr = bb.second;
            if (!b_ptr) return false;
            if (!b_ptr->size()) return false;
            auto i_ptr = b_ptr->getEndInsn();
            if (GetCapstoneImplment().InCondBranchGroup(*i_ptr) || GetCapstoneImplment().InLoopGroup(*i_ptr)) {
                if (isSimpleLoop(b_ptr)) {
                    continue;
                }
            }
        }
        return true;
    }

    bool LoopInit::isInvariantInsn(CapInsn& insn, int eflags)
    {
        if (!insn.self()) return false;
        if (!insn.self()->detail) return false;
        cs_x86* x86 = &insn.self()->detail->x86;
        auto flag = x86->eflags;
        if (flag & eflags)
            return true;
        return false;
    }

    std::any LoopInit::GetInvariantContext(CapInsn& insn)
    {
        if (!insn.self()) return {};
        if (!insn.self()->detail) return {};
        cs_x86* x86 = &insn.self()->detail->x86;
        if (insn.GetInsnId() == X86_INS_SUB ||
            insn.GetInsnId() == X86_INS_ADD) {
            if (x86->operands[0].type == X86_OP_REG &&
                x86->operands[1].type == X86_OP_IMM) {
                x86_reg invariant = x86->operands[0].reg;
                std::uint64_t stepsize = x86->operands[1].imm;
                return invariant_context_type(invariant, stepsize);
            }
            if(x86->operands[0].type == X86_OP_REG &&
                x86->operands[1].type == X86_OP_REG) {
                x86_reg invariant = x86->operands[0].reg;
                //x86_reg stepsize = x86->operands[1].reg;
                std::uint64_t stepsize = Loop::invalid_init_value;
                return invariant_context_type(invariant, stepsize);
            }
            if (x86->operands[0].type == X86_OP_REG &&
                x86->operands[1].type == X86_OP_MEM) {
                x86_reg invariant = x86->operands[0].reg;
                std::uint64_t stepsize = Loop::invalid_init_value;
                return invariant_context_type(invariant, stepsize);
            }
            if (x86->operands[0].type == X86_OP_MEM &&
                x86->operands[1].type == X86_OP_REG) {
            }
        }
        else if (insn.GetInsnId() == X86_INS_INC ||
                 insn.GetInsnId() == X86_INS_DEC) {
            std::uint64_t stepsize = 1;
            if (x86->operands[0].type == X86_OP_REG) {
                x86_reg invariant = x86->operands[0].reg;
                return invariant_context_type(invariant, stepsize);
            }
            if (x86->operands[0].type == X86_OP_MEM) {

            }
        }
        else if (insn.GetInsnId() == X86_INS_CMP) {
            if (x86->operands[0].type == X86_OP_REG) {
                x86_reg invariant = x86->operands[0].reg;
                std::uint64_t stepsize = Loop::invalid_init_value;
                return invariant_context_type(invariant, stepsize);
            }
        }
        else if (insn.GetInsnId() == X86_INS_TEST) {
            if (x86->operands[0].type == X86_OP_REG) {
                x86_reg invariant = x86->operands[0].reg;
                std::uint64_t stepsize = Loop::invalid_init_value;
                return invariant_context_type(invariant, stepsize);
            }
        }
        return {};
    }

    SimpleLoop::value_type LoopInit::GetInvariantValue(SimpleLoop::invariant_type invariant, std::shared_ptr<BasicBlock> block)
    {
        if (invariant.index() == 0) {
            x86_reg reg_invar = std::get<x86_reg>(invariant);
            std::shared_ptr<CapInsn> insn = nullptr;
            for (int count = block->size() - 1; count >= 0; count--) {
                insn = block->getInsn(count);
                if (!insn) continue;
                if (!insn->self()) continue;
                if (!insn->self()->detail) continue;
                cs_x86* x86 = &insn->self()->detail->x86;
                if (x86->op_count == 2) {
                    if (x86->operands[0].type == X86_OP_REG &&
                        x86->operands[0].reg == reg_invar) {

                    }
                }
            }
        }
        else if (invariant.index() == 1) {

        }
        return Loop::invalid_init_value;
    }

    bool LoopInit::simulateTraceInvariant(std::shared_ptr<BasicBlock> block, SimpleLoop::invariant_type invariant)
    {
        if (!block) return false;
        if (invariant.index() != 0) return false;
        x86_reg reg_invar = std::get<x86_reg>(invariant);
        std::string reg_name = GetCapstoneImplment().GetRegName(reg_invar);
        if (!reg_name.length()) return false;
        std::stringstream ss;
        ss << LOCAL_PREFIX << reg_name;
        std::shared_ptr<LocalVariable> local_var = std::make_shared<LocalVariable>(nullptr, ss.str(), nullptr, Variable::variable_reg, _abi->getRegSize(reg_invar));
        if (!local_var) return false;
        for (auto& insn : *block) {
            if (!insn) continue;
            if (!insn->self()) continue;
            if (!insn->self()->detail) continue;
            cs_detail* detail = insn->self()->detail;
            cs_x86* x86 = &insn->self()->detail->x86;
            for (int count = 0; count < detail->regs_write_count; count++) {
                if (detail->regs_write[count] == reg_invar)
                    return true;
            }
            if (x86->op_count == 2) {
                if (x86->operands[0].type == X86_OP_REG &&
                    x86->operands[0].reg == reg_invar &&
                    x86->operands[0].access & CS_AC_WRITE) {
                    if (x86->operands[1].type != X86_OP_IMM) {
                        //std::cout << "modify_insn: " << insn->dump() << std::endl;
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool LoopInit::isSimpleLoop(std::shared_ptr<BasicBlock> block)
    {
        auto func = block->GetParent();
        if (!func) return false;
        auto end_i_ptr = block->getEndInsn();
        auto base_ptr = end_i_ptr->getPointerObject();
        if (!base_ptr) return false;
        if (base_ptr->GetBaseType() != CBaseStruc::base_basicblock)
            return false;
        auto branch_ptr = std::static_pointer_cast<BasicBlock>(base_ptr);
        if (branch_ptr->GetParent() != branch_ptr->GetParent())
            return false;
        if (branch_ptr == block) {
            block->setBlockType(BasicBlock::block_loop);
            int mflag = GetCapstoneImplment().GetModifyEflags(*end_i_ptr);
            if (!mflag) return false;
            bool bfind = false;
            std::shared_ptr<CapInsn> invariant_insn = nullptr;
            int insn_count = 0;
            for (insn_count = block->size() - 2; insn_count >= 0; insn_count--) {
                invariant_insn = block->getInsn(insn_count);
                if (!invariant_insn) continue;
                if(isInvariantInsn(*invariant_insn, mflag)) {
                    bfind = true;
                    break;
                }
            }
            if (bfind && invariant_insn) {
                std::any anyvalue = GetInvariantContext(*invariant_insn);
                if (anyvalue.has_value() && anyvalue.type() == typeid(invariant_context_type)) {
                    auto[invariant, stepsize] = std::any_cast<invariant_context_type>(anyvalue);
                    if (simulateTraceInvariant(block, invariant)) {
                        return false;
                    }
                    SimpleLoop::value_type value = Loop::invalid_init_value;
                    if (block->GetPreBlockList().size() == 2) {
                        for (auto& pre_bb : block->GetPreBlockList()) {
                            if (pre_bb.lock() == block) continue;
                            value = GetInvariantValue(invariant, pre_bb.lock());
                            if (value != Loop::invalid_init_value) {
                                break;
                            }
                        }
                    }
                    return func->addSimpleLoop(block, invariant, value, stepsize);
                }
            }
        }
        return false;
    }
} // namespace cchips
