# mycompiler

Minimal Mu-like compiler that supports:
- `name : int = <int>;`
- `print(name);` or `print(123);`

Build:
```
mkdir -p build
cd build
cmake ..
cmake --build .
```

Run:
```
./mucc path/to/file.mu -o out
./out
```
# mu_lang
