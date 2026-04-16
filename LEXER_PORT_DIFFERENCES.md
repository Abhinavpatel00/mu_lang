# Lexer Port Comparison: Proposed Ring-Buffer Lexer vs Current Implementation

## Executive Summary

The proposed lexer is not a drop-in translation of the current lexer in this repository. It follows a different lexer model, token model, API shape, and error strategy.

The most important differences are:

- Lookahead model changes from single cached token to fixed-size ring buffer (up to 8 lookahead tokens).
- Token taxonomy shifts from language-level token kinds to mixed model where many punctuation tokens are represented as raw ASCII values.
- Error handling changes from ILLEGAL tokens to callback-based reporting with internal error state.
- String/escape behavior changes significantly, including Unicode escape decoding and explicit null te  rminator behavior.
- Input model changes from constructor-bound string_view to mutable owned string with file-loading and reset APIs.

If adopted as-is, parser and semantic layers will require non-trivial adaptation.

## Source Baseline

Current implementation compared:

- lexer.hpp
- lexer.cpp

Proposed implementation compared:

- Lexer.hpp (from the provided draft)
- Lexer.cpp (from the provided draft)

## High-Impact Architectural Differences

## 1) Token Stream and Lookahead

Current:

- Supports next() and peek() with one cached token (std::optional<Token> m_peek).
- Lookahead depth is effectively 1 unless parser buffers externally.

Proposed:

- Uses a ring buffer (MAX_CONCURRENT_TOKENS, power-of-two) and exposes peek_token(index).
- Supports internal multi-token lookahead without parser-side buffering.

Impact:

- Parser API integration changes are required.
- Any parser logic relying on next()/peek() semantics must be adapted to eat_token()/peek_token(i).

## 2) Token Kind Encoding Strategy

Current:

- Uses strongly named TokenKind entries for most operators and punctuation (PLUS, ASSIGN, RANGE, ARROW, etc.).

Proposed:

- Uses TokenType enum with custom tokens starting at 256, and many one-character tokens encoded as their ASCII value via static_cast<TokenType>(c).

Impact:

- Existing parser switch statements over TokenKind will not compile or will misbehave without a mapping layer.
- Diagnostics and tooling that depend on descriptive token names become harder unless wrapped.

## 3) Lexeme and Value Representation

Current:

- Token stores std::string_view lexeme into original source.
- Efficient and zero-copy for most tokens.

Proposed:

- Token stores std::string string_value and value_flags.
- Identifier/string data copied into token payload.

Impact:

- Higher per-token memory churn.
- Different ownership assumptions; safer independence from source lifetime, but potentially slower.

## 4) Error Model

Current:

- Produces TokenKind::ILLEGAL token for lexical errors.
- Error message text is not currently embedded into token (make_error ignores message).

Proposed:

- Reports through configurable callback (line, col, message).
- Tracks reported_error_ and may force EOF behavior in lookahead.

Impact:

- Cleaner diagnostics path for frontends.
- Parser error recovery strategy must be reconsidered because error token stream behavior changes.

## 5) Input Lifecycle

Current:

- Lexer(std::string_view source) receives source once; no explicit file loading in lexer.

Proposed:

- Default-constructible lexer with set_input_from_string and set_input_from_file.
- Includes read_entire_file helper.

Impact:

- More flexible API for driver integration.
- Requires clear reset semantics in parser pipeline between compilations.

## Detailed Behavior Differences

## Comments

Current:

- Supports // line comments and nested /* */ block comments.

Proposed:

- Uses # for line comments and nested /* */ block comments.

Impact:

- Source files using // comments will tokenize differently unless behavior is merged.

## Dot/Range Operators

Current:

- Supports ., .., ..= via lex_dot().

Proposed:

- Supports . and ... (TRIPLE_DOT).
- No direct ..= equivalent in provided draft.

Impact:

- Grammar and parser expectations diverge for range syntax.

## Minus Family

Current:

- Supports - and ->.

Proposed:

- Supports - and --- (TRIPLE_MINUS).
- No direct arrow token in provided draft.

Impact:

- Existing function/type syntax that depends on -> may break.

## Number Lexing

Current:

- Distinguishes INTEGER_LITERAL and FLOAT_LITERAL.
- Handles decimal floats with exponents, 0x hex, and 0b binary.

