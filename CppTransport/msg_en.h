//
// Created by David Seery on 12/06/2013.
// Copyright (c) 2013 University of Sussex. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//

#ifndef __msg_H_
#define __msg_H_

#define ERROR_OPEN_TOPLEVEL        "Could not open top-level file"
#define ERROR_INCLUDE_FILE         "Could not open included file"
#define ERROR_INCLUDE_DIRECTIVE    "Expected filename following #include directive"

#define ERROR_MISSING_PATHNAME     "Too few arguments: expected pathname following"
#define ERROR_MISSING_BACKEND      "Too few arguments: expected backend name following --backend"

#define ERROR_UNKNOWN_BACKEND      "Requested unknown backend"
#define ERROR_BACKEND_FAILURE      "Failed to generate output for backend"

#define ERROR_BACKEND_OUTPUT       "Error opening backend output file"

#define ERROR_NO_CPP_TEMPLATE      "No C++ template specified"
#define ERROR_MISSING_CPP_HEADER   "Could not find specified C++ header template"
#define ERROR_CPP_TEMPLATE_READ    "Error reading from C++ template file"

#define ERROR_MISSING_POTENTIAL    "No potential specified"

#define ERROR_UNRECOGNIZED_NUMBER  "Unrecognized numeric literal"
#define ERROR_UNRECOGNIZED_SYMBOL  "Unrecognized character"
#define ERROR_EXPECTED_CLOSE_QUOTE "Unterminated string literal"
#define ERROR_EXPECTED_ELLIPSIS    "Expected '...', not '..'"

#define ERROR_SYMBOL_EXISTS        "Redefinition of existing symbol"
#define ERROR_IDENTIFIER_LOOKUP    "Lookup of identifier name failed"
#define ERROR_STRING_LOOKUP        "Lookup of string literal value failed"
#define ERROR_INTEGER_LOOKUP       "Lookup of integer literal value failed"
#define ERROR_DECIMAL_LOOKUP       "Lookup of decimal literal value failed"
#define ERROR_UNKNOWN_IDENTIFIER   "Undeclared identifier"

#define ERROR_EXPECTED_INDEX       "Expected index label following prefix"
#define ERROR_TOO_FEW_INDICES      "Too few indices supplied to summation-convention object"
#define ERROR_EXPECTED_OPEN_IDXL   "Expected index list beginning with '[' for indexed macro"
#define ERROR_EXPECTED_CLOSE_IDXL  "Expected close of index list ']' for indexed macro"
#define ERROR_DUPLICATE_INDEX      "Duplicate declaration of lvalue summation convention index"
#define ERROR_EXPECTED_CLOSE_ARGL  "Expected close of argument list '}'"
#define ERROR_WRONG_ARG_NUM        "Unexpected number of arguments to macro"
#define ERROR_WRONG_ARG_NUM_EXPECT "expected"

#define WARNING_HEX_CONVERSION_A   "Leading '0x' in "
#define WARNING_HEX_CONVERSION_B   "indicates hex, but did not convert: treated as decmial"
#define WARNING_OCTAL_CONVERSION_A "Leading '0' in "
#define WARNING_OCTAL_CONVERSION_B "indicates octal, but did not convert: treated as decimal"

#define WARNING_LEXEME_KEYWORD     "Internal error: attempt to get keyword type for non-keyword lexeme"
#define WARNING_LEXEME_SYMBOL      "Internal error: attempt to get symbol type for non-symbol lexeme"
#define WARNING_LEXEME_INTEGER     "Internal error: attempt to get integer value for non-integer lexeme"
#define WARNING_LEXEME_DECIMAL     "Internal error: attempt to get decimal value for non-deciaml lexeme"
#define WARNING_LEXEME_STRING      "Internal error: attempt to get string value for non-string lexeme"
#define WARNING_LEXEME_IDENTIFIER  "Internal error: attempt to get identifier value for non-identifier lexeme"

#define WARNING_PARSING_FAILED     "Failed to parse file"

#endif //__msg_H_
