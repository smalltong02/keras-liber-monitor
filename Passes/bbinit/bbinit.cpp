#include "bbinit.h"
#include "PassSupport.h"
#include "utils.h"

namespace cchips {

    char BBInit::ID = 0;

    static RegisterPass<BBInit> X(
        "BBInit",
        "BasicBlock initializing",
        PassInfo::passreg_pre
    );

    bool BBInit::runOnBasicBlock(std::shared_ptr<BasicBlock> Block)
    {
        if (!Block || !Block->GetParent())
            return false;
        return run(Block);
    }

    bool BBInit::run(std::shared_ptr<BasicBlock>& Block)
    {
        if (!Block) return false;
        if (!Block->IsUnknown() && !Block->IsStart()) return false;
        for (auto& insn : *Block) {
            if (!insn) return false;
            if (cchips::GetCapstoneImplment().InCallGroup(*insn)) {
                processCallInstruction(*insn);
            }
        }
        std::shared_ptr<CapInsn> end_insn = Block->getEndInsn();
        if (!end_insn) return false;
        return processEndBlock(*end_insn);
    }

    bool BBInit::processEndBlock(CapInsn& insn)
    {
        std::shared_ptr<BasicBlock> block = insn.GetParent();
        if (!block) return false;
        std::shared_ptr<Function> function = block->GetParent();
        if (!function) return false;
        if (cchips::GetCapstoneImplment().InJmpGroup(insn)) {
            return processJmpInstruction(insn);
        }
        else if (cchips::GetCapstoneImplment().InRetGroup(insn)) {
            block->setBlockType(BasicBlock::block_end);
        }
        else if (cchips::GetCapstoneImplment().InLoopGroup(insn)) {
            return processLoopInstruction(insn);
        }
        else {
            block->setBlockType(BasicBlock::block_sequnce);
            std::shared_ptr<BasicBlock> after_block = function->getBasicBlockAfter(block->getAddress());
            if (!after_block) return false;
            return function->linkBasicBlock(block, after_block, JumpTarget::jmp_type::JMP_CONTROL_FLOW_BR_FALSE);
        }
        return true;
    }

    bool BBInit::processJmpInstruction(CapInsn& insn)
    {
        std::shared_ptr<BasicBlock> block = insn.GetParent();
        if (!block) return false;
        std::shared_ptr<Function> function = block->GetParent();
        if (!function) return false;
        std::shared_ptr<Module> cur_module = function->GetParent();
        if (!cur_module) return false;
        std::uint8_t* next_addr = nullptr;
        std::uint8_t* jmp_addr = nullptr;
        if (x86_op_type op_type; GetCapstoneImplment().GetJmpAddress(insn, next_addr, jmp_addr, op_type)) {
            block->setBlockType(BasicBlock::block_branch);
            std::shared_ptr<BasicBlock> post_block = function->getBasicBlockAtAddress(next_addr);
            if (post_block) {
                function->linkBasicBlock(block, post_block, JumpTarget::jmp_type::JMP_CONTROL_FLOW_BR_FALSE);
            }
            if (jmp_addr) {
                if (op_type == X86_OP_MEM) {
                    tls_check_struct *tls = check_get_tls();
                    if (tls) {
                        tls->active = 1;
                        if (setjmp(tls->jb) == 0) {
                            jmp_addr = *reinterpret_cast<std::uint8_t**>(jmp_addr);
                            tls->active = 0;
                        }
                        else {
                            //execption occur
                            tls->active = 0;
                        }
                    }
                }
                std::shared_ptr<BasicBlock> jmp_block = function->getBasicBlockAtAddress(jmp_addr);
                if (jmp_block) {
                    insn.setPointerObject(jmp_block);
                    insn.updateMnemonic(jmp_block->GetName());
                    function->linkBasicBlock(block, jmp_block, JumpTarget::jmp_type::JMP_CONTROL_FLOW_BR_TRUE);
                }
                else if (std::shared_ptr<CBaseStruc> jmpto_base; jmpto_base = cur_module->GetBaseObjectAtAddress(jmp_addr)) {
                    switch (jmpto_base->GetBaseType())
                    {
                    case CBaseStruc::base_function:
                    {
                        std::shared_ptr<Function> post_func = std::static_pointer_cast<Function>(jmpto_base);
                        insn.setPointerObject(jmpto_base);
                        insn.updateMnemonic(post_func->GetFuncName());
                        block->setBlockType(BasicBlock::block_linkage);
                    }
                    break;
                    case CBaseStruc::base_globalifunc:
                    {
                        std::shared_ptr<GlobalIFunc> post_ifunc = std::static_pointer_cast<GlobalIFunc>(jmpto_base);
                        insn.setPointerObject(jmpto_base);
                        insn.updateMnemonic(post_ifunc->GetFullName());
                        block->setBlockType(BasicBlock::block_linkage);
                    }
                    break;
                    case CBaseStruc::base_globalvariable:
                    {
                        std::shared_ptr<GlobalVariable> post_ifunc = std::static_pointer_cast<GlobalVariable>(jmpto_base);
                        insn.setPointerObject(jmpto_base);
                        insn.updateMnemonic(post_ifunc->GetName());
                        block->setBlockType(BasicBlock::block_linkage);
                    }
                    break;
                    default:
                        // don't know.
                        ;
                    }
                }
                else {
                    // maybe error.
                }
            }
        }
        else {
            block->setBlockType(BasicBlock::block_invalid);
            return false;
        }
        return true;
    }

