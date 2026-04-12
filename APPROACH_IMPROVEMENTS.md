# Prototype-Phase Simplification Plan

This guide focuses on one goal: keep the code very simple while preserving both speed and flexibility during the prototype stage.

Scope: recommendations are based on the current code shape in [main.cpp](main.cpp).

## What We Optimize For In Prototype

1. Fast iteration: new syntax/features should take hours, not days.
2. Good-enough performance: avoid obvious slow paths, skip heavy architecture.
3. Easy rewrite path: choices today should not block a future AST or full compiler pipeline.

## Simple Rules To Follow

1. Keep one-pass where possible.
2. Use plain data structures first.
3. Split files only when a section becomes hard to scan.
4. Prefer predictable code over clever abstractions.
5. Add micro-benchmarks before tuning.

## Practical Simplifications (High Impact, Low Complexity)

## 1. Keep Current Token-Driven Checker, But Formalize It

Do not build full AST yet. Instead:
- Keep token-stream semantic checks.
- Add a tiny set of parser helper utilities.
- Document accepted statement patterns in one place.

Why this is better now:
- Minimal rewrite.
- Maintains flexibility while grammar is still changing.
- Avoids early architecture lock-in.

Implementation pattern:

```cpp
// Statement forms this prototype understands:
// 1) name := expr
// 2) name :: expr
// 3) name : type (= expr)?
// 4) name = expr
```

## 2. Replace Expensive Defaults With Faster Ones

Current simplification wins:
- Change symbol table from `std::map` to `std::unordered_map`.
- Reserve token storage when file size is known.
- Avoid unnecessary `std::string` copies for token text in hot paths.

Why this is prototype-friendly:
- Tiny code changes.
- Immediate speed improvements.
- No new architecture required.

## 3. Split Only Into 3 Files (Not Full Module Tree Yet)

Instead of fully modularizing now, do a minimal split:

```text
main.cpp            # CLI + orchestration only
lexer.cpp/.hpp      # tokenization
sema.cpp/.hpp       # MiniSemanticChecker + diagnostics
```

Why this balance works:
- Keeps project discoverable.
- Reduces merge conflicts and giant-file fatigue.
- Still easy to collapse/reshape later.

## 4. Use a Lightweight Diagnostic Model

Keep diagnostics simple:
- line, column, message
- optional code string (example: MU001)

Do not introduce a full diagnostic engine yet.

Why:
- Users still get actionable errors.
- Low implementation and maintenance cost.

## 5. Add One Benchmark Command

Prototype performance needs visibility, not a full infra stack.

Add one benchmark mode:
- run lexer + sema N times
- print average ms and tokens/s

Minimal output example:

```text
file=examples/bigtest.mu runs=50 avg_ms=3.8 tokens_per_sec=1.2M
```

## 6. Test Only What Prevents Regressions

In prototype, small targeted tests are enough:
- 10 lexer golden tests.
- 10 semantic diagnostics tests.
- 2 large-file smoke tests.

Skip broad framework-heavy testing until grammar stabilizes.

## Recommended Coding Style For Prototype Flexibility

1. Keep functions short and data-in/data-out.
2. Use explicit names (`parseTypedDeclaration`, `skipExpression`) over generic helpers.
3. Avoid class hierarchies for syntax nodes until AST is required.
4. Keep mutation local and obvious.
5. Prefer adding a new function over adding a new abstraction layer.

## 2-Week Action Plan

Week 1:
1. Swap `std::map` -> `std::unordered_map` in semantic bindings.
2. Move lexer + sema into separate files.
3. Add benchmark mode in CLI.

Week 2:
1. Add 20 core regression tests.
2. Tighten token/string allocation hot paths.
3. Document supported grammar patterns in one markdown section.

## What Not To Do Yet

1. Do not introduce full AST + visitor framework.
2. Do not over-generalize type system internals.
3. Do not build multi-pass optimization infrastructure.
4. Do not add complex plugin architecture.

These are valuable later, but they slow down prototype speed now.

## Exit Criteria For Moving Beyond Prototype

Move to a fuller architecture only when at least 2 are true:
1. Grammar changes become rare (stable for 2-3 weeks).
2. Token-based semantic logic becomes hard to reason about.
3. New feature work repeatedly requires touching unrelated logic.
4. Benchmark shows structural bottlenecks, not micro bottlenecks.

## Final Recommendation

For this phase: simplify by keeping the current model, making small performance-focused data-structure changes, and introducing only minimal file boundaries. This gives strong iteration speed now and still leaves a clean path to AST and deeper architecture later.