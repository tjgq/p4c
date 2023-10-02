#include "backends/p4tools/modules/smith/common/baseType.h"

#include <vector>

#include "backends/p4tools/modules/smith/common/scope.h"
#include "backends/p4tools/modules/smith/util/util.h"

namespace P4Tools {

namespace P4Smith {

const int BaseType::bit_widths[6];

IR::Type *BaseType::pick_rnd_base_type(const std::vector<int64_t> &type_probs) {
    if (type_probs.size() != 7) {
        BUG("pick_rnd_base_type: Type probabilities must be exact");
    }
    IR::Type *tb = nullptr;
    switch (randInd(type_probs)) {
        case 0: {
            // bool
            tb = gen_bool_type();
            break;
        }
        case 1: {
            // error, this is !supported right now
            break;
        }
        case 2: {
            // int, this is !supported right now
            tb = gen_int_type();
            break;
        }
        case 3: {
            // string, this is !supported right now
            break;
        }
        case 4: {
            // bit<>
            tb = gen_bit_type(false);
            break;
        }
        case 5: {
            // int<>
            tb = gen_bit_type(true);
            break;
        }
        case 6: {
            // varbit<>, this is !supported right now
            break;
        }
    }
    return tb;
}

IR::BoolLiteral *BaseType::gen_bool_literal() {
    if (getRndInt(0, 1)) {
        return new IR::BoolLiteral(false);
    } else {
        return new IR::BoolLiteral(true);
    }
}

// isSigned, true -> int<>, false -> bit<>
IR::Type_Bits *BaseType::gen_bit_type(bool isSigned) {
    auto size = getRndInt(0, sizeof(bit_widths) / sizeof(int) - 1);

    return new IR::Type_Bits(bit_widths[size], isSigned);
}

IR::Constant *BaseType::gen_int_literal(size_t bit_width) {
    big_int min = -(((big_int)1 << bit_width - 1));
    if (P4Scope::req.not_negative) {
        min = 0;
    }
    big_int max = (((big_int)1 << bit_width - 1) - 1);
    big_int value = getRndBigInt(min, max);
    while (true) {
        if (P4Scope::req.not_zero && value == 0) {
            value = getRndBigInt(min, max);
            // retry until we generate a value that is !zero
            continue;
        }
        break;
    }
    return new IR::Constant(value);
}

IR::Constant *BaseType::gen_bit_literal(const IR::Type *tb) {
    big_int max_size = ((big_int)1U << tb->width_bits());

    big_int value;
    if (P4Scope::req.not_zero) {
        value = getRndBigInt(1, max_size - 1);
    } else {
        value = getRndBigInt(0, max_size - 1);
    }
    return new IR::Constant(tb, value);
}

}  // namespace P4Smith

}  // namespace P4Tools
