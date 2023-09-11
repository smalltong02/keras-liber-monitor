/* This file was auto-generated with comp_map.py.  DO NOT EDIT! */

#include "bytecode.h"

int python_39_map(int id)
{
    switch (id) {
    case 1: return Pyc::POP_TOP;
    case 2: return Pyc::ROT_TWO;
    case 3: return Pyc::ROT_THREE;
    case 4: return Pyc::DUP_TOP;
    case 5: return Pyc::DUP_TOP_TWO;
    case 6: return Pyc::ROT_FOUR;
    case 9: return Pyc::NOP;
    case 10: return Pyc::UNARY_POSITIVE;
    case 11: return Pyc::UNARY_NEGATIVE;
    case 12: return Pyc::UNARY_NOT;
    case 15: return Pyc::UNARY_INVERT;
    case 16: return Pyc::BINARY_MATRIX_MULTIPLY;
    case 17: return Pyc::INPLACE_MATRIX_MULTIPLY;
    case 19: return Pyc::BINARY_POWER;
    case 20: return Pyc::BINARY_MULTIPLY;
    case 22: return Pyc::BINARY_MODULO;
    case 23: return Pyc::BINARY_ADD;
    case 24: return Pyc::BINARY_SUBTRACT;
    case 25: return Pyc::BINARY_SUBSCR;
    case 26: return Pyc::BINARY_FLOOR_DIVIDE;
    case 27: return Pyc::BINARY_TRUE_DIVIDE;
    case 28: return Pyc::INPLACE_FLOOR_DIVIDE;
    case 29: return Pyc::INPLACE_TRUE_DIVIDE;
    case 48: return Pyc::RERAISE;
    case 49: return Pyc::WITH_EXCEPT_START;
    case 50: return Pyc::GET_AITER;
    case 51: return Pyc::GET_ANEXT;
    case 52: return Pyc::BEFORE_ASYNC_WITH;
    case 54: return Pyc::END_ASYNC_FOR;
    case 55: return Pyc::INPLACE_ADD;
    case 56: return Pyc::INPLACE_SUBTRACT;
    case 57: return Pyc::INPLACE_MULTIPLY;
    case 59: return Pyc::INPLACE_MODULO;
    case 60: return Pyc::STORE_SUBSCR;
    case 61: return Pyc::DELETE_SUBSCR;
    case 62: return Pyc::BINARY_LSHIFT;
    case 63: return Pyc::BINARY_RSHIFT;
    case 64: return Pyc::BINARY_AND;
    case 65: return Pyc::BINARY_XOR;
    case 66: return Pyc::BINARY_OR;
    case 67: return Pyc::INPLACE_POWER;
    case 68: return Pyc::GET_ITER;
    case 69: return Pyc::GET_YIELD_FROM_ITER;
    case 70: return Pyc::PRINT_EXPR;
    case 71: return Pyc::LOAD_BUILD_CLASS;
    case 72: return Pyc::YIELD_FROM;
    case 73: return Pyc::GET_AWAITABLE;
    case 74: return Pyc::LOAD_ASSERTION_ERROR;
    case 75: return Pyc::INPLACE_LSHIFT;
    case 76: return Pyc::INPLACE_RSHIFT;
    case 77: return Pyc::INPLACE_AND;
    case 78: return Pyc::INPLACE_XOR;
    case 79: return Pyc::INPLACE_OR;
    case 82: return Pyc::LIST_TO_TUPLE;
    case 83: return Pyc::RETURN_VALUE;
    case 84: return Pyc::IMPORT_STAR;
    case 85: return Pyc::SETUP_ANNOTATIONS;
    case 86: return Pyc::YIELD_VALUE;
    case 87: return Pyc::POP_BLOCK;
    case 89: return Pyc::POP_EXCEPT;
    case 90: return Pyc::STORE_NAME_A;
    case 91: return Pyc::DELETE_NAME_A;
    case 92: return Pyc::UNPACK_SEQUENCE_A;
    case 93: return Pyc::FOR_ITER_A;
    case 94: return Pyc::UNPACK_EX_A;
    case 95: return Pyc::STORE_ATTR_A;
    case 96: return Pyc::DELETE_ATTR_A;
    case 97: return Pyc::STORE_GLOBAL_A;
    case 98: return Pyc::DELETE_GLOBAL_A;
    case 100: return Pyc::LOAD_CONST_A;
    case 101: return Pyc::LOAD_NAME_A;
    case 102: return Pyc::BUILD_TUPLE_A;
    case 103: return Pyc::BUILD_LIST_A;
    case 104: return Pyc::BUILD_SET_A;
    case 105: return Pyc::BUILD_MAP_A;
    case 106: return Pyc::LOAD_ATTR_A;
    case 107: return Pyc::COMPARE_OP_A;
    case 108: return Pyc::IMPORT_NAME_A;
    case 109: return Pyc::IMPORT_FROM_A;
    case 110: return Pyc::JUMP_FORWARD_A;
    case 111: return Pyc::JUMP_IF_FALSE_OR_POP_A;
    case 112: return Pyc::JUMP_IF_TRUE_OR_POP_A;
    case 113: return Pyc::JUMP_ABSOLUTE_A;
    case 114: return Pyc::POP_JUMP_IF_FALSE_A;
    case 115: return Pyc::POP_JUMP_IF_TRUE_A;
    case 116: return Pyc::LOAD_GLOBAL_A;
    case 117: return Pyc::IS_OP_A;
    case 118: return Pyc::CONTAINS_OP_A;
    case 121: return Pyc::JUMP_IF_NOT_EXC_MATCH_A;
    case 122: return Pyc::SETUP_FINALLY_A;
    case 124: return Pyc::LOAD_FAST_A;
    case 125: return Pyc::STORE_FAST_A;
    case 126: return Pyc::DELETE_FAST_A;
    case 130: return Pyc::RAISE_VARARGS_A;
    case 131: return Pyc::CALL_FUNCTION_A;
    case 132: return Pyc::MAKE_FUNCTION_A;
    case 133: return Pyc::BUILD_SLICE_A;
    case 135: return Pyc::LOAD_CLOSURE_A;
    case 136: return Pyc::LOAD_DEREF_A;
    case 137: return Pyc::STORE_DEREF_A;
    case 138: return Pyc::DELETE_DEREF_A;
    case 141: return Pyc::CALL_FUNCTION_KW_A;
    case 142: return Pyc::CALL_FUNCTION_EX_A;
    case 143: return Pyc::SETUP_WITH_A;
    case 144: return Pyc::EXTENDED_ARG_A;
    case 145: return Pyc::LIST_APPEND_A;
    case 146: return Pyc::SET_ADD_A;
    case 147: return Pyc::MAP_ADD_A;
    case 148: return Pyc::LOAD_CLASSDEREF_A;
    case 154: return Pyc::SETUP_ASYNC_WITH_A;
    case 155: return Pyc::FORMAT_VALUE_A;
    case 156: return Pyc::BUILD_CONST_KEY_MAP_A;
    case 157: return Pyc::BUILD_STRING_A;
    case 160: return Pyc::LOAD_METHOD_A;
    case 161: return Pyc::CALL_METHOD_A;
    case 162: return Pyc::LIST_EXTEND_A;
    case 163: return Pyc::SET_UPDATE_A;
    case 164: return Pyc::DICT_MERGE_A;
    case 165: return Pyc::DICT_UPDATE_A;
    default: return Pyc::PYC_INVALID_OPCODE;
    }
}