    bool BBInit::processLoopInstruction(CapInsn& insn)
    {
        std::shared_ptr<BasicBlock> block = insn.GetParent();
        if (!block) return false;
        std::shared_ptr<Function> function = block->GetParent();
        if (!function) return false;
        std::shared_ptr<Module> cur_module = function->GetParent();
        if (!cur_module) return false;
        std::uint8_t* next_addr = nullptr;
        std::uint8_t* loop_addr = nullptr;
        if (x86_op_type op_type; GetCapstoneImplment().GetLoopAddress(insn, next_addr, loop_addr, op_type)) {
            block->setBlockType(BasicBlock::block_loop);
            std::shared_ptr<BasicBlock> post_block = function->getBasicBlockAtAddress(next_addr);
            if (post_block) {
                function->linkBasicBlock(block, post_block, JumpTarget::jmp_type::JMP_CONTROL_FLOW_BR_FALSE);
            }
            else {
                //error
                block->setBlockType(BasicBlock::block_invalid);
                return false;
            }
            if (op_type == X86_OP_MEM) {
                
            }
            std::shared_ptr<BasicBlock> loop_block = function->getBasicBlockAtAddress(loop_addr);
            if (loop_block) {
                insn.setPointerObject(loop_block);
                insn.updateMnemonic(loop_block->GetName());
                function->linkBasicBlock(block, loop_block, JumpTarget::jmp_type::JMP_CONTROL_FLOW_BR_TRUE);
            }
            else {
                // maybe error.
            }
        }
        else {
            block->setBlockType(BasicBlock::block_invalid);
            return false;
        }
        return true;
    }

    void BBInit::processCallInstruction(CapInsn& insn)
    {
        std::shared_ptr<BasicBlock> block = insn.GetParent();
        if (!block) return;
        std::shared_ptr<Function> function = block->GetParent();
        if (!function) return;
        std::shared_ptr<Module> cur_module = function->GetParent();
        if (!cur_module) return;
        std::uint8_t* call_addr = nullptr;
        Range<std::uint8_t*> image_range;
        image_range.setStartEnd(const_cast<std::uint8_t*>(cur_module->GetContext()->GetBaseAddress()), const_cast<std::uint8_t*>(cur_module->GetContext()->GetEndAddress()));
        if (x86_op_type op_type; GetCapstoneImplment().GetCallAddress(insn, call_addr, op_type)) {
            if (call_addr) {
                if (op_type == X86_OP_MEM) {
                    tls_check_struct *tls = check_get_tls();
                    if (tls) {
                        tls->active = 1;
                        if (setjmp(tls->jb) == 0) {
                            call_addr = *reinterpret_cast<std::uint8_t**>(call_addr);
                            tls->active = 0;
                        }
                        else {
                            //execption occur
                            tls->active = 0;
                        }
                    }
                }
                std::shared_ptr<BasicBlock> call_block = function->getBasicBlockAtAddress(call_addr);
                if (call_block) {
                    // Confuse, call address in basicblock.
                }
                else if (std::shared_ptr<CBaseStruc> callto_base; callto_base = cur_module->GetBaseObjectAtAddress(call_addr)) {
                    if (callto_base) {
                        switch (callto_base->GetBaseType())
                        {
                        case CBaseStruc::base_function:
                        {
                            std::shared_ptr<Function> post_func = std::static_pointer_cast<Function>(callto_base);
                            insn.setPointerObject(callto_base);
                            insn.updateMnemonic(post_func->GetFuncName());
                        }
                        break;
                        case CBaseStruc::base_globalifunc:
                        {
                            std::shared_ptr<GlobalIFunc> post_ifunc = std::static_pointer_cast<GlobalIFunc>(callto_base);
                            insn.setPointerObject(callto_base);
                            insn.updateMnemonic(post_ifunc->GetFullName());
                        }
                        break;
                        case CBaseStruc::base_globalvariable:
                        {
                            std::shared_ptr<GlobalVariable> post_ifunc = std::static_pointer_cast<GlobalVariable>(callto_base);
                            insn.setPointerObject(callto_base);
                            insn.updateMnemonic(post_ifunc->GetName());
                        }
                        break;
                        default:
                            // don't know.
                            ;
                        }
                    }
                }
                else {
                    //if (image_range.contains(call_addr)) {
                    //    if (!cur_module->AddFunction(std::string(""), call_addr)) return;
                    //    std::shared_ptr<Function> func = cur_module->GetFunction(call_addr);
                    //    if (!func) return;
                    //    insn.setPointerObject(func);
                    //    insn.updateMnemonic(func->GetFuncName());
                    //    std::shared_ptr<FunctionPassManager> funcs_manager = std::static_pointer_cast<FunctionPassManager>(GetPassRegistry().getPassManager(Pass::passmanager_function));
                    //    if (!funcs_manager) return;
                    //    funcs_manager->Run(func);
                    //}
                    //else {
                    //    // report, feature.
                    //    // check stack? heap? or other module.
                    //}
                }
            }
        }
        return;
    }
} // namespace cchips
