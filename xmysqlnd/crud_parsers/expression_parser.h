/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 2006-2017 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Oracle Corp                                                 |
  +----------------------------------------------------------------------+
*/

#ifndef CRUD_PARSERS_EXPRESSION_PARSER_H
#define CRUD_PARSERS_EXPRESSION_PARSER_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <stdexcept>

#include <boost/function.hpp>

// Avoid warnings from includes of other project and protobuf
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#elif defined _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4018 4996)
#endif

#include "legacy_tokenizer.h"
#include "xmysqlnd/proto_gen/mysqlx_datatypes.pb.h"
#include "xmysqlnd/proto_gen/mysqlx_expr.pb.h"
#include "xmysqlnd/proto_gen/mysqlx_crud.pb.h"

#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)
#pragma GCC diagnostic pop
#elif defined _MSC_VER
#pragma warning (pop)
#endif


namespace mysqlx {

namespace old_parser_api {

class Expr_builder
{
  public:
    static Mysqlx::Datatypes::Scalar* build_null_scalar();
    static Mysqlx::Datatypes::Scalar* build_double_scalar(double d);
    static Mysqlx::Datatypes::Scalar* build_int_scalar(google::protobuf::int64 i);
    static Mysqlx::Datatypes::Scalar* build_string_scalar(const std::string& s);
    static Mysqlx::Datatypes::Scalar* build_bool_scalar(bool b);
    static Mysqlx::Expr::Expr* build_literal_expr(Mysqlx::Datatypes::Scalar* sc);
    static Mysqlx::Expr::Expr* build_unary_op(const std::string& name, Mysqlx::Expr::Expr* param);
};

class Expression_parser
{
  public:
    Expression_parser(const std::string& expr_str, bool document_mode = false, bool allow_alias = false, std::vector<std::string>* place_holders = nullptr);

    typedef boost::function<Mysqlx::Expr::Expr*(Expression_parser*)> inner_parser_t;

    void paren_expr_list(::google::protobuf::RepeatedPtrField< ::Mysqlx::Expr::Expr >* expr_list);
    Mysqlx::Expr::Identifier* identifier();
    Mysqlx::Expr::Expr* function_call();
    void docpath_member(Mysqlx::Expr::DocumentPathItem& item);
    void docpath_array_loc(Mysqlx::Expr::DocumentPathItem& item);
    void document_path(Mysqlx::Expr::ColumnIdentifier& colid);
    const std::string& id();
    Mysqlx::Expr::Expr* column_field();
    Mysqlx::Expr::Expr* document_field();
    Mysqlx::Expr::Expr* atomic_expr();
    Mysqlx::Expr::Expr* array_();
    Mysqlx::Expr::Expr* parse_left_assoc_binary_op_expr(std::set<Token::TokenType>& types, inner_parser_t inner_parser);
    Mysqlx::Expr::Expr* mul_div_expr();
    Mysqlx::Expr::Expr* add_sub_expr();
    Mysqlx::Expr::Expr* shift_expr();
    Mysqlx::Expr::Expr* bit_expr();
    Mysqlx::Expr::Expr* comp_expr();
    Mysqlx::Expr::Expr* ilri_expr();
    Mysqlx::Expr::Expr* and_expr();
    Mysqlx::Expr::Expr* or_expr();
    Mysqlx::Expr::Expr* expr();

    std::vector<Token>::const_iterator begin() const { return _tokenizer.begin(); }
    std::vector<Token>::const_iterator end() const { return _tokenizer.end(); }

  protected:

    struct operator_list
    {
      std::set<Token::TokenType> mul_div_expr_types;
      std::set<Token::TokenType> add_sub_expr_types;
      std::set<Token::TokenType> shift_expr_types;
      std::set<Token::TokenType> bit_expr_types;
      std::set<Token::TokenType> comp_expr_types;
      std::set<Token::TokenType> and_expr_types;
      std::set<Token::TokenType> or_expr_types;

      operator_list()
      {
        mul_div_expr_types.insert(Token::MUL);
        mul_div_expr_types.insert(Token::DIV);
        mul_div_expr_types.insert(Token::MOD);

        add_sub_expr_types.insert(Token::PLUS);
        add_sub_expr_types.insert(Token::MINUS);

        shift_expr_types.insert(Token::LSHIFT);
        shift_expr_types.insert(Token::RSHIFT);

        bit_expr_types.insert(Token::BITAND);
        bit_expr_types.insert(Token::BITOR);
        bit_expr_types.insert(Token::BITXOR);

        comp_expr_types.insert(Token::GE);
        comp_expr_types.insert(Token::GT);
        comp_expr_types.insert(Token::LE);
        comp_expr_types.insert(Token::LT);
        comp_expr_types.insert(Token::EQ);
        comp_expr_types.insert(Token::NE);

        and_expr_types.insert(Token::AND);

        or_expr_types.insert(Token::OR);
      }
    };

    static operator_list _ops;

    // json
    void json_key_value(Mysqlx::Expr::Object* obj);
    Mysqlx::Expr::Expr* json_doc();
    // placeholder
    std::vector<std::string> _place_holders;
    std::vector<std::string>* _place_holder_ref;
    Mysqlx::Expr::Expr* placeholder();
    // cast
    Mysqlx::Expr::Expr* my_expr();
    Mysqlx::Expr::Expr* cast();
    Mysqlx::Expr::Expr *binary();
    std::string cast_data_type();
    std::string cast_data_type_dimension(bool double_dimension = false);
    std::string opt_binary();
    std::string charset_def();
    Tokenizer _tokenizer;
    bool _document_mode;
    bool _allow_alias;
};

class Expression_unparser
{
  public:
    static std::string any_to_string(const Mysqlx::Datatypes::Any& a);
    static std::string escape_literal(const std::string& s);
    static std::string scalar_to_string(const Mysqlx::Datatypes::Scalar& s);
    static std::string document_path_to_string(const ::google::protobuf::RepeatedPtrField< ::Mysqlx::Expr::DocumentPathItem >& dp);
    static std::string column_identifier_to_string(const Mysqlx::Expr::ColumnIdentifier& colid);
    static std::string function_call_to_string(const Mysqlx::Expr::FunctionCall& fc);
    static std::string operator_to_string(const Mysqlx::Expr::Operator& op);
    static std::string quote_identifier(const std::string& id);
    static std::string expr_to_string(const Mysqlx::Expr::Expr& e);
    static std::string object_to_string(const Mysqlx::Expr::Object& o);
    static std::string placeholder_to_string(const Mysqlx::Expr::Expr& e);
    static std::string column_to_string(const Mysqlx::Crud::Projection& c);
    static std::string order_to_string(const Mysqlx::Crud::Order& c);
    static std::string array_to_string(const Mysqlx::Expr::Expr& e);
    static std::string column_list_to_string(google::protobuf::RepeatedPtrField< ::Mysqlx::Crud::Projection > columns);
    static std::string order_list_to_string(google::protobuf::RepeatedPtrField< ::Mysqlx::Crud::Order> columns);

    static void replace(std::string& target, const std::string& old_val, const std::string& new_val);
};

} // namespace parser

} // namespace mysqlx

#endif /* CRUD_PARSERS_EXPRESSION_PARSER_H */