int python_39_unmap(int id)
{
    switch (id) {
    case Pyc::BEFORE_ASYNC_WITH: return 52;
    case Pyc::BINARY_ADD: return 23;
    case Pyc::BINARY_AND: return 64;
    case Pyc::BINARY_FLOOR_DIVIDE: return 26;
    case Pyc::BINARY_LSHIFT: return 62;
    case Pyc::BINARY_MATRIX_MULTIPLY: return 16;
    case Pyc::BINARY_MODULO: return 22;
    case Pyc::BINARY_MULTIPLY: return 20;
    case Pyc::BINARY_OR: return 66;
    case Pyc::BINARY_POWER: return 19;
    case Pyc::BINARY_RSHIFT: return 63;
    case Pyc::BINARY_SUBSCR: return 25;
    case Pyc::BINARY_SUBTRACT: return 24;
    case Pyc::BINARY_TRUE_DIVIDE: return 27;
    case Pyc::BINARY_XOR: return 65;
    case Pyc::BUILD_CONST_KEY_MAP_A: return 156;
    case Pyc::BUILD_LIST_A: return 103;
    case Pyc::BUILD_MAP_A: return 105;
    case Pyc::BUILD_SET_A: return 104;
    case Pyc::BUILD_SLICE_A: return 133;
    case Pyc::BUILD_STRING_A: return 157;
    case Pyc::BUILD_TUPLE_A: return 102;
    case Pyc::CALL_FUNCTION_A: return 131;
    case Pyc::CALL_FUNCTION_EX_A: return 142;
    case Pyc::CALL_FUNCTION_KW_A: return 141;
    case Pyc::CALL_METHOD_A: return 161;
    case Pyc::COMPARE_OP_A: return 107;
    case Pyc::CONTAINS_OP_A: return 118;
    case Pyc::DELETE_ATTR_A: return 96;
    case Pyc::DELETE_DEREF_A: return 138;
    case Pyc::DELETE_FAST_A: return 126;
    case Pyc::DELETE_GLOBAL_A: return 98;
    case Pyc::DELETE_NAME_A: return 91;
    case Pyc::DELETE_SUBSCR: return 61;
    case Pyc::DICT_MERGE_A: return 164;
    case Pyc::DICT_UPDATE_A: return 165;
    case Pyc::DUP_TOP: return 4;
    case Pyc::DUP_TOP_TWO: return 5;
    case Pyc::END_ASYNC_FOR: return 54;
    case Pyc::EXTENDED_ARG_A: return 144;
    case Pyc::FORMAT_VALUE_A: return 155;
    case Pyc::FOR_ITER_A: return 93;
    case Pyc::GET_AITER: return 50;
    case Pyc::GET_ANEXT: return 51;
    case Pyc::GET_AWAITABLE: return 73;
    case Pyc::GET_ITER: return 68;
    case Pyc::GET_YIELD_FROM_ITER: return 69;
    case Pyc::IMPORT_FROM_A: return 109;
    case Pyc::IMPORT_NAME_A: return 108;
    case Pyc::IMPORT_STAR: return 84;
    case Pyc::INPLACE_ADD: return 55;
    case Pyc::INPLACE_AND: return 77;
    case Pyc::INPLACE_FLOOR_DIVIDE: return 28;
    case Pyc::INPLACE_LSHIFT: return 75;
    case Pyc::INPLACE_MATRIX_MULTIPLY: return 17;
    case Pyc::INPLACE_MODULO: return 59;
    case Pyc::INPLACE_MULTIPLY: return 57;
    case Pyc::INPLACE_OR: return 79;
    case Pyc::INPLACE_POWER: return 67;
    case Pyc::INPLACE_RSHIFT: return 76;
    case Pyc::INPLACE_SUBTRACT: return 56;
    case Pyc::INPLACE_TRUE_DIVIDE: return 29;
    case Pyc::INPLACE_XOR: return 78;
    case Pyc::IS_OP_A: return 117;
    case Pyc::JUMP_ABSOLUTE_A: return 113;
    case Pyc::JUMP_FORWARD_A: return 110;
    case Pyc::JUMP_IF_FALSE_OR_POP_A: return 111;
    case Pyc::JUMP_IF_NOT_EXC_MATCH_A: return 121;
    case Pyc::JUMP_IF_TRUE_OR_POP_A: return 112;
    case Pyc::LIST_APPEND_A: return 145;
    case Pyc::LIST_EXTEND_A: return 162;
    case Pyc::LIST_TO_TUPLE: return 82;
    case Pyc::LOAD_ASSERTION_ERROR: return 74;
    case Pyc::LOAD_ATTR_A: return 106;
    case Pyc::LOAD_BUILD_CLASS: return 71;
    case Pyc::LOAD_CLASSDEREF_A: return 148;
    case Pyc::LOAD_CLOSURE_A: return 135;
    case Pyc::LOAD_CONST_A: return 100;
    case Pyc::LOAD_DEREF_A: return 136;
    case Pyc::LOAD_FAST_A: return 124;
    case Pyc::LOAD_GLOBAL_A: return 116;
    case Pyc::LOAD_METHOD_A: return 160;
    case Pyc::LOAD_NAME_A: return 101;
    case Pyc::MAKE_FUNCTION_A: return 132;
    case Pyc::MAP_ADD_A: return 147;
    case Pyc::NOP: return 9;
    case Pyc::POP_BLOCK: return 87;
    case Pyc::POP_EXCEPT: return 89;
    case Pyc::POP_JUMP_IF_FALSE_A: return 114;
    case Pyc::POP_JUMP_IF_TRUE_A: return 115;
    case Pyc::POP_TOP: return 1;
    case Pyc::PRINT_EXPR: return 70;
    case Pyc::RAISE_VARARGS_A: return 130;
    case Pyc::RERAISE: return 48;
    case Pyc::RETURN_VALUE: return 83;
    case Pyc::ROT_FOUR: return 6;
    case Pyc::ROT_THREE: return 3;
    case Pyc::ROT_TWO: return 2;
    case Pyc::SETUP_ANNOTATIONS: return 85;
    case Pyc::SETUP_ASYNC_WITH_A: return 154;
    case Pyc::SETUP_FINALLY_A: return 122;
    case Pyc::SETUP_WITH_A: return 143;
    case Pyc::SET_ADD_A: return 146;
    case Pyc::SET_UPDATE_A: return 163;
    case Pyc::STORE_ATTR_A: return 95;
    case Pyc::STORE_DEREF_A: return 137;
    case Pyc::STORE_FAST_A: return 125;
    case Pyc::STORE_GLOBAL_A: return 97;
    case Pyc::STORE_NAME_A: return 90;
    case Pyc::STORE_SUBSCR: return 60;
    case Pyc::UNARY_INVERT: return 15;
    case Pyc::UNARY_NEGATIVE: return 11;
    case Pyc::UNARY_NOT: return 12;
    case Pyc::UNARY_POSITIVE: return 10;
    case Pyc::UNPACK_EX_A: return 94;
    case Pyc::UNPACK_SEQUENCE_A: return 92;
    case Pyc::WITH_EXCEPT_START: return 49;
    case Pyc::YIELD_FROM: return 72;
    case Pyc::YIELD_VALUE: return 86;
    default: return -1;
    }
}