Proposed:

- Numbers currently flow through make_ident_or_keyword path in compose_new_token().
- Uses ValueFlags for NUMBER/HEX/BINARY/FLOAT in token metadata model, but parsing logic for full numeric classification is not present in the shown code.

Impact:

- Numeric token typing behavior does not match current compiler expectations.
- Requires explicit numeric scanner parity work before replacement.

## String and Escape Semantics

Current:

- Basic string lexing with escape skipping; no full escape decoding into token value.
- Char literal scanner exists.

Proposed:

- Decodes escapes: \n, \r, \t, \0, \e, \xNN, \dNNN, \uXXXX, \UXXXXXXXX.
- Converts Unicode code points to UTF-8 bytes.
- Appends explicit '\0' terminator to built string payload.

Impact:

- Semantics are richer, but differ from current token contract.
- Explicit null terminator can create subtle bugs if downstream code expects plain source lexeme content.

## Identifier Rules

Current:

- Identifier start: alpha or underscore.
- Continue: alnum or underscore.

Proposed:

- Start allows slash '/'.
- Continue allows '-', '.', '/'.

Impact:

- This can absorb punctuation into identifiers that are currently tokenized as operators/delimiters.
- Grammar ambiguities likely unless this is intentional for a different language mode.

## Keyword Handling

Current:

- Full keyword table in lexer.

Proposed:

- check_for_keyword is stubbed and returns non-keyword sentinel.

Impact:

- All keywords currently become identifiers unless keyword map is implemented.

## Here-String Support

Current:

- Contains here-string state and consume_here_string logic.

Proposed:

- Token flag mentions HERE_STRING but no equivalent here-string scanning logic in shown compose_new_token path.

Impact:

- Current here-string language feature likely regresses if replaced directly.

## Token Location and Trivia Differences

Current:

- Token has start/end SourceLocation and source offset.

Proposed:

- Token has l0/c0/l1/c1 and preceeding_whitespace count.

Impact:

- Potential gains for formatting-sensitive parsing.
- Loss of explicit byte offset unless reconstructed.

## API Compatibility Matrix

Current API:

- Lexer(std::string_view)
- Token next()
- Token peek()
- void synchronize()

Proposed API:

- Lexer()
- void set_input_from_string(std::string_view)
- bool set_input_from_file(const std::string&)
- const Token* peek_next_token()
- const Token* peek_token(int32_t)
- void eat_token()
- const Token* get_last_token() const
- std::string get_and_eat_remainder_of_line(...)
- void set_error_reporter(...)

Result:

- Not source-compatible.
- Parser and diagnostics interfaces need adapters.

## Performance and Memory Tradeoffs

Potential wins in proposed version:

- Multi-lookahead without parser-managed buffering.
- Richer string unescape in lexer may reduce parser work.

Potential costs:

- More token copying via std::string payload.
- Callback/error plumbing overhead.
- Extra state bookkeeping and ring management complexity.

## Correctness and Regression Risks If Replaced As-Is

- Keyword tokenization regression (keyword map currently stubbed).
- Operator grammar mismatch (missing/changed tokens like ->, ..=, // comments).
- Identifier over-acceptance (/, -, . continuation rules).
- Number token kind mismatch with current parser expectations.
- String payload contract changes (decoded data and trailing null byte).
- Here-string behavior mismatch.

## Recommendation

Do not replace the current lexer wholesale with this draft yet.

Safer migration path:

1. Introduce an adapter layer that maps proposed TokenType to current TokenKind and preserves current parser API.
2. Implement keyword table parity before enabling parser integration.
3. Add compatibility tests for existing syntax features:
   - arrows, ranges, assignment forms
   - numeric literals (int/float/hex/binary/exponent)
   - comments (line + block)
   - here-string behavior
   - string and char literal edge cases
4. Gate Unicode escape decoding behind tests that assert expected byte sequences.
5. Decide explicitly whether null-terminated string payload is part of token contract.

## Suggested Next Step in This Repo

Create a compatibility-focused intermediate lexer mode that keeps current TokenKind output while incrementally adopting selected improvements from the proposed implementation:

- configurable error callback
- nested block comment handling parity checks
- Unicode escape decode for strings
- optional multi-token lookahead

This keeps parser/sema stable while allowing measured adoption of new lexer features.