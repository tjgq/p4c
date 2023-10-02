#include "backends/p4tools/modules/smith/common/expression_boolean.h"

#include <vector>

#include "backends/p4tools/modules/smith/common/argument.h"
#include "backends/p4tools/modules/smith/common/baseType.h"
#include "backends/p4tools/modules/smith/common/scope.h"

namespace P4Tools {

namespace P4Smith {

IR::Expression *expression_boolean::construct_cmp_expr() {
    IR::Expression *expr = nullptr;

    // gen some random type
    // can be either bits, int, bool, or structlike
    // for now it is just bits
    auto new_type_size = getRndInt(1, 128);
    auto new_type = new IR::Type_Bits(new_type_size, false);
    IR::Expression *left = expression_bit::construct(new_type);
    IR::Expression *right = expression_bit::construct(new_type);

    std::vector<int64_t> percent = {PCT.EXPRESSION_BOOLEAN_CMP_EQU, PCT.EXPRESSION_BOOLEAN_CMP_NEQ};

    switch (randInd(percent)) {
        case 0: {
            expr = new IR::Equ(left, right);
            // pick an equals that matches the type
        } break;
        case 1: {
            expr = new IR::Neq(left, right);
            // pick a not-equals that matches the type
        } break;
    }
    return expr;
}

IR::Expression *expression_boolean::construct() {
    IR::Expression *expr = nullptr;
    IR::Expression *left;
    IR::Expression *right;

    std::vector<int64_t> percent = {
        PCT.EXPRESSION_BOOLEAN_VAR,      PCT.EXPRESSION_BOOLEAN_LITERAL, PCT.EXPRESSION_BOOLEAN_NOT,
        PCT.EXPRESSION_BOOLEAN_LAND,     PCT.EXPRESSION_BOOLEAN_LOR,     PCT.EXPRESSION_BOOLEAN_CMP,
        PCT.EXPRESSION_BOOLEAN_FUNCTION, PCT.EXPRESSION_BOOLEAN_BUILT_IN};

    switch (randInd(percent)) {
        case 0: {
            auto tb = new IR::Type_Boolean();
            // TODO: This is lazy, we can easily check
            if (P4Scope::req.compile_time_known) {
                expr = BaseType::gen_bool_literal();
                break;
            }
            if (P4Scope::check_lval(tb)) {
                cstring name = P4Scope::pick_lval(tb);
                expr = new IR::TypeNameExpression(name);
            } else {
                expr = BaseType::gen_bool_literal();
            }
        } break;
        case 1: {
            // pick a boolean literal
            expr = BaseType::gen_bool_literal();
        } break;
        case 2: {
            // pick a Not expression
            expr = new IR::LNot(construct());
        } break;
        case 3: {
            // pick an And expression
            left = construct();
            right = construct();
            expr = new IR::LAnd(left, right);
        } break;
        case 4: {
            // pick an Or expression
            left = construct();
            right = construct();
            expr = new IR::LOr(left, right);
        } break;
        case 5: {
            // pick a comparison
            expr = construct_cmp_expr();
        } break;
        case 6: {
            auto p4_functions = P4Scope::get_decls<IR::Function>();
            auto p4_externs = P4Scope::get_decls<IR::Method>();

            IR::IndexedVector<IR::Declaration> viable_functions;
            for (auto fun : p4_functions) {
                if (fun->type->returnType->to<IR::Type_Boolean>()) {
                    viable_functions.push_back(fun);
                }
            }
            for (auto fun : p4_externs) {
                if (fun->type->returnType->to<IR::Type_Boolean>()) {
                    viable_functions.push_back(fun);
                }
            }
            const IR::Type *ret_type;
            expr = expression::pick_function(viable_functions, &ret_type);
            // can !find a suitable function, generate a default value
            if (!expr) {
                expr = BaseType::gen_bool_literal();
            }
        } break;
        case 7: {
            // get the expression
            auto tbl_set = P4Scope::get_callable_tables();

            // just generate a literal if there are no tables left
            if (tbl_set->size() == 0 || P4Scope::req.compile_time_known) {
                expr = BaseType::gen_bool_literal();
                break;
            }
            auto idx = getRndInt(0, tbl_set->size() - 1);
            auto tbl_iter = std::begin(*tbl_set);

            std::advance(tbl_iter, idx);
            const IR::P4Table *tbl = *tbl_iter;
            expr = new IR::Member(new IR::MethodCallExpression(
                                      new IR::Member(new IR::PathExpression(tbl->name), "apply")),
                                  "hit");
            tbl_set->erase(tbl_iter);
        }
    }
    return expr;
}

}  // namespace P4Smith

}  // namespace P4Tools
