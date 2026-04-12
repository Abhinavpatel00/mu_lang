# Copilot Instructions

This project follows a strict prototype coding style.

## Core Rules

- Do not use exceptions.
- Do not use RAII patterns.
- Do not use object-oriented programming.
- Do not introduce classes unless the design is discussed and approved first.
- Everything is public by default.
- Prefer simple structs for data.
- Prefer free functions that operate on structs.
- we use data oriented design 
## Code Style Direction

- Keep logic explicit and easy to trace.
- Avoid deep abstraction layers.
- Prefer straightforward control flow over clever patterns.
- Keep functions small and focused.
- Optimize for fast iteration and predictable performance.

## Preferred C++ Features

- Use plain structs with public fields.
- Use namespaces only for logical grouping.
- Use standard containers and algorithms in simple ways.
- Keep ownership and lifetime handling explicit.

## Avoid By Default

- class hierarchies
- virtual methods
- inheritance
- polymorphic OO designs
- hidden control flow
- exception-based error handling

## Design Change Policy

If a change requires classes, RAII, or OO patterns, stop and discuss the design first before implementation.

- lexer is fine to be a class if it simplifies state management, but semantic analysis should avoid classes unless necessary.
- enum  and constants should be UPPER_SNAKE_CASE, struct and function names should be lower_snake_case.
- If a new abstraction layer is needed, prefer adding a new function or struct over introducing a new class hierarchy.
- For data structures, prefer simple structs with public fields over complex classes with private members and accessors.
- For error handling, prefer returning error codes or using `std::optional` over exceptions.
-  type names should be PascalCase and function/variable names should be snake_case.
