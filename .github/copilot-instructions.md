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
